/*
 * Milestone 5 probe: can we talk to the internal UFS that ABL booted from?
 *
 * Runs with the MMU off (all memory Device, so DMA is coherent with no cache
 * maintenance). Dumps the UFSHCI registers ABL left behind and, if the host
 * is enabled and the link is up, points the transfer request list at our own
 * buffers and issues READ-ONLY SCSI commands on LUN 0. Nothing is ever
 * written to the device: there is no SCSI WRITE in this file.
 *
 * Register layout and descriptor formats: JEDEC UFSHCI (JESD223).
 *
 * v6: START STOP UNIT (power condition ACTIVE) to the device W-LU 0xD0
 * before giving up. Recipe from UniXNU NativeUfs (SM8550 sheng, hardware
 * 2026-10-17): their ABL warm link showed the exact same picture (NOP alive,
 * every LU NOT READY) and Linux' resume path (ufshcd_set_dev_pwr_mode) cured
 * it with this one SSU. SSU touches no user data; there is still no SCSI
 * WRITE in this file.
 */
#include <stdint.h>
#include "console.h"
#include "ufs.h"

#define UFS_BASE	0x01d84000UL	/* ufshc@1d84000, same address on sweet and kalama */

#define REG_CAP		0x00
#define REG_VER		0x08
#define REG_IS		0x20
#define REG_HCS		0x30
#define REG_HCE		0x34
#define REG_UTRLBA	0x50
#define REG_UTRLBAU	0x54
#define REG_UTRLDBR	0x58
#define REG_UTRLCLR	0x5c
#define REG_UTRLRSR	0x60

#define HCS_DP		(1u << 0)
#define HCS_UTRLRDY	(1u << 1)
#define HCS_UCRDY	(1u << 3)

#define BLOCK		4096u

static inline uint32_t ufs_rd(uint32_t off) { return *(volatile uint32_t *)(UFS_BASE + off); }
static inline void ufs_wr(uint32_t off, uint32_t v) { *(volatile uint32_t *)(UFS_BASE + off) = v; }

/* UTP transfer request descriptor (32 bytes). */
struct utrd {
	uint32_t dw0;		/* [31:28] CT, [26:25] DD, [24] I */
	uint32_t dw1;
	uint32_t dw2;		/* [7:0] OCS */
	uint32_t dw3;
	uint32_t ucdba;		/* command descriptor base, 128-byte aligned */
	uint32_t ucdbau;
	uint32_t resp;		/* [31:16] response UPIU offset, [15:0] length (DWORDs) */
	uint32_t prdt;		/* [31:16] PRDT offset, [15:0] entries (DWORDs/entries) */
};

/* UTP command descriptor: command UPIU, response UPIU, PRDT. */
struct ucd {
	uint8_t cmd[512];
	uint8_t resp[512];
	struct {
		uint32_t addr_lo, addr_hi, rsvd, dbc;	/* dbc = byte count - 1 */
	} prdt[1];
};

static struct utrd utrl[32] __attribute__((aligned(1024)));
static struct ucd ucd __attribute__((aligned(128)));
static uint8_t data[2 * BLOCK] __attribute__((aligned(4096)));
static uint32_t last_sense;		/* key<<16 | asc<<8 | ascq, 0 if none */
static uint32_t qs_val;			/* query response value field (BE32 at resp+20) */

static uint64_t ticks(void)
{
	uint64_t t;
	__asm__ volatile("isb; mrs %0, cntvct_el0" : "=r"(t));
	return t;
}

static void zero(void *p, unsigned n)
{
	volatile uint8_t *b = p;
	for (unsigned i = 0; i < n; i++)
		b[i] = 0;
}

static uint32_t be32(const uint8_t *p)
{
	return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3];
}

/*
 * Issue one SCSI command (read direction or no data) in slot 0.
 * Returns 0 on success, otherwise a code describing the failure stage.
 */
static uint32_t scsi(uint8_t lun, const uint8_t *cdb, unsigned cdb_len, void *buf, uint32_t len,
    uint32_t *ocs_out, uint32_t *status_out)
{
	static uint8_t tag;
	zero(&utrl[0], sizeof(utrl[0]));
	zero(&ucd, sizeof(ucd));

	/* Command UPIU: header (12 bytes), expected length, CDB. */
	ucd.cmd[0] = 0x01;			/* transaction type: COMMAND */
	ucd.cmd[1] = len ? 0x40 : 0x00;		/* flags: R (data in) */
	ucd.cmd[2] = lun;
	ucd.cmd[3] = ++tag;
	ucd.cmd[12] = len >> 24;
	ucd.cmd[13] = len >> 16;
	ucd.cmd[14] = len >> 8;
	ucd.cmd[15] = len;
	for (unsigned i = 0; i < cdb_len; i++)
		ucd.cmd[16 + i] = cdb[i];

	uintptr_t ucd_pa = (uintptr_t)&ucd;
	utrl[0].dw0 = (1u << 28) | ((len ? 2u : 0u) << 25) | (1u << 24);	/* UFS storage, dev->host */
	utrl[0].dw2 = 0x0f;			/* OCS: invalid until the host updates it */
	utrl[0].ucdba = (uint32_t)ucd_pa;
	utrl[0].ucdbau = (uint32_t)(ucd_pa >> 32);
	utrl[0].resp = (512u / 4) << 16 | (512u / 4);
	if (len) {
		uintptr_t buf_pa = (uintptr_t)buf;
		ucd.prdt[0].addr_lo = (uint32_t)buf_pa;
		ucd.prdt[0].addr_hi = (uint32_t)(buf_pa >> 32);
		ucd.prdt[0].dbc = len - 1;
		utrl[0].prdt = (1024u / 4) << 16 | 1;
	}
	__asm__ volatile("dsb sy" ::: "memory");

	ufs_wr(REG_UTRLDBR, 1);
	uint64_t start = ticks();
	while (ufs_rd(REG_UTRLDBR) & 1) {
		if (ticks() - start > 19200000) {	/* 1 s */
			ufs_wr(REG_UTRLCLR, ~1u);	/* clear slot 0 (write 0 to the bit) */
			return 1;			/* timeout */
		}
	}
	__asm__ volatile("dsb sy" ::: "memory");
	*ocs_out = utrl[0].dw2 & 0xff;
	*status_out = ucd.resp[7];		/* SCSI status in the response UPIU */
	/* Sense data sits after the 16 reserved bytes: len@32, data@34, key@+2, asc@+12, ascq@+13. */
	last_sense = ((uint32_t)(ucd.resp[36] & 0xf) << 16) | ((uint32_t)ucd.resp[46] << 8) | ucd.resp[47];
	if (*status_out != 2)			/* CHECK CONDITION is the only status carrying sense */
		last_sense = 0;
	if (*ocs_out)
		return 2;
	if (ucd.resp[0] != 0x21)		/* RESPONSE UPIU */
		return 3;
	return *status_out ? 4 : 0;
}

static uint32_t read_blocks(uint32_t lba, uint32_t count, void *buf)
{
	uint8_t cdb[10] = { 0x28, 0, lba >> 24, lba >> 16, lba >> 8, lba, 0, count >> 8, count, 0 };
	uint32_t ocs, st;
	return scsi(0, cdb, sizeof(cdb), buf, count * BLOCK, &ocs, &st);
}

/*
 * Raw UPIU sender (query 0x16 / NOP OUT 0x00) with a configurable transfer
 * descriptor command type. fn = query function byte (header dword 1 byte 1:
 * 0x01 standard read, 0x02 standard write); the pre-wake sweep passes 0 to
 * stay byte-identical to the old rounds. Packs ocs<<24 | resp type<<16 |
 * resp[6]<<8 | resp[12] (query: result and echoed opcode) so one hex print
 * tells the whole story. 0xffffffff = doorbell timeout.
 */
static uint32_t ufssend(uint8_t ct, uint8_t type, uint8_t lun, uint8_t f1, uint8_t f2, uint8_t fn)
{
	static uint8_t qtag;
	zero(&utrl[0], sizeof(utrl[0]));
	zero(&ucd, sizeof(ucd));
	ucd.cmd[0] = type;
	ucd.cmd[2] = lun;
	ucd.cmd[3] = ++qtag;
	ucd.cmd[5] = fn;
	ucd.cmd[12] = f1;
	ucd.cmd[13] = f2;
	uintptr_t ucd_pa = (uintptr_t)&ucd;
	utrl[0].dw0 = ((uint32_t)ct << 28) | (1u << 24);
	utrl[0].dw2 = 0x0f;
	utrl[0].ucdba = (uint32_t)ucd_pa;
	utrl[0].ucdbau = (uint32_t)(ucd_pa >> 32);
	utrl[0].resp = (512u / 4) << 16 | (512u / 4);
	__asm__ volatile("dsb sy" ::: "memory");
	ufs_wr(REG_UTRLDBR, 1);
	uint64_t start = ticks();
	while (ufs_rd(REG_UTRLDBR) & 1) {
		if (ticks() - start > 19200000) {
			ufs_wr(REG_UTRLCLR, ~1u);
			return 0xffffffff;
		}
	}
	__asm__ volatile("dsb sy" ::: "memory");
	qs_val = be32(ucd.resp + 20);
	return ((uint32_t)(utrl[0].dw2 & 0xff) << 24) | ((uint32_t)ucd.resp[0] << 16) |
	    ((uint32_t)ucd.resp[6] << 8) | ucd.resp[12];
}

/*
 * Query matrix: the XNU driver needs the (command type, LUN) shape this
 * device answers. READ_FLAG opcode 0x5 / SET_FLAG 0x6, fDeviceInit idn 1.
 */
static void query_sweep(void)
{
	static const struct { uint8_t ct, lun, op; } g[] = {
		{ 1, 0x00, 0x05 },	/* READ fDeviceInit, LU 0: the XNU shape */
		{ 1, 0xd0, 0x05 },	/* device well-known LUN */
		{ 1, 0x50, 0x05 },	/* alternative W-LUN encoding */
		{ 0, 0x00, 0x05 },	/* command type 0 */
		{ 2, 0x00, 0x05 },	/* command type 2 */
		{ 1, 0x00, 0x06 },	/* SET fDeviceInit */
		{ 1, 0xd0, 0x06 },
	};
	con_str("UFS QSWEEP", "a=ct/lun/op b=ocs/typ/res/echo");
	for (unsigned i = 0; i < sizeof(g) / sizeof(g[0]); i++) {
		uint32_t r = ufssend(g[i].ct, 0x16, g[i].lun, g[i].op, 0x01, 0);
		con_hex2("UFS Q", "a=",
		    (uint32_t)g[i].ct << 24 | (uint32_t)g[i].lun << 16 | (uint32_t)g[i].op << 8 | 0x01,
		    "b=", r);
	}
	/* NOP OUT handshake: a live UTP answers with NOP IN (0x20). */
	uint32_t r = ufssend(1, 0x00, 0, 0, 0, 0);
	con_hex2("UFS NOP", "ocs/typ=", r >> 16, "res/echo=", r & 0xffff);
	/* REPORT LUNs: what LUs does the device admit to? */
	static const uint8_t rl12[12] = { 0xa0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0 };
	uint32_t ocs = 0, st = 0;
	uint32_t rc = scsi(0, rl12, sizeof(rl12), data, 16, &ocs, &st);
	con_hex2("UFS RPTLUN", "rc=", rc, "ocs/st=", ocs << 8 | st);
	if (!rc)
		con_hex2("UFS LUNS", "len=", be32(data), "e1hi=", be32(data + 8));
}

static void settle_ms(unsigned ms)
{
	uint64_t start = ticks();
	while (ticks() - start < (uint64_t)ms * 19200)
		;
}

/* Name the filesystem whose superblock starts at block lba (f2fs/ext4). */
/*
 * The UniXNU wake (SM8550 sheng, hardware 2026-10-17): SSU power condition
 * ACTIVE to the device W-LU, the Linux ufshcd_set_dev_pwr_mode resume path.
 * Their ABL warm link showed the identical all-LUs-NOT-READY picture and this
 * one command cured it. SSU changes device power state only; no user data is
 * touched. Retries once per unit attention (sense key 6).
 */
static uint32_t ssu_wake(uint8_t lun, uint32_t *ocs_out, uint32_t *st_out)
{
	static const uint8_t ssu[6] = { 0x1b, 0, 0, 0, 0x10, 0 };	/* byte 4: power cond ACTIVE */
	uint32_t rc = 0, ocs = 0, st = 0;
	for (int i = 0; i < 3; i++) {
		rc = scsi(lun, ssu, sizeof(ssu), 0, 0, &ocs, &st);
		if (rc != 4 || (last_sense >> 16) != 6)
			break;
		settle_ms(10);
	}
	*ocs_out = ocs;
	*st_out = st;
	return rc;
}

/* SSU wake, then re-probe everything the old rounds found dead. */
static void wake_and_reprobe(void)
{
	uint32_t ocs = 0, st = 0, rc;

	con_str("UFS WAKE", "ssu active -> 0xd0 then 0x50 (uniXNU)");
	rc = ssu_wake(0xd0, &ocs, &st);
	con_hex2("UFS W", "d0 rc=", rc, "ocs/st=", ocs << 8 | st);
	con_hex2("UFS W", "d0 sense=", last_sense, 0, 0);
	rc = ssu_wake(0x50, &ocs, &st);
	con_hex2("UFS W", "50 rc=", rc, "ocs/st=", ocs << 8 | st);
	con_hex2("UFS W", "50 sense=", last_sense, 0, 0);
	settle_ms(50);

	static const uint8_t tur[6] = { 0x00 };
	int green = 0;
	for (int i = 0; i < 5; i++) {
		rc = scsi(0, tur, sizeof(tur), 0, 0, &ocs, &st);
		con_hex2("UFS T2", "try=", i, "rc=", rc);
		con_hex2("UFS T2", "ocs/st=", ocs << 8 | st, "sense=", last_sense);
		if (!rc) {
			green = 1;
			break;
		}
		settle_ms(10);
	}

	/* Did the wake open the query engine too? Proper function byte this time. */
	uint32_t r0 = ufssend(1, 0x16, 0x00, 0x05, 0x01, 0x01);	/* READ fDeviceInit */
	uint32_t rd = ufssend(1, 0x16, 0xd0, 0x05, 0x01, 0x01);
	con_hex2("UFS Q2", "lun0=", r0, "wd0=", rd);
	uint32_t ra = ufssend(1, 0x16, 0x00, 0x03, 0x02, 0x01);	/* READ bCurrentPowerMode */
	con_hex2("UFS Q2", "pwr res=", ra, "val=", qs_val);
	if (green)
		return;

	/* Query alive and fDeviceInit pending: the standard init dance (0083 shape). */
	if ((r0 >> 24) == 0 && (r0 >> 16 & 0xff) == 0x22 && (r0 >> 8 & 0xff) == 0) {
		uint32_t set = ufssend(1, 0x16, 0x00, 0x06, 0x01, 0x02);	/* SET fDeviceInit = 1 */
		con_hex2("UFS Q2", "set res=", set, "val=", qs_val);
		for (int i = 0; i < 100; i++) {
			uint32_t r = ufssend(1, 0x16, 0x00, 0x05, 0x01, 0x01);
			if ((r >> 24) == 0 && (r >> 16 & 0xff) == 0x22 && (r >> 8 & 0xff) == 0 &&
			    (qs_val & 1) == 0) {
				con_hex2("UFS Q2", "cleared@ms=", i * 10, "r=", r);
				break;
			}
			settle_ms(10);
		}
		for (int i = 0; i < 3; i++) {
			rc = scsi(0, tur, sizeof(tur), 0, 0, &ocs, &st);
			con_hex2("UFS T3", "rc=", rc, "ocs/st=", ocs << 8 | st);
			con_hex2("UFS T3", "sense=", last_sense, 0, 0);
			if (!rc)
				break;
			settle_ms(10);
		}
	}
}

static uint64_t le64(const uint8_t *p)
{
	uint64_t v = 0;
	for (int i = 7; i >= 0; i--)
		v = v << 8 | p[i];
	return v;
}

static const char *fs_type(uint32_t lba)
{
	if (read_blocks(lba, 1, data))
		return "read error";
	const uint8_t *sb = data + 1024;
	if (sb[0] == 0x10 && sb[1] == 0x20 && sb[2] == 0xf5 && sb[3] == 0xf2)
		return "f2fs";
	if (sb[56] == 0x53 && sb[57] == 0xef)
		return "ext4";
	return "other";
}

/* One line per GPT entry of at least 64 MiB: name, first LBA, size in MiB (4K blocks). */
static uint64_t ud_first, ud_nblocks;	/* userdata extent, 4K blocks (for the log dump) */

static void list_partitions(uint32_t entry_lba, uint32_t nent)
{
	static uint8_t ents[128 * 128] __attribute__((aligned(4096)));
	uint32_t blocks = (nent * 128 + BLOCK - 1) / BLOCK;
	if (read_blocks(entry_lba, blocks, ents)) {
		con_str("UFS PART", "read error");
		return;
	}
	for (uint32_t i = 0; i < nent; i++) {
		const uint8_t *e = ents + i * 128;
		uint64_t first = le64(e + 32), last = le64(e + 40);
		if (!first)
			continue;
		char name[17];
		for (int c = 0; c < 16; c++) {
			uint8_t ch = e[56 + 2 * c];
			name[c] = (ch >= 32 && ch < 127) ? ch : (ch ? '?' : 0);
		}
		name[16] = 0;
		if (!__builtin_strcmp(name, "userdata")) {
			ud_first = first;
			ud_nblocks = last - first + 1;
		}
		if (((last - first + 1) >> 8) < 64)	/* keep the screen to the big ones */
			continue;
		con_hex2(name, "lba=", (uint32_t)first, "MiB=", (uint32_t)((last - first + 1) >> 8));
		if (!__builtin_strcmp(name, "userdata") || !__builtin_strcmp(name, "cust") ||
		    !__builtin_strcmp(name, "cache"))
			con_str(" fs", fs_type((uint32_t)first));
	}
}

/* --- SWEETLOG dump ---------------------------------------------------------
 * XNU patch 0047 mirrors the console stream into a 4 MiB ring it flushes to
 * the last 4 MiB + 4 KiB of userdata: device block at (tail - 1024) holds the
 * header "SWEETLOG" | u32 ver | u32 ring | u64 total | u64 seq, the ring
 * follows. This probe boot is the only way to read it back on M2381 (no
 * Android side): print the lines that matter, oldest first.
 */
static int line_has(const char *line, const char *sub)
{
	for (; *line; line++) {
		const char *a = line, *b = sub;
		while (*b && *a == *b) { a++; b++; }
		if (!*b)
			return 1;
	}
	return 0;
}

static void dump_sweetlog(void)
{
	static uint8_t buf[64 * BLOCK] __attribute__((aligned(4096)));
	static const char keys[][17] = {
		"IPS|", "SWEET HID", "SWEET SURF", "SWEET FB", "SWEET LOG",
		"AppleCLCD", "exit_with_reason", "backboardd", "SpringBoard",
		"ReportCrash", "STR ", "CRASH ", "exception", "Terminating",
	};

	if (!ud_nblocks) {
		con_str("LOG", "userdata not seen");
		return;
	}
	/* The hardware probe above already had its photo: this dump owns the screen. */
	con_clear(BLUE);
	con_row_set(0);
	uint32_t base = (uint32_t)(ud_first + ud_nblocks - (1 + (4u << 20) / BLOCK));
	if (read_blocks(base, 1, buf)) {
		con_str("LOG", "header read error");
		return;
	}
	if (__builtin_memcmp(buf, "SWEETLOG", 8)) {
		con_str("LOG", "no SWEETLOG magic (0047 never flushed)");
		con_hex2("LOG", "b04=", *(uint32_t *)buf, "b48=", *(uint32_t *)(buf + 4));
		return;
	}
	uint32_t ver, ring;
	uint64_t total, seq;
	__builtin_memcpy(&ver, buf + 8, 4);
	__builtin_memcpy(&ring, buf + 12, 4);
	__builtin_memcpy(&total, buf + 16, 8);
	__builtin_memcpy(&seq, buf + 24, 8);
	con_hex2("LOG", "ver=", ver, "ring=", ring);
	con_hex2("LOG", "total=", (uint32_t)total, "seq=", (uint32_t)seq);
	if (!total) {
		con_str("LOG", "empty");
		return;
	}

	uint64_t start = total > ring ? total - ring : 0;
	char line[112];
	int len = 0, lines = 0;

	/* The kernel's own "IPS| " mirror lines are the payload: pre-scan them
	 * first so the event cap below can never push them off screen. */
	lines = 0;
	for (uint64_t pos = start; pos < total && lines < 40; ) {
		uint32_t off = (uint32_t)(pos % ring);
		uint32_t skip = off % BLOCK;
		uint32_t n = ring - off;
		if (n > sizeof(buf) - BLOCK) n = sizeof(buf) - BLOCK;
		if (n > total - pos) n = (uint32_t)(total - pos);
		uint32_t raw = (skip + n + BLOCK - 1) / BLOCK * BLOCK;
		if (read_blocks((uint32_t)(base + 1 + off / BLOCK), raw / BLOCK, buf)) {
			con_str("LOG", "ring read error");
			return;
		}
		for (uint32_t i = skip; i < skip + n && lines < 40; i++) {
			char c = buf[i];
			if (c == '\r')
				continue;
			if (c != '\n') {
				if (len < (int)sizeof(line) - 1)
					line[len++] = c;
				continue;
			}
			line[len] = 0;
			if (len && line_has(line, "IPS|")) {
				con_str("I", line);
				lines++;
			}
			len = 0;
		}
		pos += n;
	}
	if (lines == 0)
		con_str("I", "no IPS| lines in ring");

	len = 0;
	lines = 0;
	for (uint64_t pos = start; pos < total && lines < 30; ) {
		uint32_t off = (uint32_t)(pos % ring);
		uint32_t skip = off % BLOCK;
		uint32_t n = ring - off;
		if (n > sizeof(buf) - BLOCK) n = sizeof(buf) - BLOCK;
		if (n > total - pos) n = (uint32_t)(total - pos);
		uint32_t raw = (skip + n + BLOCK - 1) / BLOCK * BLOCK;
		if (read_blocks((uint32_t)(base + 1 + off / BLOCK), raw / BLOCK, buf)) {
			con_str("LOG", "ring read error");
			return;
		}
		for (uint32_t i = skip; i < skip + n && lines < 34; i++) {
			char c = buf[i];
			if (c == '\r')
				continue;
			if (c != '\n') {
				if (len < (int)sizeof(line) - 1)
					line[len++] = c;
				continue;
			}
			line[len] = 0;
			if (len)
				for (unsigned k = 0; k < sizeof(keys) / sizeof(keys[0]); k++)
					if (line_has(line, keys[k])) {
						con_str("L", line);
						lines++;
						break;
					}
			len = 0;
		}
		pos += n;
	}
	if (len) { line[len] = 0; con_str("L+", line); }
	con_hex2("LOG", "lines=", lines, "from=", (uint32_t)start);

	/* Raw tail: the words right before the flush stopped (abort text, panic
	 * context) never match the keyword list, so show them verbatim. Two
	 * passes over the window: count lines, then print only the last 8. */
	uint64_t tail = total > 64 * 256 ? total - 64 * 256 : start;
	uint32_t nl = 0;
	len = 0;
	for (uint64_t pos = tail; pos < total; ) {
		uint32_t off = (uint32_t)(pos % ring);
		uint32_t skip = off % BLOCK;
		uint32_t n = ring - off;
		if (n > sizeof(buf) - BLOCK) n = sizeof(buf) - BLOCK;
		if (n > total - pos) n = (uint32_t)(total - pos);
		uint32_t raw = (skip + n + BLOCK - 1) / BLOCK * BLOCK;
		if (read_blocks((uint32_t)(base + 1 + off / BLOCK), raw / BLOCK, buf)) {
			con_str("TAIL", "ring read error");
			return;
		}
		for (uint32_t i = skip; i < skip + n; i++)
			if (buf[i] == '\n')
				nl++;
		pos += n;
	}
	uint32_t first = nl > 8 ? nl - 8 : 0;
	uint32_t idx = 0;
	lines = 0;
	len = 0;
	/* Second pass over the same window: buf still holds it whole (the window
	 * is one chunk, far below buf size), so no re-read. */
	for (uint64_t pos = tail; pos < total && lines < 8; ) {
		uint32_t off = (uint32_t)(pos % ring);
		uint32_t skip = off % BLOCK;
		uint32_t n = ring - off;
		if (n > sizeof(buf) - BLOCK) n = sizeof(buf) - BLOCK;
		if (n > total - pos) n = (uint32_t)(total - pos);
		for (uint32_t i = skip; i < skip + n && lines < 8; i++) {
			char c = buf[i];
			if (c == '\r')
				continue;
			if (c != '\n') {
				if (len < (int)sizeof(line) - 1)
					line[len++] = c;
				continue;
			}
			if (idx >= first && len) {
				line[len] = 0;
				con_str("T", line);
				lines++;
			}
			idx++;
			len = 0;
		}
		pos += n;
	}
	if (len && idx >= first) { line[len] = 0; con_str("T+", line); }
	con_str("LOG", "done");
}

void test_ufs(void)
{
	con_str("UFS", "v6 ssu-wake probe (read-only)");
	uint32_t cap = ufs_rd(REG_CAP), hcs = ufs_rd(REG_HCS), hce = ufs_rd(REG_HCE);
	con_hex2("UFS", "ver=", ufs_rd(REG_VER), "cap=", cap);
	con_hex2("UFS", "hce=", hce, "hcs=", hcs);
	con_hex2("UFS", "is=", ufs_rd(REG_IS), "rsr=", ufs_rd(REG_UTRLRSR));
	con_hex2("UFS", "utrlba=", ufs_rd(REG_UTRLBA), "hi=", ufs_rd(REG_UTRLBAU));
	con_hex2("UFS", "dbr=", ufs_rd(REG_UTRLDBR), "nutrs=", (cap & 0x1f) + 1);

	if (!(hce & 1) || !(hcs & HCS_DP) || !(hcs & HCS_UTRLRDY)) {
		con_str("UFS", "host not enabled/ready: stop");
		return;
	}
	if (ufs_rd(REG_UTRLDBR)) {
		con_str("UFS", "requests outstanding: stop");
		return;
	}

	/* Take over the transfer request list: stop, point at ours, run. */
	ufs_wr(REG_UTRLRSR, 0);
	uintptr_t list = (uintptr_t)utrl;
	ufs_wr(REG_UTRLBA, (uint32_t)list);
	ufs_wr(REG_UTRLBAU, (uint32_t)(list >> 32));
	ufs_wr(REG_UTRLRSR, 1);
	con_hex2("UFS", "ours=", (uint32_t)list, "rsr=", ufs_rd(REG_UTRLRSR));
	/*
	 * Never ring a doorbell unless the host really uses our list: otherwise
	 * it would execute whatever ABL left in its own slot 0 (maybe a write).
	 */
	if (ufs_rd(REG_UTRLBA) != (uint32_t)list || ufs_rd(REG_UTRLBAU) != (uint32_t)(list >> 32) ||
	    !(ufs_rd(REG_UTRLRSR) & 1)) {
		con_str("UFS", "list base not taken: stop");
		return;
	}

	uint32_t ocs = 0, st = 0, rc;
	static const uint8_t tur[6] = { 0x00 };
	/* A first command may report a unit attention; retry a few times. */
	for (int i = 0; i < 3; i++) {
		rc = scsi(0, tur, sizeof(tur), 0, 0, &ocs, &st);
		con_hex2("UFS TUR", "rc=", rc, "ocs/st=", ocs << 8 | st);
		con_hex2("UFS TUR", "sense=", last_sense, 0, 0);
		if (!rc)
			break;
	}

	query_sweep();
	wake_and_reprobe();

	static const uint8_t cap10[10] = { 0x25 };	/* READ CAPACITY(10) */
	rc = scsi(0, cap10, sizeof(cap10), data, 8, &ocs, &st);
	con_hex2("UFS CAP", "rc=", rc, "ocs/st=", ocs << 8 | st);
	if (!rc)
		con_hex2("UFS LUN0", "lastlba=", be32(data), "blk=", be32(data + 4));

	/* READ(10) LBA 0, 2 blocks: protective MBR and the GPT header. */
	static const uint8_t rd10[10] = { 0x28, 0, 0, 0, 0, 0, 0, 0, 2, 0 };
	rc = scsi(0, rd10, sizeof(rd10), data, 2 * BLOCK, &ocs, &st);
	con_hex2("UFS READ", "rc=", rc, "ocs/st=", ocs << 8 | st);
	if (rc)
		return;
	con_hex2("UFS MBR", "sig=", data[510] | data[511] << 8, "type=", data[450]);
	const uint8_t *gpt = data + BLOCK;
	char sig[9];
	for (int i = 0; i < 8; i++)
		sig[i] = (gpt[i] >= 32 && gpt[i] < 127) ? gpt[i] : '.';
	sig[8] = 0;
	con_str("UFS GPT", sig);
	uint32_t nent = gpt[80] | gpt[81] << 8 | gpt[82] << 16, esz = gpt[84] | gpt[85] << 8;
	uint32_t elba = gpt[72] | gpt[73] << 8;
	con_hex2("UFS GPT", "entries=", nent, "lba=", elba);
	if (esz != 128 || nent > 128)
		return;
	list_partitions(elba, nent);
	dump_sweetlog();
}
