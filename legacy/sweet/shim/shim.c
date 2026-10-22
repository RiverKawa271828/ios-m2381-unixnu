/*
 * m2381 probe: one boot answers the questions that decide the XNU port.
 *   - CPU feature registers (16K granule, PARANGE)
 *   - PSCI/SMCCC version and which PSCI calls the Qualcomm TZ offers
 *   - SMP: PSCI CPU_ON for the 7 secondaries, each reports its MPIDR
 *   - GICv3 Group 1 IRQ from the virtual timer (standard INTID 27)
 *   - read-only UFS probe on the link ABL left up
 * Any unexpected exception paints a blue panic screen.
 */
#include <stdint.h>
#include "console.h"
#include "mmu.h"
#include "loader.h"
#include "ufs.h"

/* PSHOLD (upstream-kalama guess 0x0c264000) is deliberately never written:
 * no pshold node exists in either DT (nura kernel or Flyme stock FDT), and
 * PSCI SYSTEM_RESET is the only reset this device has ever exercised.
 * Reboot-to-fastboot is NOT routed via the IMEM restart_reason magic: on
 * 2026-10-19 round 1 the shim wrote 0x77665500 to 0x146aa65c, the PSCI
 * reset fired, and the device came back as a NORMAL boot -- booting shim
 * again, looping, and burning the ABL slot retry counter (7 -> 1, one step
 * from an automatic slot switch). The pmOS-proven soft path to fastboot
 * goes through the PMK8550 PON reboot-mode register (kernel qcom-pon),
 * which bare metal can't reach without an SPMI driver. Until that exists
 * the shim never resets itself: it sits on the screen and a human reset
 * (pwr+volDn) enters fastboot directly without any boot attempt. */

#define GICD_BASE		0x17100000UL
#define GICR_BASE		0x17180000UL
#define GICR_SIZE		0x200000UL	/* 8 PE */
#define GICR_PE_SIZE		0x40000UL
#define GICR_TYPER		0x08
#define GICD_TYPER2		0x1c		/* v3.1: bit8 nASSGIcap */
#define GIC_IIDR		0xffe8		/* GICD and each GICR frame */
#define GICD_CTLR_nASSGIreq	(1U << 8)	/* kernel sets this before its SGI-page priority writes */
#define GICR_WAKER		0x14
#define GICR_SGI		0x10000
#define GICR_IGROUPR0		(GICR_SGI + 0x080)
#define GICR_ISENABLER0		(GICR_SGI + 0x100)
#define GICR_ICENABLER0		(GICR_SGI + 0x180)
#define GICR_IPRIORITYR		(GICR_SGI + 0x400)
#define GICR_ISPENDR0		(GICR_SGI + 0x200)
#define GICR_ISACTIVER0		(GICR_SGI + 0x300)
#define GICR_ICFGR1		(GICR_SGI + 0xc04)
#define VTIMER_INTID		27	/* standard PPI11 */
#define PTIMER_INTID		28	/* standard PPI12 */
#define SGI_INTID		1

#define PSCI_VERSION		0x84000000UL
#define PSCI_CPU_SUSPEND	0xc4000001UL
#define PSCI_CPU_OFF		0x84000002UL
#define PSCI_CPU_ON		0xc4000003UL
#define PSCI_MIGRATE_INFO_TYPE	0x84000006UL
#define PSCI_SYSTEM_RESET	0x84000009UL
#define PSCI_FEATURES		0x8400000aUL
#define PSCI_SYSTEM_SUSPEND	0xc400000eUL
#define PSCI_SYSTEM_RESET2	0xc4000012UL
#define SMCCC_VERSION		0x80000000UL

#define NR_CPUS			8

#define rsr(r) ({ uint64_t _v; __asm__ volatile("mrs %0, " r : "=r"(_v)); _v; })
#define wsr(r, v) __asm__ volatile("msr " r ", %0" : : "r"((uint64_t)(v)))
#define isb() __asm__ volatile("isb" ::: "memory")

static inline uint32_t rd32(uintptr_t a) { return *(volatile uint32_t *)a; }
static inline uint64_t rd64(uintptr_t a) { return *(volatile uint64_t *)a; }
static inline void wr32(uintptr_t a, uint32_t v) { *(volatile uint32_t *)a = v; }
static inline void wr8(uintptr_t a, uint8_t v) { *(volatile uint8_t *)a = v; }

/* v4 abort-recovery: armed around one deliberate MMIO access; a sync
 * external data abort on it advances ELR past the faulting instruction. */
static volatile int rec_arm;
static volatile uint64_t rec_far;
static volatile uint64_t rec_ec;

/* guarded MMIO: run one access with abort recovery armed.
 * returns 0 = completed, nonzero = sync exception recovered (rec_ec = ESR.EC). */
static int g_wr32(uintptr_t a, uint32_t v)
{
	rec_far = 0;
	rec_ec = 0x100;
	rec_arm = 1;
	wr32(a, v);
	rec_arm = 0;
	return rec_far != 0;
}

static int g_rd32(uintptr_t a, uint32_t *out)
{
	rec_far = 0;
	rec_ec = 0x100;
	rec_arm = 1;
	*out = rd32(a);
	rec_arm = 0;
	return rec_far != 0;
}

extern char secondary_entry[];

static const char *stage;	/* set at runtime: no absolute pointers in data */
static volatile uint64_t sec_mpidr[NR_CPUS];
static volatile uint64_t sec_probe[6];	/* CPU1 redistributor write test */
static volatile uint32_t irq_count, irq_other, irq_last;
static volatile int irq_src;

/* ICC_SGI1R_EL1 value targeting the calling core only. */
static uint64_t sgi_self(unsigned id)
{
	uint64_t m = rsr("mpidr_el1");
	return ((m >> 16) & 0xff) << 32 |	/* Aff2 */
	       ((m >> 8) & 0xff) << 16 |	/* Aff1 */
	       (uint64_t)id << 24 |
	       (1U << (m & 0xf));		/* target list: Aff0 */
}

static uint64_t smc(uint64_t fn, uint64_t a1, uint64_t a2, uint64_t a3)
{
	register uint64_t x0 __asm__("x0") = fn;
	register uint64_t x1 __asm__("x1") = a1;
	register uint64_t x2 __asm__("x2") = a2;
	register uint64_t x3 __asm__("x3") = a3;
	__asm__ volatile("smc #0"
			 : "+r"(x0), "+r"(x1), "+r"(x2), "+r"(x3)
			 :
			 : "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11",
			   "x12", "x13", "x14", "x15", "x16", "x17", "memory");
	return x0;
}

static uint64_t now(void)
{
	isb();
	return rsr("cntpct_el0");
}

static void delay_ms(unsigned ms)
{
	uint64_t end = now() + rsr("cntfrq_el0") / 1000 * ms;
	while (now() < end)
		;
}

static void __attribute__((noreturn)) hold_screen(void)
{
	con_skip();
	con_str("RESET", "manual: pwr+volDn -> fastboot");
	for (;;)
		__asm__ volatile("wfe");
}

/* ---- exceptions ---- */

static void irq_handler(void)
{
	uint64_t iar = rsr("icc_iar1_el1");
	uint32_t id = iar & 0xffffff;

	if (id >= 1020 && id <= 1023)
		return;	/* spurious */
	irq_last = id;
	if (id == (uint32_t)irq_src)
		irq_count++;
	else
		irq_other++;
	if (id == VTIMER_INTID)
		wsr("cntv_tval_el0", rsr("cntfrq_el0") / 100);	/* 10 ms */
	else if (id == PTIMER_INTID)
		wsr("cntp_tval_el0", rsr("cntfrq_el0") / 100);
	wsr("icc_eoir1_el1", iar);
	isb();
}

void exc_handler(uint64_t slot, uint64_t *regs)
{
	if (slot == 5) {	/* IRQ from current EL, SPx */
		irq_handler();
		return;
	}
	/* guarded-probe recovery: a deliberate sync exception on one MMIO
	 * access resumes past its (single-word) str/ldr and the probe
	 * continues. v5: ANY sync class recovers while armed -- round 4 saw
	 * the same guarded store die as EC=0/FAR=0 (undefined-instruction
	 * class) that v4's EC=0x25-only gate let through to the panic. */
	if (rec_arm && (slot & 3) == 0) {
		uint64_t esr = rsr("esr_el1");
		rec_ec = esr >> 26;
		rec_far = rsr("far_el1");
		rec_arm = 0;
		wsr("elr_el1", rsr("elr_el1") + 4);
		return;
	}
	/* keep the probe results on screen: append below, never clear.
	 * Bottom-anchor so the panic block survives even a full probe screen
	 * (worst case clips the tail of the UFS partition listing). */
	con_skip();
	if (con_row() + 17 > con_rows())
		con_row_set(con_rows() - 17);
	switch (slot & 3) {
	case 0: con_str("PANIC", "SYNC"); break;
	case 1: con_str("PANIC", "IRQ"); break;
	case 2: con_str("PANIC", "FIQ"); break;
	default: con_str("PANIC", "SERROR"); break;
	}
	con_hex2("SLOT", "n=", slot, "cpu=", rsr("mpidr_el1"));
	con_str("STAGE", stage);
	con_hex("ESR", rsr("esr_el1"));
	con_hex("ELR", rsr("elr_el1"));
	con_hex("FAR", rsr("far_el1"));
	/* what the CPU actually executed (or fetched) at ELR: round 4 died
	 * as EC=0 on a word our ELF says is a plain STR -- corruption vs
	 * misclassified abort, decided by this line. */
	con_hex("INSN", *(volatile uint32_t *)(uintptr_t)(rsr("elr_el1") & ~3UL));
	con_hex("SPSR", rsr("spsr_el1"));
	con_hex("X0", regs[0]);
	con_hex("X1", regs[1]);
	con_hex("X2", regs[2]);
	con_hex("X3", regs[3]);
	con_hex("X19", regs[19]);
	con_hex("X21", regs[21]);
	con_hex("X24", regs[24]);
	con_hex("X30", regs[31]);	/* frame is 32 slots: x30 stashed at [sp+248]; regs[29] read dead stack (v3.x bug) */
	hold_screen();
}

/* ---- SMP ---- */

static uintptr_t find_gicr(void);

void secondary_main(uint64_t idx)
{
	sec_mpidr[idx] = rsr("mpidr_el1") | (1ULL << 63);
	if (idx == 1) {
		uintptr_t r = find_gicr();
		if (r) {
			sec_probe[0] = rd32(r + GICR_IGROUPR0);
			wr32(r + GICR_IGROUPR0, 0xffffffff);
			sec_probe[1] = rd32(r + GICR_IGROUPR0);
			wr32(r + GICR_IPRIORITYR + 16, 0xa0a0a0a0);	/* CPU1's own SGI page, MMU off */
			sec_probe[2] = rd32(r + GICR_IPRIORITYR + 16);
			sec_probe[4] = 0x600d;	/* got past the priority write */
		}
		sec_probe[3] = r ? 1 : 0xdead;
	}
	__asm__ volatile("dsb sy" ::: "memory");
	smc(PSCI_CPU_OFF, 0, 0, 0);
}

static void __attribute__((unused)) test_smp(void)
{
	stage = "smp";
	uint64_t self = rsr("mpidr_el1") & 0xffffff;
	int64_t ret[NR_CPUS];

	for (int i = 1; i < NR_CPUS; i++)
		ret[i] = smc(PSCI_CPU_ON, (uint64_t)i << 8, (uintptr_t)secondary_entry, i);
	delay_ms(500);
	for (int i = 1; i < NR_CPUS; i++) {
		char label[] = "CPU0";
		label[3] = '0' + i;
		if (((uint64_t)i << 8) == self)
			con_str(label, "is boot cpu");
		else
			con_hex2(label, "ret=", (uint32_t)ret[i], "mpidr=",
				 (uint32_t)sec_mpidr[i] | (sec_mpidr[i] ? 0 : 0xdead0000));
	}
}

/* ---- GIC + timer ---- */

static uintptr_t find_gicr(void)
{
	uint64_t aff = rsr("mpidr_el1");
	uint32_t want = (aff & 0xffffff) | ((aff >> 8) & 0xff000000);

	for (uintptr_t off = 0; off < GICR_SIZE; off += GICR_PE_SIZE) {
		uint64_t typer = rd64(GICR_BASE + off + GICR_TYPER);
		if ((uint32_t)(typer >> 32) == want)
			return GICR_BASE + off;
		if (typer & (1 << 4))	/* Last */
			break;
	}
	return 0;
}

static void gicd_wait_rwp(void)
{
	for (int i = 0; i < 1000000 && (rd32(GICD_BASE) & (1U << 31)); i++)
		;
}

/* Run one interrupt source for ~300 ms and report what the GIC saw. */
static void irq_window(const char *label, uintptr_t r, int src)
{
	uint64_t f = rsr("cntfrq_el0");
	uint32_t pend = 0, act = 0, isr = 0, hppi = 0x3ff, ist = 0;

	irq_count = irq_other = 0;
	irq_last = 0x3ff;
	irq_src = src;
	if (src == VTIMER_INTID) {
		wsr("cntv_tval_el0", f / 100);
		wsr("cntv_ctl_el0", 1);
	} else if (src == PTIMER_INTID) {
		wsr("cntp_tval_el0", f / 100);
		wsr("cntp_ctl_el0", 1);
	}
	isb();
	/* sample state with IRQs still masked, after the first period */
	uint64_t t = now() + f / 50;
	while (now() < t)
		;
	if (src == SGI_INTID)
		wsr("icc_sgi1r_el1", sgi_self(SGI_INTID));
	isb();
	ist = src == PTIMER_INTID ? rsr("cntp_ctl_el0") : rsr("cntv_ctl_el0");
	pend = rd32(r + GICR_ISPENDR0);
	act = rd32(r + GICR_ISACTIVER0);
	isr = rsr("isr_el1");
	hppi = rsr("icc_hppir1_el1");

	__asm__ volatile("msr daifclr, #2" ::: "memory");
	t = now() + f * 3 / 10;
	/* SGI: one already sent above, send 9 more 10 ms apart -> expect 10 */
	uint64_t next = now() + f / 100;
	for (int sent = 1; now() < t;)
		if (src == SGI_INTID && sent < 10 && now() >= next) {
			wsr("icc_sgi1r_el1", sgi_self(SGI_INTID));
			isb();
			sent++;
			next += f / 100;
		}
	__asm__ volatile("msr daifset, #2" ::: "memory");
	wsr("cntv_ctl_el0", 0);
	wsr("cntp_ctl_el0", 0);
	isb();

	con_hex2(label, "irqs=", irq_count, "other=", irq_other);
	con_hex2(" pre", "pend=", pend, "act=", act);
	con_hex2(" pre", "isr=", isr, "hppi=", hppi);
	con_hex2(" pre", "tctl=", ist, "last=", irq_last);
}

static void test_gic(void)
{
	stage = "gic sre";
	uint64_t sre = rsr("icc_sre_el1");
	if (!(sre & 1)) {
		wsr("icc_sre_el1", sre | 1);
		isb();
	}
	if (!(rsr("icc_sre_el1") & 1)) {
		con_str("GIC", "no sysreg iface, skip");
		return;
	}

	stage = "gic dist";
	uint32_t dctlr = rd32(GICD_BASE);
	/* Non-secure view (DS=0): bit0 EnableGrp1, bit1 EnableGrp1A, bit4 ARE_NS; Linux writes 0x13 */
	wr32(GICD_BASE, dctlr | 0x13);
	gicd_wait_rwp();
	con_hex2("GICD", "was=", dctlr, "now=", rd32(GICD_BASE));

	stage = "gic redist";
	uintptr_t r = find_gicr();
	if (!r) {
		con_str("GICR", "not found");
		return;
	}
	uint32_t waker = rd32(r + GICR_WAKER);
	con_hex2("GICR", "waker0=", waker, "typerL=", (uint32_t)rd64(r + GICR_TYPER));
	if (waker & 2) {
		wr32(r + GICR_WAKER, waker & ~2U);
		int i;
		for (i = 0; i < 100000 && (rd32(r + GICR_WAKER) & 4); i++)
			;
		con_hex2("GICR", "wakespin=", (uint32_t)i, "waker1=", rd32(r + GICR_WAKER));
		if (i >= 100000) {
			con_str("GICR", "wake stuck, skip gic");
			return;	/* sleeping RD: SGI page would abort */
		}
	}
	con_hex2("GICR", "base=", r, "waker=", rd32(r + GICR_WAKER));

	stage = "gic mmu";
	/* rounds 1-3 aborted on SGI-page priority writes at nGnRnE AND at the
	 * 32MB nGnRE block below (round 3, this exact mapping) -- same frame
	 * takes IGROUPR0/WAKER writes fine, so memory TYPE is ruled out.
	 * nGnRE kept for parity with the kernel's ioremap. */
	struct mmu_info mi;
	if (mmu_enable_probe(&mi)) {
		con_str("MMU", "enable failed");
		return;
	}
	con_hex2("MMU", "sctlr=", (uint32_t)mi.sctlr_after, "ttbr=", (uint32_t)(mi.ttbr >> 14));
	con_hex2(" MMAIR", "mair=", (uint32_t)rsr("mair_el1"),
		 "par=", (uint32_t)mmu_probe(GICR_BASE + GICR_SGI + 0x400));

	stage = "gic config";
	/* GIC identity: kernel dmesg says DS=1, nASSGIcap present. IIDR tells
	 * which implementation (GIC-600/700/QCOM); the SGI frames' IIDR tells
	 * whether those frames decode as GICR at all. */
	uint32_t typer = 0, typer4 = 0, typer2 = 0, diidr = 0, riidr = 0,sgi0_iidr = 0, pe1_iidr = 0;
	g_rd32(GICD_BASE + 8, &typer);
	g_rd32(GICD_BASE + 4, &typer4);	/* v3.1 read 0x016B001F here, TYPER-shaped (DS=1); +8 = 0x47000070. which is TYPER? */
	g_rd32(GICD_BASE + GICD_TYPER2, &typer2);
	g_rd32(GICD_BASE + GIC_IIDR, &diidr);
	g_rd32(r + GIC_IIDR, &riidr);
	g_rd32(r + GICR_SGI + GIC_IIDR, &sgi0_iidr);
	g_rd32(GICR_BASE + GICR_PE_SIZE + GICR_SGI + GIC_IIDR, &pe1_iidr);
	con_hex2(" ID", "typr8=", typer, "typr4=", typer4);
	con_hex2(" ID2", "typr2=", typer2, "diidr=", diidr);
	con_hex2(" IIDR", "r=", riidr, "sgi0=", sgi0_iidr);
	con_hex2(" SGIF", "pe1=", pe1_iidr, "pe0=", riidr);

	/* reads first, printed before any write, so an abort leaves data */
	uint32_t grp = rd32(r + GICR_IGROUPR0);
	uint32_t en_before = rd32(r + GICR_ISENABLER0);
	uint32_t pri_before = rd32(r + GICR_IPRIORITYR + 16);	/* INTIDs 16..19 */
	con_hex2(" CFG", "grp=", grp, "en=", en_before);
	con_hex(" PRI16", pri_before);
	wr32(r + GICR_IGROUPR0, 0xffffffff);	/* NS view: secure bits are WI */
	con_hex(" GRPRD", rd32(r + GICR_IGROUPR0));	/* does the write stick in this window? */

	/* map the SGI page: one guarded word write + readback per offset.
	 * A = data-abort, U = undefined-class sync (round 4's mystery),
	 * ? = other sync class, ! = readback aborted,
	 * S = write stuck (0x5a5a5a5a readback), . = write completed but
	 * readback unchanged. */
	static const uint32_t map_offs[] = {
		0x080, 0x100, 0x180, 0x200, 0x300, 0x380,
		0x400, 0x404, 0x408, 0x410, 0x414, 0x418, 0x41c,
		0x420, 0x440, 0x600, 0x800, 0xc00, 0xc04, 0xc08
	};
	static const char hd[] = "0123456789abcdef";
	char line[96];
	int n = 0;
	g_wr32(r + GICR_ICENABLER0, 0xffffffff);
	for (unsigned i = 0; i < sizeof(map_offs) / sizeof(map_offs[0]); i++) {
		uint32_t rb = 0;
		int wa = g_wr32(r + GICR_SGI + map_offs[i], 0x5a5a5a5a);
		char st = '?';
		if (wa)
			st = rec_ec == 0x25 ? 'A' : rec_ec == 0 ? 'U' : '?';
		int ra = g_rd32(r + GICR_SGI + map_offs[i], &rb);
		if (ra)
			st = '!';
		else if (!wa)
			st = rb == 0x5a5a5a5a ? 'S' : '.';
		line[n++] = hd[(map_offs[i] >> 8) & 0xf];
		line[n++] = hd[(map_offs[i] >> 4) & 0xf];
		line[n++] = hd[map_offs[i] & 0xf];
		line[n++] = st;
		line[n++] = ' ';
		if (n >= 11 * 6) {
			line[n] = 0;
			con_str(" MAP", line);
			n = 0;
		}
	}
	if (n) {
		line[n] = 0;
		con_str(" MAP", line);
	}

	/* kernel-proven range for contrast: GICD priority regs, same +0x400 */
	uint32_t drb = 0;
	int da = g_wr32(GICD_BASE + 0x400, 0xa0a0a0a0);
	int dra = g_rd32(GICD_BASE + 0x400, &drb);
	con_hex2(" GD4", "wrabt=", (uint32_t)da, "rdabt=", (uint32_t)dra);
	con_hex(" GDRB", drb);

	/* cross-PE: PE0 writing PE1's SGI-page priority word */
	int p1a = g_wr32(GICR_BASE + GICR_PE_SIZE + GICR_SGI + 0x400, 0xa0a0a0a0);
	uint32_t p1rb = 0;
	g_rd32(GICR_BASE + GICR_PE_SIZE + GICR_SGI + 0x400, &p1rb);
	con_hex2(" PE1", "wrabt=", (uint32_t)p1a, "rb=", p1rb);

	/* CPU1's own-page result from its nGnRnE window (secondary_main) */
	con_hex2(" CPU1", "grp=", (uint32_t)sec_probe[1], "pri=", (uint32_t)sec_probe[2]);
	con_hex2(" CPU1", "ok=", (uint32_t)sec_probe[4], "rdok=", (uint32_t)sec_probe[3]);

	/* the one config bit the kernel sets before its SGI-page priority
	 * writes and we don't: GICD_CTLR.nASSGIreq (TYPER2.nASSGIcap) */
	uint32_t gctlr = 0;
	g_rd32(GICD_BASE, &gctlr);
	int ca = g_wr32(GICD_BASE, gctlr | GICD_CTLR_nASSGIreq);
	gicd_wait_rwp();
	int na = g_wr32(r + GICR_SGI + 0x400, 0xa0a0a0a0);
	uint32_t nrb = 0;
	g_rd32(r + GICR_SGI + 0x400, &nrb);
	con_hex2(" nAS", "ctlabt=", (uint32_t)ca, "priabt=", (uint32_t)na);
	con_hex(" PRIRD", nrb);

	wr32(r + GICR_ICENABLER0, 0xffffffff);
	wr32(r + GICR_IGROUPR0, 0xffffffff);	/* map pass scrambled group bits */
	wr32(r + GICR_ISENABLER0, (1U << VTIMER_INTID) | (1U << PTIMER_INTID) | (1U << SGI_INTID));
	con_hex2("ENABLE", "was=", en_before, "now=", rd32(r + GICR_ISENABLER0));
	con_hex(" ICFGR1", rd32(r + GICR_ICFGR1));

	uint64_t ctlr = rsr("icc_ctlr_el1");
	wsr("icc_ctlr_el1", ctlr & ~2ULL);	/* EOImode 0: EOIR also deactivates */
	wsr("icc_pmr_el1", 0xf0);
	wsr("icc_bpr1_el1", 0);
	wsr("icc_igrpen1_el1", 1);
	isb();
	con_hex2("ICC", "ctlr=", ctlr, "grpen1=", rsr("icc_igrpen1_el1"));

	stage = "gic vtimer";
	irq_window("VTIMER", r, VTIMER_INTID);
	stage = "gic sgi";
	irq_window("SGI1", r, SGI_INTID);
	wr32(r + GICR_ICENABLER0, 0xffffffff);
	isb();
}

/* ---- main ---- */

static void __attribute__((unused)) psci_line(const char *label, uint64_t fn)
{
	con_hex2(label, "feat=", smc(PSCI_FEATURES, fn, 0, 0), 0, 0);
}

static uint64_t scratch[8192];	/* 64KB, lives in our (later cacheable) image */

static uint64_t __attribute__((unused)) ticks_fill(uint32_t c)
{
	uint64_t t = now();
	con_clear(c);
	return now() - t;
}

static uint64_t __attribute__((unused)) ticks_mem(void)
{
	uint64_t t = now();
	for (int pass = 0; pass < 16; pass++)
		for (int i = 0; i < 8192; i++)
			scratch[i] = scratch[i] * 3 + i;
	return now() - t;
}

static uint32_t __attribute__((unused)) to_us(uint64_t ticks)
{
	return ticks * 10 / 192;	/* 19.2 MHz */
}

void shim_main(uint64_t dtb)
{
	/* An xnu payload in the ramdisk means: boot it (MMU stays off). */
	stage = "loader";
	con_clear(RED);
	con_str("SHIM", "m2381 probe: hw bring-up questions");
	con_skip();
	const char *why = loader_boot(dtb);
	con_str("LOADER", why);
	con_skip();

	/* No payload: run the whole probe suite (MMU off, all Device memory),
	 * keep every result on screen, reset after 3 min. */
	con_hex2("PARANGE", "mmfr0=", rsr("id_aa64mmfr0_el1") & 0xf, "tg16=", (rsr("id_aa64mmfr0_el1") >> 20) & 0xf);
	con_skip();

	stage = "psci";
	con_hex2("PSCI", "ver=", smc(PSCI_VERSION, 0, 0, 0), "smccc=", smc(SMCCC_VERSION, 0, 0, 0));
	con_skip();

	stage = "smp";
	test_smp();
	con_hex2(" SEC1", "r=", (uint32_t)sec_probe[3], "wok=", (uint32_t)sec_probe[4]);
	con_skip();

	stage = "ufs";
	test_ufs();
	con_skip();

	/* gic last: v3.x lost the UFS verdict to the SGI-page panic */
	stage = "gic";
	test_gic();
	con_skip();
	con_str("DONE", "probe complete");
	hold_screen();
}
