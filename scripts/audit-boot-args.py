#!/usr/bin/env python3
"""Boot-args regression gate: diff the boot cmdline of two payload ESPs.

Per the s102/s109 lesson class: board.toml arg lines can silently ride along
with an unrelated change (s102: sheng-data-persist lost in a Case-A args
rewrite) or be suspected of killing a boot when they were innocent (s109: the
offline-keystore=1 removal was falsified as the wall-76 killer — but only
after two wasted rounds). Either way the args delta vs the last green payload
is a first-class gate, checked BEFORE flashing:

  python3 scripts/audit-boot-args.py GREEN_PAYLOAD CANDIDATE_PAYLOAD
  python3 scripts/audit-boot-args.py GREEN.toml CANDIDATE.toml   # pre-extracted

Removals and value changes FAIL (exit 2) unless declared:
  --allow-arg-change offline-keystore=1 "liveactivitiesd fix, s108 verdict"
Extraction: 7z e -so payload.img EFI/UniXNU/unixnu.toml (ESP FAT32).
"""
import argparse, re, subprocess, sys
from pathlib import Path

TOML_IN_ESP = 'EFI/UniXNU/unixnu.toml'


def toml_text(path):
    p = Path(path)
    data = p.read_bytes()
    if data[:2] == b'\xebX' or data[:5] == b'\xebX\x90M':
        r = subprocess.run(['7z', 'e', '-so', str(p), TOML_IN_ESP],
                           capture_output=True)
        if r.returncode != 0 or not r.stdout:
            sys.exit(f'cannot extract {TOML_IN_ESP} from {p}: {r.stderr.decode(errors="replace")[:200]}')
        return r.stdout.decode(errors='replace')
    return data.decode(errors='replace')


def args_of(text):
    for line in text.splitlines():
        m = re.match(r'^args\s*=\s*"(.*)"\s*$', line)
        if m:
            return m.group(1).split()
    return []


def parse_arg(tok):
    k, _, v = tok.partition('=')
    return (k, v if _ else None)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('green'); ap.add_argument('candidate')
    ap.add_argument('--allow-arg-change', nargs=2, action='append', default=[],
                    metavar=('ARG', 'NOTE'),
                    help='declared change, e.g. offline-keystore=1 "reason" (repeatable)')
    args = ap.parse_args()

    g, c = args_of(toml_text(args.green)), args_of(toml_text(args.candidate))
    gd = dict(parse_arg(t) for t in g)
    cd = dict(parse_arg(t) for t in c)
    allowed = {parse_arg(a)[0] for a, _ in args.allow_arg_change}

    removed = [t for t in g if parse_arg(t)[0] not in cd]
    added = [t for t in c if parse_arg(t)[0] not in gd]
    changed = [f'{k}={gd[k]}->{cd[k]}' for k in gd
               if k in cd and gd[k] != cd[k] and gd[k] is not None]
    undeclared = [t for t in removed + changed
                  if parse_arg(t)[0] not in allowed]

    print(f'green     {args.green}: {len(g)} args')
    print(f'candidate {args.candidate}: {len(c)} args')
    for tag, items in (('REMOVED', removed), ('ADDED', added), ('CHANGED', changed)):
        for it in items:
            print(f'  {tag:8s} {it}')
    if not (removed or added or changed):
        print('  args identical')
        print('\nVERDICT: PASS — cmdline unchanged')
        return 0
    for a, note in args.allow_arg_change:
        print(f'  declared {a}: {note}')
    if undeclared:
        print(f'\nVERDICT: FAIL — {len(undeclared)} undeclared arg change(s): '
              f'{", ".join(undeclared)}; declare with --allow-arg-change or fix board.toml')
        return 2
    print('\nVERDICT: PASS — arg changes all declared')
    return 0


if __name__ == '__main__':
    sys.exit(main())
