#!/usr/bin/env python3
"""Boot-face equivalence-class audit: structural diff of two sealed kernel faces.

Per the s108 verdict, every flash candidate must pass this BEFORE flashing:
the diff against the last green face is classified per region as
  A  explained      — known per-round churn (bootargs stamp, ADT seed, linkedit)
  B  intended       — a declared native-change round (--expect-native): pairing
                      re-anchor rewrites every binding-carrying region by definition
  C  unexplained    — anything else; exit 2 (do not flash, or declare the round)

Usage:
  python3 scripts/audit-boot-face-delta.py GREEN_SEALED CANDIDATE_SEALED \
      [--expect-native] [--note "..."]

An arg-only round must NOT pass --expect-native and additionally must show
zero diff: assert the two faces are byte-identical first in that case.
Kext names are informational (nearest fileset kext at or below the region;
kext __DATA_CONST regions all attribute to the last entry — not authoritative).
"""
import argparse, hashlib, struct, sys
from pathlib import Path

BLK = 4096


def segments(data):
    magic, ct, cs, ft, ncmds, sizeofcmds, flags, _ = struct.unpack_from('<8I', data, 0)
    off, segs, ents = 32, [], []
    for _ in range(ncmds):
        cmd, size = struct.unpack_from('<II', data, off)
        if cmd == 0x19:
            name = data[off+8:off+24].rstrip(b'\0').decode()
            vmaddr, vmsize, fileoff, filesize = struct.unpack_from('<4Q', data, off+24)
            segs.append((name, vmaddr, vmsize, fileoff, filesize))
        elif cmd == 0x80000035:  # LC_FILESET_ENTRY: vmaddr@+8, fileoff@+16, entry_id cstring at cmd+0x20
            vmaddr = struct.unpack_from('<Q', data, off+8)[0]
            fileoff = struct.unpack_from('<Q', data, off+16)[0]
            s = data[off+0x20:off+size]
            ents.append((fileoff, vmaddr, s[:s.find(b'\0')].decode('utf-8', 'replace')))
        off += size
    return segs, sorted(ents)


def regions(a, b):
    blocks = [o for o in range(0, min(len(a), len(b)), BLK) if a[o:o+BLK] != b[o:o+BLK]]
    out = []
    for o in blocks:
        if out and o - out[-1][1] <= BLK:
            out[-1][1] = o + BLK
        else:
            out.append([o, o + BLK])
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('green'); ap.add_argument('candidate')
    ap.add_argument('--expect-native', action='store_true',
                    help='class B: this round changes native code — pairing re-anchor rewrites '
                         'every binding-carrying region by definition; without it any diff is class C')
    ap.add_argument('--note', default='')
    args = ap.parse_args()
    green = Path(args.green).read_bytes()
    cand = Path(args.candidate).read_bytes()
    print(f'green     {hashlib.sha256(green).hexdigest()[:16]} {len(green)} B')
    print(f'candidate {hashlib.sha256(cand).hexdigest()[:16]} {len(cand)} B')
    if len(green) != len(cand):
        print(f'SIZE DELTA {len(cand)-len(green):+#} B (both faces must be sealed)')
    segs, ents = segments(green)
    kext_starts = [e[0] for e in ents]
    import bisect
    def kext_below(fo):
        i = bisect.bisect_right(kext_starts, fo) - 1
        return ents[i][2] if i >= 0 else '(base kernel)'
    def seg_of(fo):
        for (n, vm, vs, f2, fs) in segs:
            if f2 <= fo < f2 + fs:
                return n
        return '?'
    rows = []
    for r in regions(green, cand):
        seg = seg_of(r[0])
        # A = per-round stamp churn; B = declared native round (pairing re-anchor
        # rewrites binding-carrying regions by definition); C = unexplained.
        if seg in ('__BOOTDATA', '__LINKEDIT'):
            cls = 'A'
        elif args.expect_native:
            cls = 'B'
        else:
            cls = 'C'
        rows.append((cls, seg, kext_below(r[0]), r[0], r[1]-r[0]))
    from collections import Counter
    print('\n== diff by class/segment ==')
    agg = Counter()
    for c, s, _, _, sz in rows:
        agg[(c, s)] += sz
    for (c, s), sz in sorted(agg.items()):
        n = sum(1 for x in rows if x[0] == c and x[1] == s)
        print(f'  class {c} {s:16s} {n:4d} regions {sz:>10} B')
    print('\n== regions ==')
    for c, s, kx, fo, sz in rows:
        print(f'  {c} {s:16s} {kx[:44]:44s} file {fo:#010x} {sz:>9} B')
    if args.note:
        print(f'\nnote: {args.note}')
    c_rows = [x for x in rows if x[0] == 'C']
    if c_rows:
        print(f'\nVERDICT: FAIL — {len(c_rows)} class-C (unexplained) regions; do not flash, '
              'or re-declare the round as a native/re-anchor round')
        return 2
    print('\nVERDICT: PASS — every differing region is explained (A) or declared (B)')
    return 0


if __name__ == '__main__':
    sys.exit(main())
