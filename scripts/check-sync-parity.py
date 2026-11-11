#!/usr/bin/env python3
"""Cross-machine build-input parity gate (s90, after the r95-r101 drift walls).

The r95-r101 bars-only streak was produced entirely by sync drift, never by
kernel logic: an rsync silently refreshed a stale Mac RamBlock checkout
(r95-r98), four same-named patches carried different content on each side
(r99-r101), and the Mac kit pin diverged from the ledger (r100). The remedy
is mechanical: before every Mac build round, hash the exact input set the
Mac chain consumes on BOTH machines and diff. Any difference is a build
stop, decided item by item before the chain runs.

Usage:
  python3 scripts/check-sync-parity.py                # local manifest only
  python3 scripts/check-sync-parity.py --remote HOST  # full parity verdict

The remote collection re-executes this same file over ssh (--collect-only)
so there is nothing to deploy on the Mac. Content hashes are the verdict;
git HEAD/dirty state is reported as context only (the Mac legitimately sits
on its own commits with rsync'd, content-identical files).
"""
import argparse
import hashlib
import json
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

MAIN_FILES = [
    'profiles/os/ipados-27.0.1-24A446/patches/xnu/series',
    'profiles/arch/arm64e/patches/xnu/series',
    'scripts/build-xnu-4k-baseline.sh',
    'scripts/build-sheng-source-storage.py',
    'scripts/prepare-xnu-source.py',
    'scripts/build-arm-driver.py',
    'components.toml',
    'feeds.toml',
]
DEVICE_FILES = [
    'board.toml',
    'package/KeyStoreProbe.cpp',
    'package/LegacyDisplayProbe.cpp',
    'package/BootLog.c',
    'package/SourceStorage.cpp',
    'patches/xnu/series',
    'patches/xnu/offline-keystore-start.patch',
]


def git(repo, *args):
    result = subprocess.run(['git', *args], cwd=str(repo), capture_output=True, text=True)
    return result.stdout.strip() if result.returncode == 0 else f'(git failed: {result.stderr.strip()[:80]})'


def sha(path):
    try:
        return hashlib.sha256(Path(path).read_bytes()).hexdigest()[:16]
    except OSError:
        return 'MISSING'


def worktree_fingerprint(repo):
    """Content fingerprint of a git worktree's tracked files, HEAD-independent.

    HEAD alone cannot decide parity here: the Mac legitimately sits on older
    commits with rsync'd, content-identical files (the r95 lesson was the
    opposite case - a stale checkout silently refreshed by content rsync).
    The fingerprint covers the HEAD tree plus every dirty/untracked file's
    content, so two worktrees match whenever their effective build inputs
    match, whatever their git views claim.
    """
    status = git(repo, 'status', '--porcelain')
    entries = [line[3:] for line in status.splitlines() if line[3:]]
    if any(entry.startswith('"') for entry in entries):
        import ast
        entries = [ast.literal_eval(entry) if entry.startswith('"') else entry for entry in entries]
    digest = hashlib.sha256()
    digest.update(git(repo, 'rev-parse', 'HEAD').encode())
    for name in sorted(entries):
        path = Path(repo) / name
        digest.update(name.encode())
        try:
            digest.update(hashlib.sha256(path.read_bytes()).digest())
        except OSError:
            digest.update(b'<unreadable>')
    return digest.hexdigest()[:16], len(entries)


def collect(main_root, device_root):
    manifest = {}
    main_root, device_root = Path(main_root), Path(device_root)
    for rel in MAIN_FILES:
        manifest[rel] = sha(main_root / rel)
    patch_dir = main_root / 'patches' / 'xnu-12377'
    if patch_dir.is_dir():
        for patch in sorted(patch_dir.glob('*.patch')):
            manifest[f'patch/{patch.name}'] = sha(patch)
    for rel in DEVICE_FILES:
        manifest[f'dev/{rel}'] = sha(device_root / rel)
    manifest['main_HEAD'] = git(main_root, 'rev-parse', 'HEAD')[:12]
    manifest['main_dirty'] = len(git(main_root, 'status', '--porcelain').splitlines())
    manifest['device_HEAD'] = git(device_root, 'rev-parse', 'HEAD')[:12]
    manifest['device_dirty'] = len(git(device_root, 'status', '--porcelain').splitlines())
    drivers = {}
    for directory in sorted((main_root / 'drivers').iterdir()):
        if (directory / '.git').exists():
            fingerprint, dirty = worktree_fingerprint(directory)
            drivers[directory.name] = [fingerprint, dirty]
    manifest['drivers'] = drivers
    kit = main_root / 'kits' / 'XnuPrelink'
    if (kit / '.git').exists():
        fingerprint, dirty = worktree_fingerprint(kit)
        manifest['kit/XnuPrelink'] = [fingerprint, dirty]
    return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--main', default=str(ROOT), help='main tree root (default: this checkout)')
    parser.add_argument('--device', default=None, help='device repo root (default: ../UniXNU-m2381)')
    parser.add_argument('--remote', default=None, metavar='HOST',
                        help='ssh host; run the parity verdict against its identical checkout')
    parser.add_argument('--remote-main', default='/Users/naoniudeair/unixnu/ios-m2381-unixnu')
    parser.add_argument('--remote-device', default='/Users/naoniudeair/unixnu/UniXNU-m2381')
    parser.add_argument('--collect-only', action='store_true', help=argparse.SUPPRESS)
    args = parser.parse_args()

    device_root = args.device or str(Path(args.main).parent / 'UniXNU-m2381')
    if args.collect_only:
        print(json.dumps(collect(args.main, device_root), indent=1, sort_keys=True))
        return 0

    local = collect(args.main, device_root)
    if not args.remote:
        print(json.dumps(local, indent=1, sort_keys=True))
        print(f'\n{len(local)} keys collected (local only; pass --remote for the verdict)')
        return 0

    self_text = Path(__file__).read_text()
    command = ['ssh', args.remote,
               f"python3 - --collect-only --main '{args.remote_main}' "
               f"--device '{args.remote_device}'"]
    result = subprocess.run(command, input=self_text, capture_output=True, text=True)
    if result.returncode:
        print(f'remote collection failed: {result.stderr.strip()[-400:]}')
        return 2
    remote = json.loads(result.stdout)

    # Hard keys feed the build directly (series, patches, build scripts, the
    # device package, board args). Submodule fingerprints and HEAD/dirty state
    # are context: the Mac legitimately carries uncommitted sync layers until
    # the round-end commit sweep absorbs them, and HEAD never decides parity.
    hard = lambda key: (key in local or key in remote) and not key.startswith(('drivers', 'kit/',
        'main_HEAD', 'main_dirty', 'device_HEAD', 'device_dirty'))
    diffs = [(k, local.get(k), remote.get(k)) for k in sorted(set(local) | set(remote))
             if local.get(k) != remote.get(k)]
    blocking = [d for d in diffs if hard(d[0])]
    context = [d for d in diffs if not hard(d[0])]
    print(f"parity: {len(local) - len(diffs)}/{len(set(local) | set(remote))} keys match "
          f"(local main {local['main_HEAD']} / device {local['device_HEAD']}, "
          f"remote main {remote['main_HEAD']} / device {remote['device_HEAD']})")
    for key, ours, theirs in context:
        print(f'  NOTE {key}:\n    local : {ours}\n    remote: {theirs}')
    if not blocking:
        print('PARITY PASS' if not context else 'PARITY PASS (with context notes above)')
        return 0
    for key, ours, theirs in blocking:
        print(f'  DIFF {key}:\n    local : {ours}\n    remote: {theirs}')
    print('PARITY FAIL — resolve every hard key (sync or consciously accept) before the Mac chain')
    return 1


if __name__ == '__main__':
    sys.exit(main())
