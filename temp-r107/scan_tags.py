#!/usr/bin/env python3
"""Scan donor kernelcache for movz/movk pairs building the CE dict tags.

Usage: scan_tags.py [kc_path] [base_hex]
Default KC = 24A446 donor, base 0xfffffe0007004000.
"""
import sys, re
KC = sys.argv[1] if len(sys.argv) > 1 else '/home/ri/work/unixnu-meizu/ios-m2381-unixnu/out/ipsw/24A446/kernelcache.decompressed'
BASE = int(sys.argv[2], 16) if len(sys.argv) > 2 else 0xfffffe0007004000
data = open(KC, 'rb').read()
n = len(data) // 4

def word(i):
    return int.from_bytes(data[i*4:i*4+4], 'little')

# MOVZ (sf=1, hw=0): 1101 0010 1 hw=00 imm16 Rd  -> 0xD2800000 | imm16<<5 | Rd
# MOVK (sf=1, hw=3): 1111 0010 1 hw=11 imm16 Rd  -> 0xF2E00000 | imm16<<5 | Rd
# MOVK (sf=1, hw=1): 0xF2A00000 | imm16<<5 | Rd  (lsl #16)
def decode_movz(w):
    if (w >> 23) != 0b110100101:  # sf=1 opc=10 100101
        return None
    hw = (w >> 21) & 3
    imm = (w >> 5) & 0xFFFF
    rd = w & 31
    return ('movz', hw, imm, rd)

def decode_movk(w):
    if (w >> 23) != 0b111100101:
        return None
    hw = (w >> 21) & 3
    imm = (w >> 5) & 0xFFFF
    rd = w & 31
    return ('movk', hw, imm, rd)

TARGETS = {
    (0x10, 0xa000): '0xa000000000000010 (ctx-dict)',
    (0x11, 0x2000): '0x2000000000000011 (set-dict)',
    (0x10, 0x2000): '0x2000000000000010 (sequence)',
    (0x10, 0x6000): '0x6000000000000010 (app-seq wrapper)',
    (0x10, 0x2000 | 0x4000): '0x6000000000000010-alt',
}

hits = []
for i in range(n - 6):
    w0 = word(i)
    d0 = decode_movz(w0)
    if not d0 or d0[1] != 0:
        continue
    _, _, imm0, rd0 = d0
    if imm0 not in (0x10, 0x11):
        continue
    for j in range(i + 1, min(i + 5, n)):
        wj = word(j)
        dj = decode_movk(wj)
        if not dj or dj[1] != 3 or dj[3] != rd0:
            continue
        _, _, immj, _ = dj
        if immj in (0x2000, 0xa000, 0x6000):
            key = (imm0, immj)
            if key in TARGETS:
                hits.append((i, rd0, imm0, immj, TARGETS[key], j - i))
        break  # only adjacent movk for same reg within window; break on first movk of this reg

print(f"total hits: {len(hits)}")
for i, rd, imm0, immj, name, gap in hits:
    va = BASE + i * 4
    print(f"  0x{va:x}  movz x{rd},#0x{imm0:x} + movk x{rd},#0x{immj:x},lsl#48 (gap {gap})  {name}")
