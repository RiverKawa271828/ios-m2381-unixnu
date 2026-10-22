/** @file UfsDeviceInitDxe.c
  fDeviceInit + self-read kernel delivery for the Meizu 20 post-ABL world.

  R23bb rewrite (2026-09-26).  Three things changed vs the R23ba version, each
  backed by evidence gathered in the 早29 review:

  1. UTRD response/prdt fields are in DOUBLE WORDS, not bytes.
     The whole "device ready" story of R23ax..R23ba was built on reads of a
     buffer the device never wrote: the DW6/DW7 fields were programmed with
     byte values (length 64, offset 512), so the controller wrote the response
     UPIU at UCD+2048 while we read UCD+512 - our own zeroed scratch.  Kernel
     authority: drivers/ufs/core/ufshcd.c stores response_upiu_offset,
     prd_table_offset and response_upiu_length >> 2 (only Samsung Exynos hosts
     set UFSHCD_QUIRK_PRDT_BYTE_GRAN), and prd_table_length holds the PRDT
     *entry count*.  With this fix the response lands at UCD+512, which is where
     we read it, and every "Success" from here on is backed by real bytes.

  2. No HCE reset, no UFSDxe relaunch.
     R23ba's HCE 0->1 dropped HCS from 0x10F to 0x8 (DEVICE_PRESENT=0: the link
     was dead) and the relaunched UFSDxe then failed at transport level (three
     Device Error asserts, no SCSI sense - the signature of a dead link).  The
     kernel's own sequence is HCE -> DME_LINK_STARTUP -> NOP OUT -> SET
     fDeviceInit; the OEM UFSDxe assumes XBL already did the link part.  We
     therefore never touch the controller configuration: we ride UFSDxe's
     running UTRL list, and after fDeviceInit we deliver the kernel OURSELVES
     instead of asking UFSDxe to try again.

  3. The delivery path is now self-contained: Query fDeviceInit -> read the
     GPT of the boot LUN -> locate boot_b -> read its Android boot header ->
     stream the ramdisk field (our vmlinuz.efi, raw PE, MZ+zimg) into a staging
     buffer -> LoadImage -> StartImage.  No BlockIo, no BDS, no ConnectAll.

  Still true from earlier rounds:
  - NEVER reprogram RSR/UTRLBA on the running controller (R23ap/aq silent
    hangs).  Ride XBL/UFSDxe's list; write only into UFSDxe's own list page
    (XBL's page is not even mapped in Mu: R23av hang, R23aw abort).
  - IS is write-1-to-clear: only ever write the bit(s) we own, never RMW
    (R23ax wiped pending events with MmioOr32).
  - The fDeviceInit flag lives in the high byte of the little-endian u32 we
    read at query value offset +20: (raw >> 24) & 1, i.e. the kernel's
    (be32_to_cpu(v) & 0xFF) & 1.
  - Cache maintenance: UTRD/UCD clean+invalidate before the doorbell,
    invalidate after (device DMA wrote them); data buffers invalidated before
    and after (read direction).

  All constants transcribed from silime-linux include/ufs/ufshci.h,
  include/ufs/ufs.h, include/uapi/scsi/scsi_bsg_ufs.h,
  include/linux/efi.h (DEVICE_TREE_GUID) - never from memory.

  Copyright (c) 2026, meizu20 mainline project. SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/IoLib.h>
#include <Protocol/Cpu.h>
#include <Pi/PiFirmwareFile.h>

//
// UFSHCI register offsets (include/ufs/ufshci.h, enum ufshci_mmio_offset)
//
#define UFSHCI_CAP        0x00
#define UFSHCI_VER        0x08
#define UFSHCI_IS         0x20
#define UFSHCI_IE         0x24
#define UFSHCI_HCS        0x30
#define UFSHCI_CE         0x34
#define UFSHCI_UTRLBA_L   0x50
#define UFSHCI_UTRLBA_H   0x54
#define UFSHCI_UTRLDBR    0x58
#define UFSHCI_UTRL_CLEAR 0x5C
#define UFSHCI_UTRL_RSR   0x60
#define UFSHCI_UTMRLBA_L  0x70
#define UFSHCI_UTMRLBA_H  0x74
#define UFSHCI_UTMRDBR    0x78
#define UFSHCI_UTMRL_RSR  0x80
#define UFSHCI_UICC       0x90
#define UFSHCI_UICC_ARG1  0x94
#define UFSHCI_UICC_ARG2  0x98
#define UFSHCI_UICC_ARG3  0x9C

// HCS = REG_CONTROLLER_STATUS (ufshci.h: DEVICE_PRESENT 0x1, UTRL_RDY 0x2,
// UTMR_RDY 0x4, UIC_CMD_RDY 0x8)
#define HCS_DEVICE_PRESENT 0x1
#define HCS_LISTS_READY    0xE

//
// NOTE (R23bd verdict, on-device): the UFS reset line IS TLMM GPIO 210
// (confirmed against the *live* stock FDT: reset-gpios = <&tlmm 0xd2
// GPIO_ACTIVE_LOW> on ufshc@1d84000), but DRIVING TLMM REGISTERS FROM THIS
// POST-EBS DXE WORLD IS FATAL: the R23bd build read GPIO 210's page
// (ctl 0x3FF io 0xFFFF - already suspicious: not a plain GPIO output) and then
// wrote CTL; the SoC died on the spot and fell into EDL (900e).  Do NOT touch
// TLMM from here.  The pin is reset by the (properly clocked/pinctrl'd) Linux
// kernel when it probes ufs_qcom, or by XBL/ABL before us.


#define UFSHCI_BASE       0x01D84000UL
#define UfsReg32(off)     MmioRead32 (UFSHCI_BASE + (off))

//
// Bit/constant definitions (ufshci.h / ufs.h)
//
#define IS_UTP_TRANSFER_COMPL     0x1       // UTP_TRANSFER_REQ_COMPL
#define IS_UIC_COMMAND_COMPL      0x400     // bit10
#define UIC_CMD_DME_HIBER_EXIT    0x18      // enum uic_cmd_dme
#define UIC_CMD_DME_LINK_STARTUP  0x16      // enum uic_cmd_dme
#define MASK_UIC_RESULT           0xFF
#define OCS_SUCCESS               0x0
#define OCS_INVALID               0x0F

//
// HCS bits (ufshci.h): 0x1 DEVICE_PRESENT, 0x2 UTRL_RDY, 0x4 UTMR_RDY,
// 0x8 UIC_CMD_RDY.  Printed so the log always shows link state.
//

//
// UPIU (uffs.h / scsi_bsg_ufs.h)
//
#define UPIU_TRANSACTION_COMMAND  0x01
#define UPIU_TRANSACTION_QUERY_REQ 0x16
#define UPIU_CMD_FLAGS_READ       0x40      // UPIU_CMD_FLAGS_READ
#define UPIU_COMMAND_SET_TYPE_SCSI 0x00

#define UPIU_QUERY_FUNC_READ      0x01      // UPIU_QUERY_FUNC_STANDARD_READ_REQUEST
#define UPIU_QUERY_FUNC_WRITE     0x81      // UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST
#define UPIU_QUERY_OPCODE_READ_DESC  0x01
#define UPIU_QUERY_OPCODE_READ_FLAG  0x05
#define UPIU_QUERY_OPCODE_SET_FLAG   0x06
#define QUERY_DESC_IDN_DEVICE     0x00
#define QUERY_DESC_IDN_CONFIGURATION 0x01
#define QUERY_DESC_IDN_UNIT       0x02
#define QUERY_DESC_IDN_HEALTH     0x09
#define QUERY_ATTR_IDN_POWER_MODE 0x02   // ufs.h QUERY_ATTR_IDN_POWER_MODE
#define QUERY_ATTR_IDN_ACTIVE_ICC_LVL 0x03
#define QUERY_ATTR_IDN_REF_CLK_FREQ   0x0A
#define UPIU_QUERY_OPCODE_READ_ATTR   0x03
#define UPIU_QUERY_OPCODE_WRITE_ATTR  0x04
#define QUERY_FLAG_IDN_FDEVICEINIT 0x01

//
// R23bg: the two device-side state movers the kernel relies on when a device is
// not active (coordinates from ufs.h / ufshcd.c).
//
#define UPIU_TRANSACTION_NOP_OUT  0x00      // no-data liveness probe (NOP IN back)
#define WLUN_UFS_DEVICE           0xD0      // ufs.h:64 UFS_UPIU_WLUN_ID(0x80) | 0x50
#define SCSI_START_STOP_UNIT      0x1B      // SPC START STOP UNIT
#define SSU_PWR_COND_ACTIVE       0x01      // power condition 1 = Active (cdb[4] = 1<<4)

// Query response value is a BE32 field; we read it raw (little-endian CPU) so
// the flag byte the kernel extracts as (be32_to_cpu(v) & 0xFF) & 1 sits in the
// HIGH byte here.
#define FDEVICEINIT_BIT(v)        (((UINT32)(v) >> 24) & 0x1)

// Device/unit descriptor field offsets (ufs.h enum device_desc_param /
// unit_desc_param)
#define DEV_DESC_NUM_LU           0x06
#define DEV_DESC_ACTVE_ICC_LVL    0x0F
#define DEV_DESC_SPEC_VER         0x10
#define DEV_DESC_MANF_ID          0x18
#define UNIT_DESC_LU_ENABLE       0x03
#define UNIT_DESC_BLK_SIZE        0x0A
#define UNIT_DESC_BLK_COUNT       0x0B

//
// UCD layout (kernel struct utp_transfer_cmd_desc): command UPIU at 0,
// response UPIU at 512 (ALIGNED_UPIU_SIZE), PRDT right after it.
// UTRD DW6/DW7 fields are in double words; prd_table_length is the entry count.
//
#define RSP_UPIU_OFFSET           512
#define PRDT_OFFSET               1024
#define UCD_CLEAR_SIZE            0x800
#define RESP_LEN_DW               (RSP_UPIU_OFFSET / 4)
#define RESP_OFF_DW               (RSP_UPIU_OFFSET / 4)
#define PRDT_OFF_DW               (PRDT_OFFSET / 4)

#define UTRD_CT_UFS_STORAGE       0x1
#define UTP_DEVICE_TO_HOST        0x2       // utp_data_direction (read)
#define UTP_HOST_TO_DEVICE        0x1
#define UTP_NO_DATA_TRANSFER      0x0

#define SCRATCH_PAGES             16        // 64 KB: GPT entries / boot header
#define READ_CHUNK                0x20000   // 128 KB per transfer (< 256 KB PRDT max)
#define STAGE_BYTES_MAX           0x00DE6200UL  // vmlinuz.efi size (0xde6200)

//
// GPT (UEFI spec, LBA-relative: header at LBA 1, entries at LBA 2)
//
#define GPT_SIG                   "EFI PART"
#define GPT_ENTRIES_LBA_OFF       0x48      // u64
#define GPT_ENTRY_COUNT_OFF       0x50      // u32
#define GPT_ENTRY_SIZE_OFF        0x54      // u32
#define GPT_ENTRY_FIRST_LBA_OFF   0x20      // u64
#define GPT_ENTRY_LAST_LBA_OFF    0x28      // u64
#define GPT_ENTRY_NAME_OFF        0x38      // 36 x UTF-16LE

//
// Android boot image header v3/v4 (page size fixed at 4096)
//
#define BOOT_MAGIC                "ANDROID!"
#define BOOT_KERNEL_SIZE_OFF      0x08      // u32
#define BOOT_RAMDISK_SIZE_OFF     0x0C      // u32
#define BOOT_PAGE_SIZE            4096

// Budgets sized to ALWAYS print before the ~60s watchdog fires.  One MmioRead32
// poll ~ a few hundred ns; 2M loops ~1s worst case.
#define WAIT_LOOPS                2000000UL
#define FLAG_CLEAR_POLLS          300       // 300 x ~3 ms ~ 1 s (kernel: 1500 ms)

// FDT configuration table GUID = kernel include/linux/efi.h:382
// DEVICE_TREE_GUID (consumed by drivers/firmware/efi/libstub/fdt.c:368).
STATIC EFI_GUID  mFdtTableGuid = { 0xB1B621D5, 0xF19C, 0x41A5, { 0x83, 0x0B, 0xD9, 0x15, 0x2C, 0x69, 0xAA, 0xE0 } };
// EmbeddedPkg gDtPlatformDefaultDtbFileGuid: the FV RAW section holding our DTB.
STATIC EFI_GUID  mDtbFileGuid  = { 0x25462CDA, 0x221F, 0x47DF, { 0xAC, 0x1D, 0x25, 0x9C, 0xFA, 0xA4, 0xE3, 0x26 } };

typedef struct {
  UINT8   Opcode;
  UINT8   Idn;
  UINT8   Index;
  UINT8   Selector;
  UINT16  ReservedOsf;      // BE
  UINT16  Length;           // BE
  UINT32  Value;            // BE
  UINT32  Reserved[2];
} UFS_QUERY_REQ;

STATIC UINT8   *gUcd;          // 4 KB command descriptor (UPIUs + PRDT)
STATIC UINT8   *gScratch;      // 64 KB scratch for GPT/boot header
STATIC UINT8   *gStage;        // staging buffer for the kernel ramdisk field
STATIC UINT64   gXblUtrlBase;
STATIC UINT32   gSavedIe;
STATIC UINT32   gSlot;         // UTR slot we send through
STATIC UINT32   gBlockSize;    // logical block size of the LUN we read

STATIC EFI_CPU_ARCH_PROTOCOL *mCpu;

STATIC
EFI_STATUS
MapRangeUncached (
  IN UINT64 Phys,
  IN UINT64 Length)
{
  EFI_PHYSICAL_ADDRESS Page = Phys & ~(EFI_PHYSICAL_ADDRESS)0xFFF;
  UINT64                End  = (Phys + Length + 0xFFF) & ~(UINT64)0xFFF;

  if (mCpu == NULL) {
    return EFI_NOT_READY;
  }

  return mCpu->SetMemoryAttributes (mCpu, Page, End - Page, EFI_MEMORY_UC);
}

STATIC
VOID
Dump8 (
  IN CONST CHAR8  *Tag,
  IN CONST UINT8  *Ptr)
{
  CONST UINT32 *W = (CONST UINT32 *)(CONST VOID *)Ptr;

  DEBUG ((DEBUG_ERROR, "UFS-DevInit: %a %08x %08x %08x %08x %08x %08x %08x %08x\n",
          Tag, W[0], W[1], W[2], W[3], W[4], W[5], W[6], W[7]));
}

STATIC
VOID
Dump4 (
  IN CONST CHAR8  *Tag,
  IN CONST UINT8  *Ptr)
{
  CONST UINT32 *W = (CONST UINT32 *)(CONST VOID *)Ptr;

  DEBUG ((DEBUG_ERROR, "UFS-DevInit: %a %08x %08x %08x %08x\n",
          Tag, W[0], W[1], W[2], W[3]));
}

// Attribute values are BE32 on the wire; print the semantic value.
STATIC
UINT32
Swap32 (
  IN UINT32 V)
{
  return ((V & 0xFF) << 24) | ((V & 0xFF00) << 8) |
         ((V >> 8) & 0xFF00) | ((V >> 24) & 0xFF);
}

//
// bCurrentPowerMode (QUERY_ATTR_IDN_POWER_MODE), JESD220C Table 7-10:
// 0x00 idle / 0x10 pre-active / 0x11 ACTIVE / 0x20 pre-sleep / 0x22 sleep /
// 0x30 pre-powerdown / 0x33 UFS-PowerDown.  Whatever the device's own
// bInitPowerMode (device descriptor 0x0A) asks for is where it must land after
// initialization; while it sits in a PowerDown/Sleep family mode it answers
// queries but refuses SET fDeviceInit (GENERAL_FAILURE 0xFF) and refuses every
// LU data command (NOT READY / INITIALIZING COMMAND REQUIRED, ASC/ASCQ 04/02).
//
STATIC
CONST CHAR8 *
PowerModeName (
  IN UINT8 Mode
  )
{
  switch (Mode) {
    case 0x00: return "idle";
    case 0x10: return "pre-active";
    case 0x11: return "ACTIVE";
    case 0x20: return "pre-sleep";
    case 0x22: return "sleep";
    case 0x30: return "pre-powerdown";
    case 0x33: return "POWERDOWN";
    default:   return "reserved";
  }
}

//
// All 18 sense bytes (R23bf printed only the first 8, which hid ASC/ASCQ).
// Layout as this device answers: response UPIU +32 = sense length (BE16),
// +34.. = sense data: [2] sense key, [7] additional length, [12]/[13] ASC/ASCQ.
//
STATIC
VOID
DumpSense (
  IN CONST CHAR8  *Tag,
  IN UINT8        *Rsp
  )
{
  UINT16  Len = (UINT16)((Rsp[32] << 8) | Rsp[33]);
  UINT8  *S   = Rsp + 34;

  DEBUG ((DEBUG_ERROR,
          "UFS-DevInit: %a sense len %u: %02x %02x %02x %02x %02x %02x %02x %02x %02x"
          " %02x %02x %02x %02x %02x %02x %02x %02x %02x key 0x%x asc/ascq 0x%x/0x%x\n",
          Tag, Len,
          S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], S[8],
          S[9], S[10], S[11], S[12], S[13], S[14], S[15], S[16], S[17],
          S[2], S[12], S[13]));
}

STATIC
UINT64
BeN (
  IN CONST UINT8  *P,
  IN UINTN         N)
{
  UINT64  V = 0;
  UINTN   i;

  for (i = 0; i < N; i++) {
    V = (V << 8) | P[i];
  }

  return V;
}

//
// Memory budget: total free pages + largest contiguous free run.  The endgame
// needs ~28.5 MB (14.57 MB staging + 13.9 MB kernel image), and R23ba proved
// the 72.3 MB DXE pool runs dry at the tail of dispatch, so measure it.
//
STATIC
VOID
DumpMemStats (
  VOID
  )
{
  EFI_STATUS            Status;
  UINTN                 Size = 0;
  UINTN                 Key;
  UINTN                 DescSize;
  UINT32                DescVer;
  EFI_MEMORY_DESCRIPTOR *Map;
  EFI_MEMORY_DESCRIPTOR *D;
  UINTN                 i;
  UINT64                FreePages = 0;
  UINT64                RunPages = 0;
  UINT64                BestRun = 0;
  UINT64                LargestFreeStart = 0;

  gBS->GetMemoryMap (&Size, NULL, &Key, &DescSize, &DescVer);
  Size += 4 * DescSize;
  Map   = AllocatePool (Size);
  if (Map == NULL) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: mem stats alloc failed\n"));
    return;
  }

  Status = gBS->GetMemoryMap (&Size, Map, &Key, &DescSize, &DescVer);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: GetMemoryMap %r\n", Status));
    FreePool (Map);
    return;
  }

  for (i = 0, D = Map; i < Size / DescSize; i++) {
    D = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)Map + i * DescSize);
    if (D->Type == EfiConventionalMemory) {
      FreePages += D->NumberOfPages;
      if (D->PhysicalStart == LargestFreeStart + RunPages * EFI_PAGE_SIZE) {
        RunPages += D->NumberOfPages;
      } else {
        RunPages = D->NumberOfPages;
        LargestFreeStart = D->PhysicalStart;
      }
      if (RunPages > BestRun) {
        BestRun = RunPages;
      }
    } else {
      RunPages = 0;
    }
  }

  DEBUG ((DEBUG_ERROR,
          "UFS-DevInit: mem free %lu pages (%lu.%lu MB) largest free run %lu pages"
          " (%lu.%lu MB)\n",
          FreePages, (FreePages * EFI_PAGE_SIZE * 10) / (1024 * 1024) / 10,
          ((FreePages * EFI_PAGE_SIZE * 10) / (1024 * 1024)) % 10,
          BestRun, (BestRun * EFI_PAGE_SIZE * 10) / (1024 * 1024) / 10,
          ((BestRun * EFI_PAGE_SIZE * 10) / (1024 * 1024)) % 10));

  FreePool (Map);
}

//
// Read-only reconnaissance of the controller's transfer state (R23au).
//
STATIC
VOID
ProbeControllerState (
  OUT BOOLEAN *SafeToSend)
{
  UINT32  D, I, Slot, NonZero = 0;
  UINT32 *W;

  *SafeToSend = FALSE;

  D = UfsReg32 (UFSHCI_UTRLDBR);
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: DBR 0x%08x\n", D));

  I = UfsReg32 (UFSHCI_IS);
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: IS 0x%08x\n", I));

  W = (UINT32 *)(UINTN)gXblUtrlBase;
  Dump8 ("slot0", (CONST UINT8 *)W);

  // This list belongs to UFSDxe (we run after it) and lives in normal DXE
  // memory, so it is writable - unlike XBL's page (R23av hang / R23aw abort).
  for (Slot = 0; Slot < 32; Slot++) {
    W = (UINT32 *)(UINTN)(gXblUtrlBase + (UINT64)Slot * 32);
    if ((W[0] | W[1] | W[2] | W[3] | W[4] | W[5] | W[6] | W[7]) != 0) {
      NonZero++;
    }
  }
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: nonzero slots: %u  DBR 0x%x\n",
          NonZero, D));

  for (Slot = 31; Slot > 0; Slot--) {
    if ((D & (1u << Slot)) == 0) {
      gSlot = Slot;
      break;
    }
  }
  if (Slot == 0) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: no idle slot - abort\n"));
    return;
  }

  DEBUG ((DEBUG_ERROR, "UFS-DevInit: using slot %u\n", gSlot));
  *SafeToSend = TRUE;
}

STATIC
EFI_STATUS
WaitBit32 (
  IN UINT32  RegOffset,
  IN UINT32  Mask,
  IN BOOLEAN Set,
  IN UINTN   Loops)
{
  UINTN  i;
  UINT32 v;

  for (i = 0; i < Loops; i++) {
    v = UfsReg32 (RegOffset);
    if (Set ? ((v & Mask) != 0) : ((v & Mask) == 0)) {
      return EFI_SUCCESS;
    }
  }

  return EFI_TIMEOUT;
}

//
// Issue a UIC command (e.g. DME_HIBER_EXIT to wake a hibernated link).
// Completion = IS bit10; result code = ARG2 low byte (kernel semantics).
//
STATIC
EFI_STATUS
SendUicCommand (
  IN  UINT8   Cmd,
  OUT UINT32 *Result)
{
  EFI_STATUS Status;

  MmioWrite32 (UFSHCI_BASE + UFSHCI_IS, 0xFFFFFFFF);   // W1C all (direct)
  MmioWrite32 (UFSHCI_BASE + UFSHCI_UICC_ARG1, 0);
  MmioWrite32 (UFSHCI_BASE + UFSHCI_UICC_ARG2, 0);
  MmioWrite32 (UFSHCI_BASE + UFSHCI_UICC_ARG3, 0);
  MmioWrite32 (UFSHCI_BASE + UFSHCI_UICC, Cmd);

  Status = WaitBit32 (UFSHCI_IS, IS_UIC_COMMAND_COMPL, TRUE, WAIT_LOOPS);
  if (Result != NULL) {
    *Result = MmioRead32 (UFSHCI_BASE + UFSHCI_UICC_ARG2) & MASK_UIC_RESULT;
  }
  MmioWrite32 (UFSHCI_BASE + UFSHCI_IS, IS_UIC_COMMAND_COMPL); // W1C direct
  if (EFI_ERROR (Status)) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

//
// Fill the UTRD for the next transfer.  DW6/DW7 carry the response length and
// offset in DOUBLE WORDS; prd_table_length is the PRDT entry count (kernel
// ufshcd_sgl_to_prdt) and prd_table_offset is also in double words.
//
STATIC
VOID
FillUtrd (
  IN UINT8   *Utrd,
  IN UINT8    DataDirection,
  IN UINT32   PrdtEntries,
  IN UINT32   PrdtBytes,
  IN BOOLEAN  WithData,
  IN UINT64   DataPhys,
  IN UINT32   DataLen)
{
  SetMem (Utrd, 32, 0);
  Utrd[0]  = 0;                                            // cci
  Utrd[1]  = 0;                                            // ehs_length
  Utrd[2]  = 0;                                            // crypto off
  Utrd[3]  = (UINT8)((UTRD_CT_UFS_STORAGE << 4) | (DataDirection << 1) | 1);
  Utrd[8]  = OCS_INVALID;                                  // ocs prefill
  Utrd[9]  = 0;                                            // cds
  *(UINT16 *)(Utrd + 10) = 0;                              // ldbc (kernel: 0)
  *(UINT64 *)(Utrd + 16) = (UINT64)(UINTN)gUcd;            // DW4-5 UCD base
  *(UINT16 *)(Utrd + 24) = RESP_LEN_DW;                    // DW6 response len
  *(UINT16 *)(Utrd + 26) = RESP_OFF_DW;                    // DW6 response off
  *(UINT16 *)(Utrd + 28) = (UINT16)PrdtEntries;            // DW7 PRDT entries
  *(UINT16 *)(Utrd + 30) = WithData ? (UINT16)PRDT_OFF_DW : 0;

  if (WithData) {
    UINT8 *P = gUcd + PRDT_OFFSET;

    // struct ufshcd_sg_entry { __le64 addr; __le32 reserved; __le32 size; }
    // size is a 0-based byte count; bits 1:0 must be 11b (dword granularity).
    SetMem (P, PrdtBytes, 0);
    *(UINT64 *)(P + 0) = DataPhys;
    *(UINT32 *)(P + 8) = 0;
    *(UINT32 *)(P + 12) = DataLen - 1;
  }
}

//
// Submit the prepared UTRD: arm IS, ring the doorbell, wait for completion,
// then check OCS.  The controller (and, for reads, the device) is the only
// writer of the response area, so we clean our buffers out before and
// invalidate them after.
//
STATIC
EFI_STATUS
SubmitUtp (
  IN  BOOLEAN WithData,
  IN  UINT64  DataPhys,
  IN  UINT32  DataLen,
  OUT UINT8  *Ocs,
  OUT UINT32 *IsSnapshot)
{
  UINT8      *Utrd = (UINT8 *)(UINTN)(gXblUtrlBase + (UINT64)gSlot * 32);
  EFI_STATUS  Status;

  WriteBackInvalidateDataCacheRange (Utrd, 32);
  WriteBackInvalidateDataCacheRange (gUcd, UCD_CLEAR_SIZE);
  if (WithData) {
    InvalidateDataCacheRange ((VOID *)(UINTN)DataPhys, DataLen);
  }

  // W1C: only ever write the bits we own (R23ax: MmioOr32 wiped pending events)
  MmioWrite32 (UFSHCI_BASE + UFSHCI_IS, IS_UTP_TRANSFER_COMPL);
  MmioWrite32 (UFSHCI_BASE + UFSHCI_UTRLDBR, 1u << gSlot);

  Status = WaitBit32 (UFSHCI_IS, IS_UTP_TRANSFER_COMPL, TRUE, WAIT_LOOPS);
  *IsSnapshot = UfsReg32 (UFSHCI_IS);
  MmioWrite32 (UFSHCI_BASE + UFSHCI_IS, IS_UTP_TRANSFER_COMPL);

  InvalidateDataCacheRange (Utrd, 32);
  InvalidateDataCacheRange (gUcd, UCD_CLEAR_SIZE);
  if (WithData) {
    InvalidateDataCacheRange ((VOID *)(UINTN)DataPhys, DataLen);
  }

  *Ocs = Utrd[8];
  if (EFI_ERROR (Status)) {
    return EFI_TIMEOUT;
  }
  if (*Ocs != OCS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

//
// Generic query sender (READ_FLAG / SET_FLAG / READ_DESCRIPTOR / ...).
// Byte-field writes only - u32 shift construction put query_function and the
// transaction code in the wrong bytes in R23ap (see the header comment).
//
STATIC
EFI_STATUS
SendQuery (
  IN  UINT8    Opcode,
  IN  UINT8    Idn,
  IN  UINT8    Index,
  IN  UINT16   Length,
  IN  UINT32   ValueIn,
  IN  BOOLEAN  Verbose,
  OUT UINT32  *ValueRaw OPTIONAL,
  OUT UINT16  *DsLen OPTIONAL,
  OUT UINT8  **RspUpiu OPTIONAL
  )
{
  UINT8       *Utrd = (UINT8 *)(UINTN)(gXblUtrlBase + (UINT64)gSlot * 32);
  UINT8       *Req  = gUcd;
  UINT8       *Rsp  = gUcd + RSP_UPIU_OFFSET;
  UINT32       IsSnap = 0;
  UINT8        Ocs = 0;
  EFI_STATUS   Status;

  SetMem (gUcd, UCD_CLEAR_SIZE, 0);

  FillUtrd (Utrd, UTP_NO_DATA_TRANSFER, 0, 0, FALSE, 0, 0);

  Req[0]  = UPIU_TRANSACTION_QUERY_REQ;
  Req[1]  = 0;
  Req[2]  = 0;
  Req[3]  = (UINT8)gSlot;                       // task tag echoes the slot
  Req[4]  = 0;
  Req[5]  = (Opcode == UPIU_QUERY_OPCODE_SET_FLAG) ? UPIU_QUERY_FUNC_WRITE
                                                   : UPIU_QUERY_FUNC_READ;
  Req[6]  = 0;
  Req[7]  = 0;
  Req[8]  = 0;
  Req[9]  = 0;
  Req[10] = 0;
  Req[11] = 0;

  Req[12] = Opcode;
  Req[13] = Idn;
  Req[14] = Index;
  Req[15] = 0;
  Req[16] = 0;                                  // reserved_osf (BE16)
  Req[17] = 0;
  Req[18] = (UINT8)(Length >> 8);                // length (BE16)
  Req[19] = (UINT8)(Length & 0xFF);
  Req[20] = (UINT8)(ValueIn >> 24);              // value (BE32, WRITE_ATTR)
  Req[21] = (UINT8)(ValueIn >> 16);
  Req[22] = (UINT8)(ValueIn >> 8);
  Req[23] = (UINT8)(ValueIn);

  Status = SubmitUtp (FALSE, 0, 0, &Ocs, &IsSnap);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
            "UFS-DevInit: query 0x%x idn 0x%x failed: %r ocs 0x%x IS 0x%x HCS 0x%x\n",
            Opcode, Idn, Status, Ocs, IsSnap, UfsReg32 (UFSHCI_HCS)));
    return Status;
  }

  if (Verbose) {
    Dump8 ("resp upiu:", Rsp);
    Dump8 ("resp upiu+:", Rsp + 32);
  }

  if (Rsp[6] != 0) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: query 0x%x idn 0x%x response 0x%x\n",
            Opcode, Idn, Rsp[6]));
    return EFI_DEVICE_ERROR;
  }

  if (ValueRaw != NULL) {
    // value field: query struct +8 = UPIU+20 (BE32, read raw)
    *ValueRaw = *(UINT32 *)(Rsp + 12 + 8);
  }
  if (DsLen != NULL) {
    // header data_segment_length: BE16 at UPIU+10
    *DsLen = (UINT16)((Rsp[10] << 8) | Rsp[11]);
  }
  if (RspUpiu != NULL) {
    *RspUpiu = Rsp;
  }

  return EFI_SUCCESS;
}

//
// Ask the device for a LUN's logical block size.  Unit descriptor offset 0x0A
// holds the power-of-two exponent (kernel: 1 << desc[BLK_SIZE], 0x0C = 4096).
//
STATIC
UINT32
GetLunBlockSize (
  IN UINT8   Lun,
  IN BOOLEAN Verbose
  )
{
  EFI_STATUS  Status;
  UINT16      DsLen = 0;
  UINT8      *Rsp = NULL;
  UINT32      Exp;

  Status = SendQuery (UPIU_QUERY_OPCODE_READ_DESC, QUERY_DESC_IDN_UNIT, Lun, 0x20,
                      0, Verbose, NULL, &DsLen, &Rsp);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: unit%u desc failed: %r (blk 4096 assumed)\n",
            Lun, Status));
    return 4096;
  }

  if (Verbose) {
    Dump4 ("unit desc:", Rsp + 32);
    Dump4 ("unit desc+:", Rsp + 48);
  }

  Exp = Rsp[32 + UNIT_DESC_BLK_SIZE];
  DEBUG ((DEBUG_ERROR,
          "UFS-DevInit: unit%u LU_EN %u blk exp 0x%x (%u B) count 0x%lx\n",
          Lun, Rsp[32 + UNIT_DESC_LU_ENABLE], Exp, 1u << Exp,
          BeN (Rsp + 32 + UNIT_DESC_BLK_COUNT, 8)));

  return 1u << Exp;
}

//
// SCSI READ(10) - a real data transfer, the only test that proves the pipe
// carries device data (OCS/IS only prove the controller saw the descriptor).
//
STATIC
EFI_STATUS
ScsiRead10 (
  IN  UINT8    Lun,
  IN  UINT64   Lba,
  IN  UINT32   Blocks,
  IN  UINT64   BufPhys,
  IN  UINT32   BufLen,
  IN  BOOLEAN  Verbose
  )
{
  UINT8      *Utrd = (UINT8 *)(UINTN)(gXblUtrlBase + (UINT64)gSlot * 32);
  UINT8      *Req  = gUcd;
  UINT8      *Rsp  = gUcd + RSP_UPIU_OFFSET;
  UINT8      *Cdb;
  UINT32      IsSnap = 0;
  UINT8       Ocs = 0;
  EFI_STATUS  Status;

  SetMem (gUcd, UCD_CLEAR_SIZE, 0);

  FillUtrd (Utrd, UTP_DEVICE_TO_HOST, 1, 16, TRUE, BufPhys, BufLen);

  Req[0]  = UPIU_TRANSACTION_COMMAND;
  Req[1]  = UPIU_CMD_FLAGS_READ;
  Req[2]  = Lun;
  Req[3]  = (UINT8)gSlot;
  Req[4]  = UPIU_COMMAND_SET_TYPE_SCSI;
  Req[5]  = 0;
  Req[6]  = 0;
  Req[7]  = 0;
  Req[8]  = 0;
  Req[9]  = 0;
  Req[10] = 0;
  Req[11] = 0;

  // utp_upiu_cmd: exp_data_transfer_len (BE32) then cdb[16]
  Req[12] = (UINT8)(BufLen >> 24);
  Req[13] = (UINT8)(BufLen >> 16);
  Req[14] = (UINT8)(BufLen >> 8);
  Req[15] = (UINT8)(BufLen);

  Cdb = Req + 16;
  Cdb[0] = 0x28;                                    // READ(10)
  Cdb[1] = 0;
  Cdb[2] = (UINT8)(Lba >> 24);
  Cdb[3] = (UINT8)(Lba >> 16);
  Cdb[4] = (UINT8)(Lba >> 8);
  Cdb[5] = (UINT8)(Lba);
  Cdb[6] = 0;
  Cdb[7] = (UINT8)(Blocks >> 8);
  Cdb[8] = (UINT8)(Blocks);
  Cdb[9] = 0;

  Status = SubmitUtp (TRUE, BufPhys, BufLen, &Ocs, &IsSnap);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
            "UFS-DevInit: read10 lun %u lba 0x%lx failed: %r ocs 0x%x IS 0x%x\n",
            Lun, Lba, Status, Ocs, IsSnap));
    return Status;
  }

  if (Verbose) {
    DEBUG ((DEBUG_ERROR,
            "UFS-DevInit: read10 lun %u lba 0x%lx blks %u len %u ocs 0 status 0x%x\n",
            Lun, Lba, Blocks, BufLen, Rsp[7]));
  }

  if (Rsp[7] != 0) {                                // SCSI status (2 = CHECK CONDITION)
    DEBUG ((DEBUG_ERROR,
            "UFS-DevInit: read10 lun %u lba 0x%lx CHECK CONDITION status 0x%x\n",
            Lun, Lba, Rsp[7]));
    DumpSense ("   read10", Rsp);
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

//
// Streaming read through the same descriptor slot, in READ_CHUNK pieces.
//
STATIC
EFI_STATUS
ReadChunks (
  IN UINT8  Lun,
  IN UINT64 Lba,
  IN UINT64 Bytes,
  IN UINT64 BufPhys,
  IN CONST CHAR8 *What)
{
  EFI_STATUS  Status;
  UINT64      Off;
  UINT64      Total = 0;

  for (Off = 0; Off < Bytes; Off += READ_CHUNK) {
    UINT64 Rem    = Bytes - Off;
    UINT64 Want   = (Rem < READ_CHUNK) ? Rem : READ_CHUNK;
    UINT32 Blocks = (UINT32)((Want + gBlockSize - 1) / gBlockSize);
    UINT32 Len    = Blocks * gBlockSize;

    // The final chunk rounds up to a whole block.  The staging allocation is
    // page rounded and RamBytes <= STAGE_BYTES_MAX, so that never overruns.
    Status = ScsiRead10 (Lun, Lba + Off / gBlockSize, Blocks, BufPhys + Off, Len, FALSE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "UFS-DevInit: %a chunk @0x%lx failed: %r\n",
              What, Off, Status));
      return Status;
    }
    Total += Len;

    if ((Off % (READ_CHUNK * 32)) == 0) {
      DEBUG ((DEBUG_ERROR, "UFS-DevInit: %a %lu/%lu KB\n",
              What, (UINT64)(Total / 1024), (UINT64)(Bytes / 1024)));
    }
  }

  DEBUG ((DEBUG_ERROR, "UFS-DevInit: %a done %lu B\n", What, Total));
  return EFI_SUCCESS;
}

//
// Find a GPT partition by name (UTF-16LE) on the LUN we just read the header
// from; returns first/last LBA.
//
STATIC
BOOLEAN
GptFindPartition (
  IN  UINT8        Lun,
  IN  CONST CHAR8  *Name,
  OUT UINT64       *FirstLba,
  OUT UINT64       *LastLba,
  IN  OUT UINT64   *HdrLba,
  IN  OUT UINT64   *EntLba
  )
{
  EFI_STATUS  Status;
  UINT8      *Hdr = gScratch;
  UINT32      Count, EntSize;
  UINT32      i, j;
  UINT8      *Ent;
  UINTN       NameLen = 0;

  while (Name[NameLen] != '\0') {
    NameLen++;
  }

  // GPT header lives in LBA 1 (any logical block size).
  Status = ScsiRead10 (Lun, 1, 1, (UINT64)(UINTN)Hdr, gBlockSize, FALSE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: gpt lun %u LBA1 read failed: %r\n", Lun, Status));
    return FALSE;
  }

  if (CompareMem (Hdr, GPT_SIG, 8) != 0) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: gpt lun %u LBA1 sig: %08x %08x (not EFI PART)\n",
            Lun, *(UINT32 *)Hdr, *(UINT32 *)(Hdr + 4)));
    return FALSE;
  }

  DEBUG ((DEBUG_ERROR, "UFS-DevInit: gpt lun %u header ok (blk %u)\n", Lun, gBlockSize));

  Count   = *(UINT32 *)(Hdr + GPT_ENTRY_COUNT_OFF);
  EntSize = *(UINT32 *)(Hdr + GPT_ENTRY_SIZE_OFF);
  *EntLba = *(UINT64 *)(Hdr + GPT_ENTRIES_LBA_OFF);
  *HdrLba = 1;

  if ((EntSize == 0) || (Count == 0) || (EntSize > 512)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: gpt lun %u entries count %u size %u - abort\n",
            Lun, Count, EntSize));
    return FALSE;
  }
  if ((UINT64)Count * EntSize > SCRATCH_PAGES * EFI_PAGE_SIZE) {
    Count = (SCRATCH_PAGES * EFI_PAGE_SIZE) / EntSize;
  }

  Status = ScsiRead10 (Lun, *EntLba, (UINT32)(((UINT64)Count * EntSize + gBlockSize - 1) / gBlockSize),
                       (UINT64)(UINTN)gScratch, (UINT32)(Count * EntSize), FALSE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: gpt lun %u entry array read failed: %r\n", Lun, Status));
    return FALSE;
  }

  for (i = 0; i < Count; i++) {
    Ent = gScratch + (UINTN)i * EntSize;

    for (j = 0; j < NameLen; j++) {
      if ((Ent[GPT_ENTRY_NAME_OFF + 2 * j] != (UINT8)Name[j]) ||
          (Ent[GPT_ENTRY_NAME_OFF + 2 * j + 1] != 0)) {
        break;
      }
    }

    if (j == NameLen) {
      *FirstLba = *(UINT64 *)(Ent + GPT_ENTRY_FIRST_LBA_OFF);
      *LastLba  = *(UINT64 *)(Ent + GPT_ENTRY_LAST_LBA_OFF);
      DEBUG ((DEBUG_ERROR, "UFS-DevInit: gpt %a at lba 0x%lx..0x%lx (entry %u)\n",
              Name, *FirstLba, *LastLba, i));
      return TRUE;
    }
  }

  DEBUG ((DEBUG_ERROR, "UFS-DevInit: gpt lun %u has no '%a'\n", Lun, Name));
  return FALSE;
}

//
// Make sure the kernel will find a device tree: the FDT goes into the EFI
// configuration table under DEVICE_TREE_GUID, and libstub/fdt.c looks for
// exactly that GUID.  ChainLoadDxe installs it before us, but verify (its old
// literal had the wrong Data4 tail) and install from the FV if missing.
//
STATIC
VOID
EnsureFdtTable (
  VOID
  )
{
  UINTN       Index;
  EFI_STATUS  Status;
  VOID       *Fdt = NULL;
  UINTN       FdtSize = 0;

  for (Index = 0; Index < gST->NumberOfTableEntries; Index++) {
    if (CompareGuid (&gST->ConfigurationTable[Index].VendorGuid, &mFdtTableGuid)) {
      DEBUG ((DEBUG_ERROR, "UFS-DevInit: FDT table ok at 0x%lx\n",
              (UINT64)(UINTN)gST->ConfigurationTable[Index].VendorTable));
      return;
    }
  }

  DEBUG ((DEBUG_ERROR, "UFS-DevInit: FDT table ABSENT - installing from FV\n"));
  Status = GetSectionFromAnyFv (&mDtbFileGuid, EFI_SECTION_RAW, 0, &Fdt, &FdtSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: DTB RAW section not in FV: %r\n", Status));
    return;
  }
  Status = gBS->InstallConfigurationTable (&mFdtTableGuid, Fdt);
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: FDT install 0x%lx size %lu: %r\n",
          (UINT64)(UINTN)Fdt, (UINT64)FdtSize, Status));
}

//
// R23bg state movers.  Order and shape are copied from the kernel's own device
// init path (ufshcd_device_init) because that path is the only UFS init that is
// known to work on this board - the stock Android kernel runs it every boot:
//
//   ufshcd_link_startup()      -> DME_LINKSTARTUP (twice when the device is not
//                                 active: "we will have to issue link startup
//                                 2 times to make sure the device state move to
//                                 active")
//   ufshcd_verify_dev_init()   -> NOP OUT, proves the device's UTP layer is up
//   ufshcd_complete_dev_init() -> SET fDeviceInit + poll
//   ufshcd_set_dev_pwr_mode()  -> START STOP UNIT, power condition = Active, to
//                                 the UFS Device WLUN (the wake path used from
//                                 resume; also what an SSU-shaped device needs
//                                 after a wake from the PowerDown family)
//
// All of it goes through UFSHCI registers and the UTRL slot we already ride:
// no host controller restart, no UTRL/UTMRL base rewrite, no run/stop register,
// no TLMM.  R23bd (TLMM write) and R23be (controller restart + list/RSR writes)
// are the two things that killed this machine; neither is touched here.
//

//
// Read back the two values that decide whether data commands can work at all:
// bCurrentPowerMode (must be Active/0x11) and fDeviceInit (0 = init complete).
//
STATIC
VOID
DumpDevState (
  IN CONST CHAR8  *When
  )
{
  EFI_STATUS  Status;
  UINT32      Raw = 0;
  UINT16      DsLen = 0;

  Status = SendQuery (UPIU_QUERY_OPCODE_READ_ATTR, QUERY_ATTR_IDN_POWER_MODE,
                      0, 0, 0, FALSE, &Raw, &DsLen, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: [%a] power_mode %r\n", When, Status));
    return;
  }
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: [%a] power_mode raw %08x = 0x%x (%a)\n",
          When, Raw, Swap32 (Raw) & 0xFF,
          PowerModeName ((UINT8)(Swap32 (Raw) & 0xFF))));

  Status = SendQuery (UPIU_QUERY_OPCODE_READ_FLAG, QUERY_FLAG_IDN_FDEVICEINIT,
                      0, 0, 0, FALSE, &Raw, &DsLen, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: [%a] fDeviceInit %r\n", When, Status));
    return;
  }
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: [%a] fDeviceInit raw 0x%08x flag %u\n",
          When, Raw, FDEVICEINIT_BIT (Raw)));
}

//
// Poll bCurrentPowerMode until the device reports Active (0x11) or ~200 ms pass
// (a power mode transition is not instantaneous).
//
STATIC
VOID
WaitDevActive (
  VOID
  )
{
  UINTN       i;
  UINT32      Raw = 0;
  EFI_STATUS  Status;

  for (i = 0; i < 20; i++) {
    Status = SendQuery (UPIU_QUERY_OPCODE_READ_ATTR, QUERY_ATTR_IDN_POWER_MODE,
                        0, 0, 0, FALSE, &Raw, NULL, NULL);
    if (EFI_ERROR (Status)) {
      return;
    }
    if ((Swap32 (Raw) & 0xFF) == 0x11) {
      DEBUG ((DEBUG_ERROR, "UFS-DevInit: power_mode reached ACTIVE after %u polls\n",
              (UINT32)i));
      return;
    }
    gBS->Stall (10000);
  }

  DEBUG ((DEBUG_ERROR, "UFS-DevInit: power_mode still 0x%x after 200 ms\n",
          Swap32 (Raw) & 0xFF));
}

//
// START STOP UNIT to the UFS Device WLUN (0xD0) - the kernel's
// ufshcd_set_dev_pwr_mode(): CDB[0] 0x1B, CDB[4] = power condition << 4, no
// data.  A device that came out of the boot chain in the Sleep/PowerDown family
// is moved to Active by this one command; until it is Active it refuses SET
// fDeviceInit (GENERAL_FAILURE 0xFF) and every LU read (04/02).
//
STATIC
EFI_STATUS
SendSsu (
  IN UINT8 PwrCond
  )
{
  UINT8      *Utrd = (UINT8 *)(UINTN)(gXblUtrlBase + (UINT64)gSlot * 32);
  UINT8      *Req  = gUcd;
  UINT8      *Rsp  = gUcd + RSP_UPIU_OFFSET;
  UINT32      IsSnap = 0;
  UINT8       Ocs = 0;
  EFI_STATUS  Status;

  SetMem (gUcd, UCD_CLEAR_SIZE, 0);
  FillUtrd (Utrd, UTP_NO_DATA_TRANSFER, 0, 0, FALSE, 0, 0);

  Req[0]  = UPIU_TRANSACTION_COMMAND;
  Req[1]  = 0;                              // no data stage
  Req[2]  = WLUN_UFS_DEVICE;
  Req[3]  = (UINT8)gSlot;                   // task tag echoes the UTRD slot
  Req[4]  = UPIU_COMMAND_SET_TYPE_SCSI;
  Req[16] = SCSI_START_STOP_UNIT;           // CDB[0]
  Req[20] = (UINT8)(PwrCond << 4);          // CDB[4] = power condition

  Status = SubmitUtp (FALSE, 0, 0, &Ocs, &IsSnap);
  DEBUG ((DEBUG_ERROR,
          "UFS-DevInit: SSU pwr 0x%x -> wlun 0xd0: %r ocs 0x%x type 0x%x rsp 0x%x"
          " status 0x%x\n",
          PwrCond, Status, Ocs, Rsp[0], Rsp[6], Rsp[7]));
  if (!EFI_ERROR (Status) && ((Rsp[7] & 0x3F) != 0)) {
    DumpSense ("   ssu", Rsp);
  }

  return Status;
}

//
// NOP OUT (UPIU 0x00, no data): the kernel's ufshcd_verify_dev_init() liveness
// probe - the device answers with a NOP IN (UPIU 0x20) when its UTP layer is up.
//
STATIC
EFI_STATUS
SendNopOut (
  VOID
  )
{
  UINT8      *Utrd = (UINT8 *)(UINTN)(gXblUtrlBase + (UINT64)gSlot * 32);
  UINT8      *Req  = gUcd;
  UINT8      *Rsp  = gUcd + RSP_UPIU_OFFSET;
  UINT32      IsSnap = 0;
  UINT8       Ocs = 0;
  EFI_STATUS  Status;

  SetMem (gUcd, UCD_CLEAR_SIZE, 0);
  FillUtrd (Utrd, UTP_NO_DATA_TRANSFER, 0, 0, FALSE, 0, 0);

  Req[0] = UPIU_TRANSACTION_NOP_OUT;
  Req[1] = 0;
  Req[2] = 0;
  Req[3] = (UINT8)gSlot;

  Status = SubmitUtp (FALSE, 0, 0, &Ocs, &IsSnap);
  DEBUG ((DEBUG_ERROR,
          "UFS-DevInit: NOP OUT %r ocs 0x%x is 0x%x -> type 0x%x (0x20 = NOP IN)\n",
          Status, Ocs, IsSnap, Rsp[0]));
  return Status;
}

//
// DME_LINKSTARTUP, twice - the kernel's device-state mover when the device is
// not active (see the block comment above).  UIC command only.
//
STATIC
VOID
LinkBringUp (
  VOID
  )
{
  UINTN       Try;
  UINT32      UicResult = 0;
  EFI_STATUS  Status;

  for (Try = 0; Try < 2; Try++) {
    gBS->Stall (1000);                     // kernel: >= 1 ms before a DME cmd
    Status = SendUicCommand (UIC_CMD_DME_LINK_STARTUP, &UicResult);
    DEBUG ((DEBUG_ERROR,
            "UFS-DevInit: link startup %u %r result 0x%x HCS 0x%x DBR 0x%x\n",
            (UINT32)(Try + 1), Status, UicResult,
            UfsReg32 (UFSHCI_HCS), UfsReg32 (UFSHCI_UTRLDBR)));
  }
}

//
// boot_b lives on LUN 4 in this layout (sda..sdf map to UFS LUN 0..5); fall back
// to LUN 0, with that LUN's own block size, if the boot LUN has no entry.
//
STATIC
BOOLEAN
FindBootPartition (
  OUT UINT8   *Lun,
  OUT UINT64  *First,
  OUT UINT64  *Last,
  OUT UINT64  *HdrLba,
  OUT UINT64  *EntLba
  )
{
  if (GptFindPartition (4, "boot_b", First, Last, HdrLba, EntLba)) {
    *Lun = 4;
    return TRUE;
  }

  gBlockSize = GetLunBlockSize (0, FALSE);
  if (GptFindPartition (0, "boot_b", First, Last, HdrLba, EntLba)) {
    *Lun = 0;
    return TRUE;
  }

  return FALSE;
}

//
// Steps 9-11 of the delivery chain: Android boot header -> stream the ramdisk
// field (our vmlinuz.efi) -> LoadImage -> StartImage.
//
STATIC
VOID
DeliverKernel (
  IN EFI_HANDLE  ImageHandle,
  IN UINT8       Lun,
  IN UINT64      FirstLba
  )
{
  EFI_STATUS  Status;
  UINT8      *Hdr = gScratch;
  UINT64      RamOff, RamLba, RamBytes, KernelBytes;
  EFI_HANDLE  KernelImage = NULL;

  // ---- 9. Android boot header page: kernel/ramdisk sizes ----
  Status = ScsiRead10 (Lun, FirstLba, 1, (UINT64)(UINTN)Hdr, gBlockSize, FALSE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: boot hdr read failed: %r\n", Status));
    return;
  }
  Dump8 ("boot hdr:", Hdr);
  if (CompareMem (Hdr, BOOT_MAGIC, 8) != 0) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: boot hdr magic not ANDROID! - stop\n"));
    return;
  }
  KernelBytes = *(UINT32 *)(Hdr + BOOT_KERNEL_SIZE_OFF);
  RamBytes    = *(UINT32 *)(Hdr + BOOT_RAMDISK_SIZE_OFF);
  DEBUG ((DEBUG_ERROR,
          "UFS-DevInit: boot hdr kernel %lu B ramdisk %lu B (page 4096)\n",
          KernelBytes, RamBytes));
  if ((RamBytes == 0) || (RamBytes > STAGE_BYTES_MAX)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: ramdisk size %lu out of range - stop\n", RamBytes));
    return;
  }

  // ramdisk field: page-aligned after header page + page-aligned kernel field
  RamOff = BOOT_PAGE_SIZE + ((KernelBytes + BOOT_PAGE_SIZE - 1) & ~(UINT64)(BOOT_PAGE_SIZE - 1));
  RamLba = FirstLba + RamOff / gBlockSize;

  // ---- 10. stage + stream the ramdisk field (our vmlinuz.efi) ----
  gStage = AllocatePages (EFI_SIZE_TO_PAGES (STAGE_BYTES_MAX));
  if (gStage == NULL) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: stage alloc %lu B failed (mem above) - stop\n",
            (UINT64)STAGE_BYTES_MAX));
    return;
  }
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: stage 0x%lx (%lu B) -> reading ramdisk @lba 0x%lx\n",
          (UINT64)(UINTN)gStage, (UINT64)STAGE_BYTES_MAX, RamLba));

  Status = ReadChunks (Lun, RamLba, RamBytes, (UINT64)(UINTN)gStage, "read");
  if (EFI_ERROR (Status)) {
    return;
  }

  Dump8 ("ramdisk head:", gStage);
  if (CompareMem (gStage, "MZ\x00\x00zimg", 8) != 0) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: ramdisk is not our MZ+zimg PE - stop\n"));
    return;
  }

  // ---- 11. hand over to the kernel EFI stub ----
  Status = gBS->LoadImage (FALSE, ImageHandle, NULL, gStage, (UINTN)RamBytes,
                           &KernelImage);
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: LoadImage %lu B: %r image 0x%lx\n",
          RamBytes, Status, (UINT64)(UINTN)KernelImage));
  if (EFI_ERROR (Status)) {
    return;
  }

  Status = gBS->StartImage (KernelImage, NULL, NULL);
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: StartImage returned %r (kernel did not take over)\n",
          Status));
}

//
// fDeviceInit: SET_FLAG with retries (the kernel's ufshcd_query_flag_retry does
// 3), then poll READ_FLAG until it clears.  A refused SET is NOT fatal: the
// device reports fDeviceInit = 0 when it considers itself initialized and then
// rejects the redundant write (R23bb on-device verdict) - the flag read is the
// statement that counts.
//
STATIC
EFI_STATUS
SetDeviceInit (
  VOID
  )
{
  EFI_STATUS  Status;
  UINT32      Value = 0;
  UINT16      DsLen = 0;
  UINT8      *Rsp = NULL;
  UINTN       Retry, Poll;

  for (Retry = 0; Retry < 3; Retry++) {
    Status = SendQuery (UPIU_QUERY_OPCODE_SET_FLAG, QUERY_FLAG_IDN_FDEVICEINIT,
                        0, 0, 0, (Retry == 0), NULL, &DsLen, &Rsp);
    if (!EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "UFS-DevInit: SET_FLAG fDeviceInit ok (try %u)\n",
              (UINT32)(Retry + 1)));
      break;
    }
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: SET_FLAG try %u refused (%r)\n",
            (UINT32)(Retry + 1), Status));
    gBS->Stall (1000);
  }
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: SET_FLAG refused 3x - flag is the judge\n"));
  }

  for (Poll = 0; Poll < FLAG_CLEAR_POLLS; Poll++) {
    gBS->Stall (1000);
    Status = SendQuery (UPIU_QUERY_OPCODE_READ_FLAG, QUERY_FLAG_IDN_FDEVICEINIT,
                        0, 0, 0, FALSE, &Value, &DsLen, &Rsp);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "UFS-DevInit: READ_FLAG failed at poll %u: %r\n",
              (UINT32)Poll, Status));
      return Status;
    }
    if ((Poll % 64) == 0) {
      DEBUG ((DEBUG_ERROR, "UFS-DevInit: poll %u fDeviceInit=%u raw 0x%08x\n",
              (UINT32)Poll, FDEVICEINIT_BIT (Value), Value));
    }
    if (FDEVICEINIT_BIT (Value) == 0) {
      DEBUG ((DEBUG_ERROR, "UFS-DevInit: fDeviceInit 0 after %u polls\n",
              (UINT32)Poll));
      return EFI_SUCCESS;
    }
  }

  DEBUG ((DEBUG_ERROR, "UFS-DevInit: fDeviceInit never cleared\n"));
  return EFI_TIMEOUT;
}

// R23bf: HostControllerRestart()/LinkStartupWithRetry() are not part of this
// diagnostic build (restore from commit d1ac5a2 when re-adding them).

EFI_STATUS
EFIAPI
UfsDeviceInitDxeEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  UINT32      Cap, Ver, Value = 0;
  UINT16      DsLen = 0;
  UINT8      *Rsp = NULL;
  BOOLEAN     Safe = FALSE;
  UINT8       BootLun = 4;
  UINT64      BootFirst = 0, BootLast = 0, EntLba = 0, HdrLba = 0;
  UINT32      LuNum, ManfId, SpecVer;
  UINT8       IccLvl = 0;

  // ---- 0. MMIO must be mapped before the first register read: the MemoryMapLib
  // entry that used to provide it is gone (it breaks UFSDxe's own mapping). ----
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&mCpu);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: no CPU arch protocol, abort\n"));
    return EFI_SUCCESS;
  }
  Status = MapRangeUncached (UFSHCI_BASE, 0x20000);
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: map UFSHCI @0x%lx UC: %r\n",
          (UINT64)UFSHCI_BASE, Status));
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  Cap = UfsReg32 (UFSHCI_CAP);
  Ver = UfsReg32 (UFSHCI_VER);
  if ((Cap == 0xFFFFFFFF) || (Cap == 0)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: CAP dead (0x%x) - abort\n", Cap));
    return EFI_SUCCESS;
  }
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: CAP 0x%08x VER 0x%x HCS 0x%x\n",
          Cap, Ver, UfsReg32 (UFSHCI_HCS)));

  gSavedIe = UfsReg32 (UFSHCI_IE);
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: IE 0x%x (XBL left it)\n", gSavedIe));

  // ---- 1. ride the running transfer list; never touch RSR/UTRLBA ----
  gXblUtrlBase = (UINT64)UfsReg32 (UFSHCI_UTRLBA_L) |
                 ((UINT64)UfsReg32 (UFSHCI_UTRLBA_H) << 32);
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: UTRLBA 0x%016lx - riding it\n", gXblUtrlBase));
  if (gXblUtrlBase == 0) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: UTRLBA zero - cannot ride, abort\n"));
    return EFI_SUCCESS;
  }

  ProbeControllerState (&Safe);
  if (!Safe) {
    MmioWrite32 (UFSHCI_BASE + UFSHCI_IE, gSavedIe);
    return EFI_SUCCESS;
  }

  // ---- 2. buffers ----
  gUcd     = AllocatePages (1);
  gScratch = AllocatePages (SCRATCH_PAGES);
  if ((gUcd == NULL) || (gScratch == NULL)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: buffer alloc failed\n"));
    return EFI_SUCCESS;
  }

  // Safety gate: our buffers must never overlap UFSDxe's transfer list page
  // (a UCD/DMA target equal to the list base corrupts descriptors silently).
  if ((((UINT64)(UINTN)gUcd + 0x1000) > gXblUtrlBase) &&
      ((UINT64)(UINTN)gUcd < gXblUtrlBase + 32 * 32)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: UCD 0x%lx overlaps UTRL 0x%lx - abort\n",
            (UINT64)(UINTN)gUcd, gXblUtrlBase));
    return EFI_SUCCESS;
  }
  if ((((UINT64)(UINTN)gScratch + SCRATCH_PAGES * EFI_PAGE_SIZE) > gXblUtrlBase) &&
      ((UINT64)(UINTN)gScratch < gXblUtrlBase + 32 * 32)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: scratch 0x%lx overlaps UTRL 0x%lx - abort\n",
            (UINT64)(UINTN)gScratch, gXblUtrlBase));
    return EFI_SUCCESS;
  }
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: UCD 0x%lx scratch 0x%lx\n",
          (UINT64)(UINTN)gUcd, (UINT64)(UINTN)gScratch));

  // ---- 3. memory budget (endgame needs ~28.5 MB) ----
  DumpMemStats ();

  // ---- 4. flag probe: proves request byte layout + response readback ----
  Status = SendQuery (UPIU_QUERY_OPCODE_READ_FLAG, QUERY_FLAG_IDN_FDEVICEINIT,
                      0, 0, 0, TRUE, &Value, &DsLen, &Rsp);
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: probe READ_FLAG %r fDeviceInit raw 0x%08x flag %u\n",
          Status, Value, FDEVICEINIT_BIT (Value)));

  if (Status == EFI_TIMEOUT) {
    // XBL left the link in Hibern8 (IS bit6).  Ask it to exit and retry once.
    UINT32 UicResult = 0;

    DEBUG ((DEBUG_ERROR, "UFS-DevInit: timeout; IS 0x%x - trying hiber exit\n",
            UfsReg32 (UFSHCI_IS)));
    Status = SendUicCommand (UIC_CMD_DME_HIBER_EXIT, &UicResult);
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: HIBER_EXIT %r result 0x%x IS 0x%x\n",
            Status, UicResult, UfsReg32 (UFSHCI_IS)));
    if (!EFI_ERROR (Status)) {
      Status = SendQuery (UPIU_QUERY_OPCODE_READ_FLAG, QUERY_FLAG_IDN_FDEVICEINIT,
                          0, 0, 0, TRUE, &Value, &DsLen, &Rsp);
      DEBUG ((DEBUG_ERROR, "UFS-DevInit: probe retry %r raw 0x%08x flag %u\n",
              Status, Value, FDEVICEINIT_BIT (Value)));
    }
  }

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: READ_FLAG probe failed: %r - abort\n", Status));
    MmioWrite32 (UFSHCI_BASE + UFSHCI_IE, gSavedIe);
    return EFI_SUCCESS;
  }

  // ---- 5. descriptors: device (LU count, vendor) + unit 4 (boot LUN) ----
  Status = SendQuery (UPIU_QUERY_OPCODE_READ_DESC, QUERY_DESC_IDN_DEVICE, 0, 0x40,
                      0, TRUE, NULL, &DsLen, &Rsp);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: device desc failed: %r\n", Status));
  } else {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: device desc len %u (ds %u)\n", (UINT32)0x40, DsLen));
    Dump8 ("dev desc:", Rsp + 32);
    Dump8 ("dev desc+:", Rsp + 64);
    LuNum   = Rsp[32 + DEV_DESC_NUM_LU];
    SpecVer = Rsp[32 + DEV_DESC_SPEC_VER] << 8 | Rsp[32 + DEV_DESC_SPEC_VER + 1];
    ManfId  = Rsp[32 + DEV_DESC_MANF_ID] << 8 | Rsp[32 + DEV_DESC_MANF_ID + 1];
    IccLvl = Rsp[32 + DEV_DESC_ACTVE_ICC_LVL];
    DEBUG ((DEBUG_ERROR,
            "UFS-DevInit: dev NUM_LU %u SPEC 0x%x MANF 0x%x ICC 0x%x\n",
            LuNum, SpecVer, ManfId, IccLvl));
  }

  // Unit descriptor of LUN 4 - the Android boot LUN in this layout (boot_b is
  // /dev/block/sde43; sda..sdf map to UFS LUN 0..5).
  gBlockSize = GetLunBlockSize (4, TRUE);

  // ---- 5b. read-only device-state diagnostics: the descriptors/attributes the
  //      kernel touches on every probe.  No writes here (R23bd lesson: never
  //      poke registers we have not proven safe from this world). ----
  Status = SendQuery (UPIU_QUERY_OPCODE_READ_DESC, QUERY_DESC_IDN_CONFIGURATION, 0, 0x20,
                      0, TRUE, NULL, &DsLen, &Rsp);
  if (!EFI_ERROR (Status)) {
    Dump8 ("cfg desc:", Rsp + 32);
    Dump8 ("cfg desc+:", Rsp + 64);
  } else {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: config desc: %r\n", Status));
  }

  Status = SendQuery (UPIU_QUERY_OPCODE_READ_DESC, QUERY_DESC_IDN_HEALTH, 0, 0x20,
                      0, TRUE, NULL, &DsLen, &Rsp);
  if (!EFI_ERROR (Status)) {
    Dump8 ("health desc:", Rsp + 32);
    Dump8 ("health+: ", Rsp + 64);
  } else {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: health desc: %r\n", Status));
  }

  Status = SendQuery (UPIU_QUERY_OPCODE_READ_ATTR, QUERY_ATTR_IDN_POWER_MODE, 0, 0,
                      0, FALSE, &Value, NULL, NULL);
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: attr power_mode %r = 0x%x\n", Status, Swap32 (Value)));
  Status = SendQuery (UPIU_QUERY_OPCODE_READ_ATTR, QUERY_ATTR_IDN_ACTIVE_ICC_LVL, 0, 0,
                      0, FALSE, &Value, NULL, NULL);
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: attr active_icc %r = 0x%x\n", Status, Swap32 (Value)));
  Status = SendQuery (UPIU_QUERY_OPCODE_READ_ATTR, QUERY_ATTR_IDN_REF_CLK_FREQ, 0, 0,
                      0, FALSE, &Value, NULL, NULL);
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: attr ref_clk %r = 0x%x\n", Status, Swap32 (Value)));

  // ---- 6. fDeviceInit: retried SET, the flag is the judge (R23bd) ----
  Status = SetDeviceInit ();
  MmioWrite32 (UFSHCI_BASE + UFSHCI_IE, gSavedIe);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS-DevInit: device init unconfirmed (%r) - reads decide\n",
            Status));
  }

  // ---- 7. kernel delivery: the kernel needs a DT under DEVICE_TREE_GUID ----
  EnsureFdtTable ();

  // ---- 8. boot_b: reading it is the whole game.  The R23bc datum (read10 ->
  //      CHECK CONDITION 04/02) is reproduced first, then the kernel's device
  //      side state movers are tried in escalating order - each with its own
  //      before/after prints, so the log alone says which move changed what.
  //      (The 5b block above is the pre-state evidence and comes before any
  //      action: the R23bf photo discipline.) ----
  if (FindBootPartition (&BootLun, &BootFirst, &BootLast, &HdrLba, &EntLba)) {
    DeliverKernel (ImageHandle, BootLun, BootFirst);
    return EFI_SUCCESS;
  }

  // ---- stage A: wake the device (SSU -> Active), NOP OUT, SET fDeviceInit ----
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: reads refused (pre, sense above); stage A: SSU\n"));
  SendSsu (SSU_PWR_COND_ACTIVE);
  WaitDevActive ();
  DumpDevState ("post-ssu");
  SendNopOut ();
  (void)SetDeviceInit ();

  if (FindBootPartition (&BootLun, &BootFirst, &BootLast, &HdrLba, &EntLba)) {
    DeliverKernel (ImageHandle, BootLun, BootFirst);
    return EFI_SUCCESS;
  }

  // ---- stage B: kernel-style link bring-up (DME_LINKSTARTUP x2) ----
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: stage A insufficient; stage B: link bring-up\n"));
  LinkBringUp ();
  DumpDevState ("post-link");
  SendNopOut ();
  (void)SetDeviceInit ();

  if (FindBootPartition (&BootLun, &BootFirst, &BootLast, &HdrLba, &EntLba)) {
    DeliverKernel (ImageHandle, BootLun, BootFirst);
    return EFI_SUCCESS;
  }

  // ---- dead end: stop here, leave the whole state in the frozen frame ----
  DEBUG ((DEBUG_ERROR, "UFS-DevInit: no read path after SSU + link bring-up\n"));
  MmioWrite32 (UFSHCI_BASE + UFSHCI_IE, gSavedIe);
  return EFI_SUCCESS;
}
