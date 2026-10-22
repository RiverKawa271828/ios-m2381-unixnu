/** @file ChainLoadDxe.c
  Chain-loader: start the kernel that ABL delivered as the boot-image RAMDISK.

  R23 (2026-09-25): staging-window reads beyond the FD are fenced at handoff
  (R22a: external abort at 0x80301210, fence armed by ABL's final SMC), so the
  kernel can no longer ride inside the gzip member.  Instead vmlinuz.efi is
  packed as the boot image's ramdisk: ABL concatenates boot+vendor ramdisks
  below 0xB8000000 and patches linux,initrd-start/end into its own DTB (the
  one at x0 on handoff, stashed by BootShim at 0xA7FFFFE8).  Stock kernels
  consume that same region every boot, so it is proven readable post-handoff.

  This driver scans [initrd-start, initrd-end) for the EFI PE (MZ + PE\0\0 +
  AArch64 machine) - order-independent w.r.t. ABL's ramdisk concatenation -
  and starts it via gBS->LoadImage.  The kernel size is passed through the
  boot-image cmdline ("mz_ksize=0x...", ABL patches it into chosen/bootargs)
  so the payload is self-describing; baked fallback = 0xDE6200.

  Dispatches after DtPlatformDxe (FV order) so the FDT table exists first.

  Copyright (c) 2026, meizu20 mainline project. SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/BlockIo.h>

/* FDT configuration table GUID (EmbeddedPkg gFdtTableGuid) */
STATIC EFI_GUID  mFdtTableGuid = { 0xB1B621D5, 0xF19C, 0x41A5, { 0x83, 0x0B, 0xD9, 0xEA, 0x2C, 0xFC, 0x54, 0x32 } };

/* RAW DTB file in the platform FDF (meizu20Pkg/FdtBlob/sm8550-meizu-20-inf.dtb) */
STATIC EFI_GUID  mDtbFileGuid = { 0x25462CDA, 0x221F, 0x47DF, { 0xAC, 0x1D, 0x25, 0x9C, 0xFA, 0xA4, 0xE3, 0x26 } };

/* BootShim handoff slots (inside the FD Kernel Blob mapping) */
#define HANDOFF_DTB_PTR   0xA7FFFFE8ULL   /* x0 = ABL DTB at handoff */
#define HANDOFF_SIZE_SLOT 0xA7FFFFF0ULL   /* legacy (R22a) kernel size slot */

#define FDT_MAGIC        0xD00DFEEDU
#define FDT_BEGIN_NODE   0x00000001U
#define FDT_END_NODE     0x00000002U
#define FDT_PROP         0x00000003U
#define FDT_NOP          0x00000004U
#define FDT_END          0x00000009U

#define PE_MACHINE_AA64  0xAA64U
#define FALLBACK_KSIZE   0xDE6200ULL      /* current vmlinuz.efi, update with build */

STATIC
UINT32
FdtBe32 (
  IN VOID  *P
  )
{
  UINT8  *B = P;

  return ((UINT32)B[0] << 24) | ((UINT32)B[1] << 16) | ((UINT32)B[2] << 8) | B[3];
}


STATIC
UINT8 *
Align4 (
  IN UINT8  *P
  )
{
  return (UINT8 *)(((UINTN)P + 3) & ~(UINTN)3);
}

STATIC
BOOLEAN
StrEq (
  IN CONST CHAR8  *A,
  IN CONST CHAR8  *B
  )
{
  while (*A && (*A == *B)) {
    A++;
    B++;
  }

  return *A == *B;
}


/* R23o/R23q: list depth-2 node names; dump /chosen's property names ONLY.
   bootinfo/memory props are skipped on purpose - bootinfo carries runtime
   IMEI/SN (personal data, keep it off the screen) and its 22 props starved
   the /chosen budget in R23p.  /chosen is the node the window lookup
   depends on, so it gets a dedicated budget. */
STATIC
VOID
DumpChosenDiagnostics (
  IN VOID *Fdt
  )
{
  UINT8    *P;
  UINT8    *Strings;
  INT32    Depth;
  BOOLEAN  InChosen;
  BOOLEAN  ChosenSeen;
  UINTN    PropLines;
  UINTN    NodeLines;

  if (FdtBe32 (Fdt) != FDT_MAGIC) {
    DEBUG ((DEBUG_ERROR, "ChainLoad: ABL DTB magic bad (not an FDT)\n"));
    return;
  }

  P          = (UINT8 *)Fdt + FdtBe32 ((UINT8 *)Fdt + 8);
  Strings    = (UINT8 *)Fdt + FdtBe32 ((UINT8 *)Fdt + 12);
  Depth      = 0;
  InChosen   = FALSE;
  ChosenSeen = FALSE;
  PropLines  = 0;
  NodeLines  = 0;

  while (TRUE) {
    UINT32  Tok = FdtBe32 (P);

    P += 4;
    if (Tok == FDT_BEGIN_NODE) {
      CHAR8  *Name = (CHAR8 *)P;

      while (*P != '\0') {
        P++;
      }
      P = Align4 (P + 1);
      Depth++;
      if (Depth == 2) {
        InChosen   = StrEq (Name, "chosen");
        ChosenSeen = ChosenSeen || InChosen;
        if (NodeLines < 16) {
          DEBUG ((DEBUG_ERROR, "ChainLoad: node '%a'%a\n", Name, InChosen ? " (chosen)" : ""));
          NodeLines++;
        }
      }
    } else if (Tok == FDT_END_NODE) {
      if (--Depth < 0) {
        break;
      }
      InChosen = FALSE;
    } else if (Tok == FDT_PROP) {
      UINT32  Len     = FdtBe32 (P);
      UINT32  NameOff = FdtBe32 (P + 4);
      CHAR8   *Name   = (CHAR8 *)(Strings + NameOff);

      if (InChosen && (PropLines < 12)) {
        DEBUG ((DEBUG_ERROR, "ChainLoad: chosen prop '%a' len %u\n", Name, Len));
        PropLines++;
      }
      P = Align4 (P + 8 + Len);
    } else if (Tok == FDT_END) {
      break;
    } else if (Tok == FDT_NOP) {
      /* skip */
    } else {
      DEBUG ((DEBUG_ERROR, "ChainLoad: bad FDT token 0x%x\n", Tok));
      return;
    }
  }

  if (!ChosenSeen) {
    DEBUG ((DEBUG_ERROR, "ChainLoad: truly no /chosen node in ABL DTB\n"));
  }
}


/* R23r: shared byte-wise zimg signature check (host unit-tested against the
   real vmlinuz.efi: aligned + unaligned hits verified).  The signature is
   "MZ\0\0zimg" (EFI_ZBOOT marks "zimg" at offset 4) + PE\0\0 + AA64 machine. */
STATIC
BOOLEAN
IsZimgPe (
  IN UINT8  *P,
  IN UINT8  *End
  )
{
  UINT32  PeOff;

  if (P + 8 > End) {
    return FALSE;
  }
  if ((P[0] != 'M') || (P[1] != 'Z') || (P[2] != '\0') || (P[3] != '\0') ||
      (P[4] != 'z') || (P[5] != 'i') || (P[6] != 'm') || (P[7] != 'g'))
  {
    return FALSE;
  }
  PeOff = P[0x3C] | ((UINT32)P[0x3D] << 8) | ((UINT32)P[0x3E] << 16) | ((UINT32)P[0x3F] << 24);
  return (P + PeOff + 6 <= End) &&
         (P[PeOff] == 'P') && (P[PeOff + 1] == 'E') &&
         (P[PeOff + 2] == '\0') && (P[PeOff + 3] == '\0') &&
         (P[PeOff + 4] == (UINT8)(PE_MACHINE_AA64 & 0xFF)) &&
         (P[PeOff + 5] == (UINT8)(PE_MACHINE_AA64 >> 8));
}

/* scan [Start, End) byte-wise for the EFI_ZBOOT PE (ABL need not align it) */
STATIC
UINT8 *
FindEfiPe (
  IN UINT8  *Start,
  IN UINT8  *End
  )
{
  UINT8  *P;

  for (P = Start; P + 8 <= End; P++) {
    if (IsZimgPe (P, End)) {
      return P;
    }
  }

  return NULL;
}


/* R23u: read the kernel straight from flash.  boot_b's ramdisk field is our
   own shelf (raw vmlinuz.efi since R23u; ABL never reads this field - R23r
   triple-verified).  Find the BlockIo whose LBA0 is an Android boot header
   (v4) carrying a ~14.5MB ramdisk (stock boot_a has ramdisk_size == 0), read
   the whole field, and let the caller scan it for the EFI PE. */
STATIC
UINT8 *
ReadKernelFromFlash (
  OUT UINTN  *KernelLen
  )
{
  EFI_STATUS              Status;
  EFI_HANDLE              *Handles;
  UINTN                   Count;
  UINTN                   Index;
  EFI_BLOCK_IO_PROTOCOL   *Bio;
  UINT8                   Header[4096];
  UINT32                  HdrVer;
  UINT32                  KSize;
  UINT32                  RSize;
  UINT32                  RamOff;
  UINT32                  BlockSize;
  VOID                    *Buffer;
  UINT8                   *Dst;
  UINTN                   Left;
  EFI_LBA                 Lba;

  Handles   = NULL;
  Count     = 0;
  *KernelLen = 0;

  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &Count, &Handles);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ChainLoad: BlockIo locate %r\n", Status));
    return NULL;
  }

  DEBUG ((DEBUG_ERROR, "ChainLoad: BlockIo handles %u\n", (UINT32)Count));
  // R23ae: repeat the decisive line — the INFO dispatch flood can scroll it
  // off the 150-row screen otherwise.
  gBS->Stall (250000);
  DEBUG ((DEBUG_ERROR, "ChainLoad: BlockIo handles %u (repeat)\n", (UINT32)Count));
  gBS->Stall (250000);
  DEBUG ((DEBUG_ERROR, "ChainLoad: BlockIo handles %u (repeat)\n", (UINT32)Count));
  gBS->Stall (250000);
  DEBUG ((DEBUG_ERROR, "ChainLoad: BlockIo handles %u (repeat)\n", (UINT32)Count));

  for (Index = 0; Index < Count; Index++) {
    Status = gBS->HandleProtocol (Handles[Index], &gEfiBlockIoProtocolGuid, (VOID **)&Bio);
    if (EFI_ERROR (Status) || (Bio == NULL) || (Bio->Media == NULL) ||
        !Bio->Media->MediaPresent)
    {
      continue;
    }

    Status = Bio->ReadBlocks (Bio, Bio->Media->MediaId, 0, sizeof (Header), Header);
    if (EFI_ERROR (Status)) {
      continue;
    }
    if (CompareMem (Header, "ANDROID!", 8) != 0) {
      continue;
    }

    KSize  = *(UINT32 *)(Header + 8);
    RSize  = *(UINT32 *)(Header + 12);
    HdrVer = *(UINT32 *)(Header + 40);
    DEBUG ((DEBUG_ERROR, "ChainLoad: boot hdr v%u ksize %u rsize %u\n", HdrVer, KSize, RSize));

    // Our boot_b: header v4, kernel = gzip member (~1.1MB), ramdisk = raw
    // vmlinuz.efi (14.57MB).  Stock boot_a carries ramdisk_size == 0 (GKI).
    if ((HdrVer != 4) || (KSize == 0) || (KSize > 0x200000) ||
        (RSize < 0xD00000) || (RSize > 0x1000000))
    {
      continue;
    }

    RamOff    = 4096 + ((KSize + 4095u) & ~4095u);
    BlockSize = Bio->Media->BlockSize;

    Buffer = AllocatePool (ALIGN_VALUE (RSize, BlockSize));
    if (Buffer == NULL) {
      continue;
    }

    Dst  = (UINT8 *)Buffer;
    Lba  = RamOff / BlockSize;
    Left = RSize;
    while (Left > 0) {
      UINTN  Chunk = 0x100000u;               /* 1MB steps */
      UINTN  Blocks;

      if (Chunk > Left) {
        Chunk = Left;
      }
      Chunk = ALIGN_VALUE (Chunk, BlockSize);
      Blocks = Chunk / BlockSize;
      Status = Bio->ReadBlocks (Bio, Bio->Media->MediaId, Lba, Chunk, Dst);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "ChainLoad: ReadBlocks @%lx %r\n", (UINT64)Lba, Status));
        FreePool (Buffer);
        break;
      }
      Lba  += Blocks;
      Dst  += Chunk;
      Left -= Chunk;
    }

    if (Left != 0) {
      continue;
    }

    DEBUG ((DEBUG_ERROR, "ChainLoad: flash kernel read: %u B @ramdisk field\n", RSize));
    *KernelLen = RSize;
    FreePool (Handles);
    return (UINT8 *)Buffer;
  }

  FreePool (Handles);
  return NULL;
}

STATIC
VOID *
FindFdtTable (
  VOID
  )
{
  UINTN  Index;

  for (Index = 0; Index < gST->NumberOfTableEntries; Index++) {
    if (CompareGuid (&gST->ConfigurationTable[Index].VendorGuid, &mFdtTableGuid)) {
      return gST->ConfigurationTable[Index].VendorTable;
    }
  }

  return NULL;
}

STATIC
VOID *
AcquireFdt (
  VOID
  )
{
  VOID        *Fdt;
  UINTN       FdtSize;
  EFI_STATUS  Status;

  Fdt = FindFdtTable ();
  if (Fdt != NULL) {
    return Fdt;
  }

  //
  // DtPlatformDxe did not install the table (its variable flows can fail in
  // this world) - pull the DTB RAW section straight out of the FV ourselves.
  //
  FdtSize = 0;
  Status  = GetSectionFromAnyFv (&mDtbFileGuid, EFI_SECTION_RAW, 0, &Fdt, &FdtSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ChainLoad: DTB RAW section not in FV: %r\n", Status));
    return NULL;
  }

  DEBUG ((DEBUG_ERROR, "ChainLoad: DTB from FV at 0x%lx size %lu\n", (UINT64)Fdt, (UINT64)FdtSize));

  Status = gBS->InstallConfigurationTable (&mFdtTableGuid, Fdt);
  DEBUG ((DEBUG_ERROR, "ChainLoad: self-installed FDT from FV: %r\n", Status));
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return Fdt;
}

/* R23w: protocol-notify architecture.  The entry point only registers a
   notify on gEfiBlockIoProtocolGuid and returns - dispatch order between
   ChainLoad and the UFS stack is now irrelevant (R23s flood / R23u
   dependency / R23v parking-order lessons, all the same bug class).  The
   callback fires the moment UFSDxe installs BlockIo and starts the kernel
   from flash. */
STATIC EFI_EVENT  mBlockIoEvent;
STATIC EFI_HANDLE mImageHandle;
STATIC BOOLEAN    mTried;

STATIC
VOID
EFIAPI
OnBlockIoReady (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  VOID        *Kernel;
  UINTN       KernelSize;
  EFI_HANDLE  KernelHandle;
  EFI_STATUS  Status;
  VOID        *Fdt;
  VOID        *AblDtb;
  UINT8       *Pe;

  if (mTried) {
    return;
  }
  mTried = TRUE;
  gBS->CloseEvent (Event);

  Fdt = AcquireFdt ();
  if (Fdt == NULL) {
    DEBUG ((DEBUG_ERROR, "ChainLoad: no FDT available - abort\n"));
    return;
  }

  AblDtb = (VOID *)(UINTN)*(volatile UINT64 *)HANDOFF_DTB_PTR;
  DumpChosenDiagnostics (AblDtb);

  Kernel = ReadKernelFromFlash (&KernelSize);
  if (Kernel == NULL) {
    DEBUG ((DEBUG_ERROR, "ChainLoad: no kernel from flash - abort\n"));
    return;
  }

  // The field holds raw vmlinuz.efi (MZ at offset 0); FindEfiPe re-anchors
  // in case a container (cpio padding) ever comes back.
  Pe = FindEfiPe ((UINT8 *)Kernel, (UINT8 *)Kernel + KernelSize);
  if (Pe != NULL) {
    KernelSize -= (UINTN)Pe - (UINTN)Kernel;
    Kernel      = Pe;
  }

  DEBUG ((DEBUG_ERROR, "ChainLoad: kernel at 0x%lx size 0x%lx - LoadImage\n",
          (UINT64)(UINTN)Kernel, (UINT64)KernelSize));

  KernelHandle = NULL;
  Status = gBS->LoadImage (FALSE, mImageHandle, NULL, Kernel, KernelSize, &KernelHandle);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ChainLoad: LoadImage %r\n", Status));
    return;
  }

  DEBUG ((DEBUG_ERROR, "ChainLoad: StartImage - handing over to Linux EFI stub\n"));
  Status = gBS->StartImage (KernelHandle, NULL, NULL);
  DEBUG ((DEBUG_ERROR, "ChainLoad: StartImage returned %r (kernel failed before ExitBootServices)\n", Status));
}

EFI_STATUS
EFIAPI
ChainLoadDxeEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_BLOCK_IO_PROTOCOL  *Bio;
  VOID                   *Registration;
  EFI_STATUS              Status;

  mImageHandle = ImageHandle;

  Status = gBS->CreateEvent (EVT_NOTIFY_SIGNAL, TPL_CALLBACK, OnBlockIoReady, NULL, &mBlockIoEvent);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ChainLoad: CreateEvent %r - abort\n", Status));
    return EFI_SUCCESS;
  }

  Status = gBS->RegisterProtocolNotify (&gEfiBlockIoProtocolGuid, mBlockIoEvent, &Registration);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ChainLoad: RegisterProtocolNotify %r - abort\n", Status));
    return EFI_SUCCESS;
  }

  // BlockIo may already be installed (we can be dispatched after UFSDxe):
  // RegisterProtocolNotify only fires on FUTURE installs, so probe once.
  Status = gBS->LocateProtocol (&gEfiBlockIoProtocolGuid, NULL, (VOID **)&Bio);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ChainLoad: BlockIo already present - firing\n"));
    OnBlockIoReady (mBlockIoEvent, NULL);
  } else {
    DEBUG ((DEBUG_ERROR, "ChainLoad: waiting for BlockIo (notify armed)\n"));
  }

  return EFI_SUCCESS;
}
