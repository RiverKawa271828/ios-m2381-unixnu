#!/usr/bin/env python3
"""Build the trust bundle for the boot image: one owner module for the Adreno Metal plugin, then the two original Apple
trust modules byte for byte. Output: <plugins>/owner-trust.bundle (ignored). The original modules are verified unchanged
against build/original-static-system-trust; only the plugin's CDHash is added.

--originals takes a flat two-module bundle instead of that 21H461-era target input (the 24A446 lane passes the
original-trust-24A446.bundle its payload already carries, sha256 dc655bcb); the modules must still be complete version-2
modules and are copied byte for byte. --plugins selects the plugin build whose trust.json names the CDHashes (the
iPadOS 27 build lives in build/adreno-metal-plugin27)."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import uuid

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--plugins', type=Path, default=ROOT / 'build/adreno-metal-plugin',
                    help='plugin build directory holding trust.json (default: build/adreno-metal-plugin)')
parser.add_argument('--originals', type=Path, default=ROOT / 'build/original-static-system-trust/original-static-system.bundle',
                    help='flat two-module original trust bundle (default: the 21H461 build input, sha-pinned)')
parser.add_argument('--output', type=Path, default=None,
                    help='output bundle (default: <plugins>/owner-trust.bundle)')
args = parser.parse_args()
original = args.originals.read_bytes()
if args.originals == ROOT / 'build/original-static-system-trust/original-static-system.bundle' and \
        hashlib.sha256(original).hexdigest() != '7416ca4f8377146626bb284e2133c76bee6cb4e3ee0af5d648b186235a7323a7':
    raise SystemExit('The original static/System trust bundle differs from the unchanged target input')
count = struct.unpack_from('<I', original)[0]
if count != 2:
    raise SystemExit('The original trust bundle must hold exactly the static and System modules')
offsets = struct.unpack_from('<2I', original, 4)
originals = [original[offsets[0]:offsets[1]], original[offsets[1]:]]
for module in originals:
    entries = struct.unpack_from('<I', module, 20)[0] if len(module) >= 24 else 0
    if len(module) < 24 or struct.unpack_from('<I', module)[0] != 2 or len(module) != 24 + entries * 24:
        raise SystemExit('An original module is not a complete version-2 trust module')
trust = json.loads((args.plugins / 'trust.json').read_text())
# The App cryptex (Cryptex1,AppOS) runs from owner Data, so its own trust module's hashes must be trusted too: the bundle
# carries exactly one owner module followed by both original Apple modules, and the cryptex entries join the owner module.
cryptex_trust = ROOT / 'build/app-cryptex/app-trustcache.bin'
cryptex_hashes = []
if cryptex_trust.is_file():
    module = cryptex_trust.read_bytes()
    entries = struct.unpack_from('<I', module, 20)[0]
    if struct.unpack_from('<I', module)[0] != 2 or len(module) != 24 + entries * 24:
        raise SystemExit('The App cryptex trust module is not a complete version-2 module')
    cryptex_hashes = [module[24 + 24 * entry:24 + 24 * entry + 20] for entry in range(entries)]
# DarwinBoot's ValidTrustCaches (and the kernel's binary search) need the hashes of a module in strictly ascending order.
hashes = sorted({bytes.fromhex(trust['cdhash']), bytes.fromhex(trust['daemon_cdhash']),
                 bytes.fromhex(trust['turnip_cdhash']), *cryptex_hashes})
if len(hashes) != 3 + len(set(cryptex_hashes)):
    raise SystemExit('Duplicate CDHash')
cdhash = hashes[0]
identifier = uuid.uuid5(uuid.NAMESPACE_URL, 'org.blu-sharky.adreno-metal:' + ''.join(h.hex() for h in hashes))
own = struct.pack('<I16sI', 2, identifier.bytes, len(hashes)) + b''.join(h + bytes([2, 0, 0, 0]) for h in hashes)
modules = [own, *originals]
cursor, starts = 16, []
for module in modules:
    starts.append(cursor)
    cursor += len(module)
bundle = struct.pack('<4I', 3, *starts) + b''.join(modules)
def valid_like_the_loader(data):
    # The same structural checks as drivers/DarwinBoot/DarwinBootPkg/Driver/DarwinBoot.c ValidTrustCaches.
    count = struct.unpack_from('<I', data)[0]
    if not 0 < count <= 128:
        return False
    for index in range(count):
        start = struct.unpack_from('<I', data, 4 + 4 * index)[0]
        end = struct.unpack_from('<I', data, 8 + 4 * index)[0] if index + 1 < count else len(data)
        if start < 4 + 4 * count or start & 3 or end > len(data) or end - start < 24 or struct.unpack_from('<I', data, start)[0] != 2:
            return False
        entries = struct.unpack_from('<I', data, start + 20)[0]
        if end - start != 24 + entries * 24:
            return False
        for entry in range(entries):
            at = start + 24 + entry * 24
            if not 1 <= data[at + 20] <= 4 or (entry and data[at - 24:at - 4] >= data[at:at + 20]):
                return False
    return True


if not valid_like_the_loader(bundle):
    raise SystemExit('The trust bundle would be refused by the boot loader')
out = args.output or (args.plugins / 'owner-trust.bundle')
out.parent.mkdir(parents=True, exist_ok=True)
out.write_bytes(bundle)
(out.with_suffix('.json')).write_text(json.dumps(dict(
    bundle_sha256=hashlib.sha256(bundle).hexdigest(), own_cdhashes=[h.hex() for h in hashes], original_modules_unchanged=True,
    apple_trust_chain_verified=False, purpose='owner trust for the Adreno Metal plugin only'), indent=2) + '\n')
print('PASS: owner trust bundle (own module + both original modules)', hashlib.sha256(bundle).hexdigest())
