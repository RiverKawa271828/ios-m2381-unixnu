#!/usr/bin/env python3
"""Boot-face equivalence-class audit: structural diff of two sealed kernel faces.

Per the s108 verdict, every flash candidate must pass this BEFORE flashing:
the diff against the last green face is classified per region as
  A  explained      — known per-round churn (bootargs stamp, ADT seed, linkedit)
  B  intended       — a declared native-change round (--expect-native): pairing
                      re-anchor rewrites every binding-carrying region by definition
  C  unexplained    — anything else; exit 2 (do not flash, or declare the round)

Station sentinel gate (s109 verdict, wall-76 recurrence in r119/r120): the emit
kit's TEXT_WORD_PATCHES table can silently regress — on 2026-11-14 08:28 a kit
sync copied the upstream main-tree submodule file (TEXT_WORD_PATCHES = ()) over
the Mac device-repo authority and dropped all nine stations; r119/r120 emitted
with donor-original AMFI bl words and wall-76 ("AMFI: No PMGR?") came back at
31s after r77-r118 had it dormant. The audit therefore ALWAYS re-reads the nine
station words in the candidate face: a donor-original word = the station table
was lost in the emit — that verdict FAILS regardless of the A/B/C classes
(--expect-native cannot cover it). The checked-in canon below must be updated
in the same commit that deliberately retires a station.

Kit-copy drift check: the authority table is parsed from the device-repo kit
(default auto-detect) and compared against the canon, so a kit edit that drops
or drifts a station is caught at audit time, not on the bench.

Usage:
  python3 scripts/audit-boot-face-delta.py GREEN_SEALED CANDIDATE_SEALED \
      [--expect-native] [--note "..."] [--slide 0x1f000000000] [--no-stations]
      [--kit KIT_FILE]...

An arg-only round must NOT pass --expect-native and additionally must show
zero diff: assert the two faces are byte-identical first in that case.
Kext names are informational (nearest fileset kext at or below the region;
kext __DATA_CONST regions all attribute to the last entry — not authoritative).
"""
import argparse, hashlib, re, struct, sys
from pathlib import Path

BLK = 4096

# donor 0xfffffe000xxxxxxx -> runtime 0xfffffff00xxxxxxx (uniform slide,
# prefix swap ffe->fff; s58/s76 lane constants)
STATION_SLIDE_DEFAULT = 0x1f000000000

# Checked-in canon of the 24A446-lane TEXT_WORD stations (authority:
# UniXNU-m2381/kits/XnuPrelink/fileset_emit.py TEXT_WORD_PATCHES; r76 wall-76
# trio + r106/r109/r110/r111 probe blades). VAs are donor space.
STATIONS_CANON = [
    ('wall76-1', 0xfffffe000924e77c, '9400000d', '_unixnu_amfi_devmode_config'),
    ('wall76-2', 0xfffffe000924ef1c, '97fffe25', '_unixnu_amfi_devmode_config'),
    ('wall76-3', 0xfffffe000924ef5c, '97fffe15', '_unixnu_amfi_devmode_config'),
    ('entcopy1', 0xfffffe000925d040, '94001f14', '_unixnu_amfi_ent_copy_probe'),
    ('entcopy2', 0xfffffe00092639cc, '97fffb31', '_unixnu_amfi_ent_copy_probe_l'),
    ('entconv1', 0xfffffe0009264b80, '94003256', '_unixnu_amfi_entconv_probe1'),
    ('entconv2', 0xfffffe0009264b88, '9400325c', '_unixnu_amfi_entconv_probe2'),
    ('entval1',  0xfffffe0009263af8, '97ffff78', '_unixnu_amfi_entval_probe1'),
    ('entval2',  0xfffffe0009263b6c, '97ffff5b', '_unixnu_amfi_entval_probe2'),
]

# Where the emit kit lives by default (workspace root = two levels above
# this script: <ws>/ios-m2381-unixnu/scripts/ -> <ws>).
DEFAULT_AUTHORITY_KIT = Path(__file__).resolve().parent.parent.parent \
    / 'UniXNU-m2381/kits/XnuPrelink/fileset_emit.py'


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


def va_to_off(segs, va):
    for (n, vm, vs, fo, fs) in segs:
        if vm <= va < vm + vs:
            return fo + (va - vm)
    return None


def word_at(data, segs, va):
    off = va_to_off(segs, va)
    if off is None:
        return None
    return struct.unpack_from('<I', data, off)[0]


def station_rows(data, slide):
    """Per-station word state in a sealed face: LOST = donor original word."""
    segs, _ = segments(data)
    rows = []
    for name, dva, old_hex, sym in STATIONS_CANON:
        old = int(old_hex, 16)
        rva = (dva + slide) & ((1 << 64) - 1)
        w = word_at(data, segs, rva)
        if w is None:
            rows.append((name, sym, old_hex, None, 'UNMAPPED'))
        elif w == old:
            rows.append((name, sym, old_hex, w, 'LOST'))
        else:
            rows.append((name, sym, old_hex, w, 'PATCHED'))
    return rows


def parse_kit_table(kit_path):
    """TEXT_WORD_PATCHES entries as {(donor_va, old_word, symbol)}; None on parse miss."""
    text = Path(kit_path).read_text(errors='replace')
    entries = re.findall(
        r"\(0x([0-9a-fA-F]+),\s*0x([0-9a-fA-F]+),\s*_bl_to_native\('([^']+)'\)", text)
    if not entries:
        return None  # empty table or unparseable layout
    table = set()
    for va, old, sym in entries:
        table.add((int(va, 16), int(old, 16), sym))
    return table


def canon_set():
    return {(va, int(old, 16), sym) for _, va, old, sym in STATIONS_CANON}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('green'); ap.add_argument('candidate')
    ap.add_argument('--expect-native', action='store_true',
                    help='class B: this round changes native code — pairing re-anchor rewrites '
                         'every binding-carrying region by definition; without it any diff is class C')
    ap.add_argument('--note', default='')
    ap.add_argument('--slide', type=lambda x: int(x, 0), default=STATION_SLIDE_DEFAULT,
                    help='donor->runtime VA slide for the station check')
    ap.add_argument('--no-stations', action='store_true',
                    help='skip the station gate (cross-lane audits only)')
    ap.add_argument('--kit', action='append', default=[],
                    help='kit file to check against the station canon (repeatable; '
                         'default = the device-repo authority copy)')
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

    # station file-offset pages inside __TEXT_EXEC of the GREEN face (for
    # region cross-tagging; identical layout in the candidate by construction)
    station_offs = {}
    if not args.no_stations:
        for name, dva, old_hex, sym in STATIONS_CANON:
            off = va_to_off(segs, (dva + args.slide) & ((1 << 64) - 1))
            if off is not None:
                station_offs[name] = off

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
        touching = [n for n, o in station_offs.items() if r[0] <= o < r[1]]
        rows.append((cls, seg, kext_below(r[0]), r[0], r[1]-r[0], touching))
    from collections import Counter
    print('\n== diff by class/segment ==')
    agg = Counter()
    for c, s, _, _, sz, _ in rows:
        agg[(c, s)] += sz
    for (c, s), sz in sorted(agg.items()):
        n = sum(1 for x in rows if x[0] == c and x[1] == s)
        print(f'  class {c} {s:16s} {n:4d} regions {sz:>10} B')
    print('\n== regions ==')
    for c, s, kx, fo, sz, touching in rows:
        tag = f' [stations: {",".join(touching)}]' if touching else ''
        print(f'  {c} {s:16s} {kx[:44]:44s} file {fo:#010x} {sz:>9} B{tag}')

    exit_code = 0
    if not args.no_stations:
        print('\n== station sentinels (candidate face) ==')
        lost = []
        canon_by_name = {name: (dva, int(old, 16)) for name, dva, old, _ in STATIONS_CANON}
        for name, sym, old_hex, w, verdict in station_rows(cand, args.slide):
            dva, old = canon_by_name[name]
            if w is None:
                print(f'  {name:9s} {sym:32s} old {old_hex} cand UNMAPPED  {verdict}')
            else:
                tgt = ''
                if (w >> 26) == 0x25:
                    tgt = f' bl-> {((dva + args.slide + ((w & 0x3ffffff) << 2)) & ((1 << 64) - 1)):#x}'
                print(f'  {name:9s} {sym:32s} old {old_hex} cand {w:08x}{tgt}  {verdict}')
            if verdict == 'LOST':
                lost.append(name)
        if lost:
            print(f'\nSTATIONS LOST: {", ".join(lost)} — the emit kit dropped '
                  'TEXT_WORD_PATCHES (wall-76 class recurrence, s109); --expect-native '
                  'does NOT cover this. Restore the table and re-emit.')
            exit_code = 2
        kits = args.kit or ([str(DEFAULT_AUTHORITY_KIT)] if DEFAULT_AUTHORITY_KIT.exists() else [])
        canon = canon_set()
        for kit in kits:
            table = parse_kit_table(kit)
            if table is None:
                print(f'kit {kit}: TABLE EMPTY OR UNPARSEABLE')
                exit_code = max(exit_code, 2)
            elif table != canon:
                only_kit = table - canon
                only_canon = canon - table
                print(f'kit {kit}: DRIFTED from canon'
                      + (f'; kit-only {sorted(only_kit)}' if only_kit else '')
                      + (f'; canon-only {sorted(only_canon)}' if only_canon else ''))
                exit_code = max(exit_code, 2)
            else:
                print(f'kit {kit}: canon match ({len(canon)} stations)')

    if args.note:
        print(f'\nnote: {args.note}')
    c_rows = [x for x in rows if x[0] == 'C']
    if c_rows:
        print(f'\nVERDICT: FAIL — {len(c_rows)} class-C (unexplained) regions; do not flash, '
              'or re-declare the round as a native/re-anchor round')
        return 2
    if exit_code:
        return exit_code
    print('\nVERDICT: PASS — every differing region is explained (A) or declared (B), '
          'stations patched, kit table = canon')
    return 0


if __name__ == '__main__':
    sys.exit(main())
