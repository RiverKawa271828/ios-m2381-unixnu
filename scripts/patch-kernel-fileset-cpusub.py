#!/usr/bin/env python3
"""Normalize the com.apple.kernel member cpusubtype of a fileset kernel.

DarwinBoot (DarwinBoot.c ParseKernel) requires the com.apple.kernel member's
cpusubtype to equal the collection header's cpusubtype, and the donor
kernelcaches (23F84) satisfy that: both 0xC0000002. A kernel built on Linux
with lld carries 0x80000002 (CPU_SUBTYPE_ARM64E | 0x80000000) in its own
header, and the fileset emitter wraps that header verbatim as the member while
the collection header keeps the donor value — the mismatch makes DarwinBoot
refuse the image with EFI_LOAD_ERROR before anything is mapped.

This rewrites only that one u32 so the member matches the collection. The
original value is pinned so a structurally unexpected image is refused rather
than blindly edited.
"""
from pathlib import Path
import struct
import sys

FILESET_ENTRY = 0x80000035
KERNEL_MEMBER = "com.apple.kernel"


def u32(data, off):
    return struct.unpack_from("<I", data, off)[0]


def main():
    if len(sys.argv) != 2:
        sys.exit(f"usage: {sys.argv[0]} KERNEL.MACHO")
    path = Path(sys.argv[1])
    data = bytearray(path.read_bytes())

    if u32(data, 0) != 0xFEEDFACF or u32(data, 12) != 12:
        print(f"patch-cpusub: {path.name} is not an MH_FILESET collection; nothing to do")
        return
    top_sub = u32(data, 8)

    ncmds = u32(data, 16)
    end = 32 + u32(data, 20)
    off = 32
    fixed = 0
    for _ in range(ncmds):
        cmd, length = struct.unpack_from("<II", data, off)
        if cmd == FILESET_ENTRY:
            header_off = struct.unpack_from("<Q", data, off + 16)[0]
            name_off = struct.unpack_from("<I", data, off + 24)[0]
            name = data[off + name_off:off + length].split(b"\0")[0].decode()
            if name == KERNEL_MEMBER:
                member_sub_off = header_off + 8
                member_sub = u32(data, member_sub_off)
                if member_sub == top_sub:
                    print(f"patch-cpusub: {KERNEL_MEMBER} cpusub already 0x{top_sub:08x}")
                    return
                if member_sub != 0x80000002:
                    sys.exit(f"patch-cpusub: unexpected member cpusub 0x{member_sub:08x}; refusing")
                struct.pack_into("<I", data, member_sub_off, top_sub)
                if u32(data, member_sub_off) != top_sub:
                    sys.exit("patch-cpusub: rewrite did not round-trip")
                fixed += 1
        off += length
    if fixed != 1:
        sys.exit(f"patch-cpusub: expected exactly one {KERNEL_MEMBER} member, found {fixed}")
    path.write_bytes(data)
    print(f"patch-cpusub: {KERNEL_MEMBER} cpusub 0x80000002 -> 0x{top_sub:08x} ({path.name})")


if __name__ == "__main__":
    main()
