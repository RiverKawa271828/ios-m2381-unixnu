#!/usr/bin/env python3
"""Build the r130 touch archive: Goodix GT9916 over GENI I2C SE4 plus the HID publisher, for arm64e iPadOS 27 (24A446).

Sources (device package, UNIXNU_DEVICE/package — the canonical fork):
  GoodixTouchProbe.cpp  staged probe (touch=1..4), GENI I2C engine bring-up, the frame thread
  GoodixBerlin.c        the Berlin register protocol (pure)
  GeniI2c.c             the GENI I2C state machine, vendored byte-identical from UniXNU-sheng (BSD-2-Clause-Patent)
  HidInputProbe.cpp     the owned IOHIDDevice registration (offline-hid-input=1), vendored from UniXNU-sheng
and the pinned ShengHidInput source (drivers/ShengHidInput/NativeHidInput.cpp) compiled with
-DNATIVE_HID_SOURCE_API=1 against the pinned IOHIDFamily headers, so every IOHIDDevice virtual keeps its real name and
its pointer-authentication discriminator; the original IOHIDDevice of this fileset (IOHIDFamily-2238.120.5, the OS
profile's pin) is bound through hid-device-24A446.json: each borrowed method goes to the original entry at its
collection address plus the collection slide, and every owned vtable entry is checked to carry the original diversity.

The archive is force-loaded by scripts/build-xnu-4k-baseline.sh behind XNU_NATIVE_GOODIX_TOUCH=1; the board's cmdline
gates the probes themselves (touch=, offline-hid-input=, touch-hid=). Nothing here touches the OS image or the seed.
"""
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from kernel_target import ARCH, IOS_VERSION, TRIPLE   # noqa: E402

BUILD = os.environ.get('UNIXNU_IPSW_BUILD') or '24A446'
if ARCH != 'arm64e' or IOS_VERSION.split('.')[0] != '27' or BUILD != '24A446':
    raise SystemExit('The Goodix touch builder supports arm64e iPadOS 27 (24A446).')
ROOT = Path(__file__).resolve().parents[1]
DEVICE = Path(os.environ.get('UNIXNU_DEVICE') or ROOT.parent / 'UniXNU-m2381')
PKG = DEVICE / 'package'
OUT = ROOT / 'build/goodix-touch'
OUT.mkdir(parents=True, exist_ok=True)
ABI_PATH = Path(__file__).resolve().with_name(f'hid-device-{BUILD}.json')


def run(cmd, **kw):
    return subprocess.check_output([str(x) for x in cmd], timeout=300, **kw)


def require(value, why):
    if not value:
        raise SystemExit(why)


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def discriminator(name):
    """clang ptrauth_string_discriminator: SipHash-2-4 with the fixed key, modulo 0xffff, plus one."""
    mask = (1 << 64) - 1
    rotl = lambda x, b: ((x << b) | (x >> (64 - b))) & mask  # noqa: E731
    key = bytes([0xb5, 0xd4, 0xc9, 0xeb, 0x79, 0x10, 0x4a, 0x79, 0x6f, 0xec, 0x8b, 0x1b, 0x42, 0x87, 0x81, 0xd4])
    k0, k1 = struct.unpack('<2Q', key)
    v = [0x736f6d6570736575 ^ k0, 0x646f72616e646f6d ^ k1, 0x6c7967656e657261 ^ k0, 0x7465646279746573 ^ k1]

    def rounds(n):
        for _ in range(n):
            v[0] = (v[0] + v[1]) & mask; v[1] = rotl(v[1], 13) ^ v[0]; v[0] = rotl(v[0], 32)
            v[2] = (v[2] + v[3]) & mask; v[3] = rotl(v[3], 16) ^ v[2]
            v[0] = (v[0] + v[3]) & mask; v[3] = rotl(v[3], 21) ^ v[0]
            v[2] = (v[2] + v[1]) & mask; v[1] = rotl(v[1], 17) ^ v[2]; v[2] = rotl(v[2], 32)
    data = name.encode()
    full = len(data) & ~7
    for i in range(0, full, 8):
        m = struct.unpack_from('<Q', data, i)[0]
        v[3] ^= m; rounds(2); v[0] ^= m
    b = len(data) << 56 & mask
    for i, c in enumerate(data[full:]):
        b |= c << (8 * i)
    v[3] ^= b; rounds(2); v[0] ^= b; v[2] ^= 0xff; rounds(4)
    return (v[0] ^ v[1] ^ v[2] ^ v[3]) % 0xffff + 1


def macho(data):
    """Load commands, segments and sections of an arm64e relocatable object."""
    magic, cpu, _, _, count = struct.unpack_from('<5I', data)
    require(magic == 0xfeedfacf and cpu == 0x100000c, 'arm64 Mach-O object')
    commands, segments, offset = [], [], 32
    for _ in range(count):
        command, size = struct.unpack_from('<2I', data, offset)
        commands.append((command, offset, size))
        if command == 0x19:
            nsects = struct.unpack_from('<I', data, offset + 64)[0]
            sections = []
            for i in range(nsects):
                at = offset + 72 + 80 * i
                va, length, fileoff = struct.unpack_from('<QQI', data, at + 32)
                sections.append({'name': data[at:at + 16].rstrip(b'\0').decode(), 'va': va, 'size': length,
                                 'fileoff': fileoff, 'command_offset': at})
            segments.append({'sections': sections})
        offset += size
    return {'commands': commands, 'segments': segments}


def object_symbols(data):
    commands = macho(data)['commands']
    st = next(p for c, p, _ in commands if c == 2)
    sy, count, strings, _ = struct.unpack_from('<4I', data, st + 8)
    found = []
    for i in range(count):
        ix, kind, _, _, va = struct.unpack_from('<IBBHQ', data, sy + i * 16)
        found.append((data[strings + ix:data.index(b'\0', strings + ix)].decode(), va, kind))
    return found


def object_vtable(data, found, name, count):
    """[(symbol, signed word)] of the first count entries of vtable name (authenticated pointer relocations)."""
    va = next(v for n, v, _ in found if n == name)
    entries = {}
    for segment in macho(data)['segments']:
        for sec in segment['sections']:
            if sec['va'] <= va < sec['va'] + sec['size']:
                ro, rc = struct.unpack_from('<2I', data, sec['command_offset'] + 56)
                for i in range(rc):
                    offset, bits = struct.unpack_from('<2I', data, ro + i * 8)
                    if bits >> 27 & 1 and bits >> 25 & 3 == 3 and bits >> 28 == 11:
                        entries[sec['va'] + offset] = (found[bits & 0xffffff][0],
                                                       struct.unpack_from('<Q', data, sec['fileoff'] + offset)[0])
    return [entries.get(va + 16 + i * 8) for i in range(count)]


# ---- compilers (the kernel's own compile records) ---------------------------------------------------------------
cpp_record = json.loads((ROOT / 'build/xnu/ios-obj/DEVELOPMENT_ARM64_BCM2837/iokit/DEVELOPMENT/IOPlatformExpert.cpo.json')
                        .read_text().strip().rstrip(','))
c_record = json.loads((ROOT / 'build/xnu/ios-obj/DEVELOPMENT_ARM64_BCM2837/osfmk/DEVELOPMENT/startup.o.json')
                      .read_text().strip().rstrip(','))


def compile_command(record, source, output, extra=()):
    cmd, args, i = [], record['arguments'], 0
    while i < len(args):
        value = args[i]
        if value == record['file']:
            cmd.append(str(source))
        elif value == '-o':
            cmd += ['-o', str(output)]
            i += 1
        elif value in ('-MF', '-MT', '-MJ'):
            i += 1
        elif value == '-Xclang' and args[i + 1].startswith('-W'):
            # The records carry a mispaired -Xclang -Wno-error=... item (the same pair the source-storage filter drops).
            i += 1
        elif not value.startswith('-W'):
            cmd.append(value)
        i += 1
    cmd += ['-mgeneral-regs-only', '-Wno-unused-parameter', '-I', str(PKG)]
    cmd += list(extra)
    return cmd


def compile_source(record, source, output, extra=()):
    command = compile_command(record, source, output, extra)
    (OUT / (output.stem + '.command.json')).write_text(json.dumps(command, indent=2) + '\n')
    result = subprocess.run(command, cwd=record['directory'], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    (OUT / (output.stem + '.log')).write_text(result.stdout)
    if result.returncode:
        raise SystemExit(f'{source.name}: compile failed\n{result.stdout[-6000:]}')
    return output


require((PKG / 'GoodixTouchProbe.cpp').is_file(), f'{PKG}: device package missing (set UNIXNU_DEVICE)')
for name in ('GoodixTouchBoard.h', 'GoodixBerlin.h', 'GoodixBerlin.c', 'GeniI2c.h', 'GeniI2c.c', 'HidInputProbe.cpp'):
    require((PKG / name).is_file(), f'{PKG / name}: missing')

objects, sources = [], {}
for source in (PKG / 'GoodixBerlin.c', PKG / 'GeniI2c.c'):
    objects.append(compile_source(c_record, source, OUT / (source.stem + '.o')))
    sources[str(source)] = sha(source)
for source in (PKG / 'GoodixTouchProbe.cpp', PKG / 'HidInputProbe.cpp'):
    objects.append(compile_source(cpp_record, source, OUT / (source.stem + '.o')))
    sources[str(source)] = sha(source)

# ---- ShengHidInput with the public IOHIDFamily source ------------------------------------------------------------
# The owned IOHIDDevice, byte-identical to UniXNU/ShengHidInput 1140614 ("pair the touch screen with the macOS
# display"; its delta over the catalog's 71a40e76 is the XNU_TARGET_OS_OSX container-ID block and the sheng- arg
# prefix retirement, which this lane needs for touch-hid/touch-split to match HidInputProbe). It is vendored in the
# device package because the catalog recipe (build-sheng-hid-input.py) is the legacy arm64 iPadOS 17 path, while this
# lane needs the source-API objects the HID fixture's bindings are checked against.
hid_source = PKG / 'NativeHidInput.cpp'
require(hid_source.is_file(), f'{hid_source}: the vendored ShengHidInput source is missing')
hid_headers = ROOT / 'refer/IOHIDFamily'
require(hid_headers.is_dir(), f'{hid_headers}: the pinned IOHIDFamily source is not in the tree (unixnu fetch)')
include = OUT / 'include'
for source_dir, relative in ((hid_headers / 'IOHIDFamily', 'IOKit/hid'),
                             (hid_headers / 'IOHIDSystem/IOKit/hidsystem', 'IOKit/hidsystem')):
    destination = include / relative
    destination.mkdir(parents=True, exist_ok=True)
    for path in sorted(source_dir.glob('*.h')):
        (destination / path.name).write_bytes(path.read_bytes())
sdk = Path(run(['xcrun', '--sdk', 'iphoneos', '--show-sdk-path'], text=True).strip())
sdk_kernel = sdk / 'System/Library/Frameworks/Kernel.framework/Versions/A/Headers'
# XNU's own headers keep precedence; the SDK adds the generated DriverKit declarations the source checkout lacks.
hid_object = compile_source(cpp_record, hid_source, OUT / 'NativeHidInput.o',
                            ['-I', str(include), '-idirafter', str(sdk_kernel), '-DNATIVE_HID_SOURCE_API=1'])
sources[str(hid_source)] = sha(hid_source)

# ---- bind the original IOHIDDevice ------------------------------------------------------------------------------
abi = json.loads(ABI_PATH.read_text())
require(abi['kernelcache_sha256'] == json.loads((ROOT / 'scripts/legacy-display-24A446.json').read_text())['kernelcache_sha256'],
        'The HID fixture names another kernelcache than the display fixture')
slide = int(abi['collection_slide'], 16)
slots = [(int(t, 16), int(d, 16)) for t, d in abi['slots']]
# The pure-virtual target of the collection, when the reviewed fixtures name one: a binding must never land on it.
pure = set()
display_abi = json.loads((ROOT / 'scripts/legacy-display-24A446.json').read_text())
if display_abi.get('pure_target'):
    pure = {i for i, (t, _) in enumerate(slots) if t == int(display_abi['pure_target'], 16)}
raw = hid_object.read_bytes()
found = object_symbols(raw)
vtables = [n for n, _, k in found if n.startswith('__ZTV') and k != 1 and '9MetaClassE' not in n]
require(vtables, 'NativeHidInput.o has no vtables')
bindings = {n: int(v, 16) + slide for n, v in abi['bindings'].items()}
by_diversity = {}
for i, (_, d) in enumerate(slots):
    by_diversity.setdefault(d, []).append(i)
slot_of = {}
for vt_name in vtables:
    for i, entry in enumerate(object_vtable(raw, found, vt_name, len(slots))):
        if entry and '11IOHIDDevice' in entry[0]:
            require(slot_of.setdefault(entry[0], i) == i, f'{entry[0]}: one slot in every owned vtable')
for name in sorted({n for n, _, k in found if k == 1 and '11IOHIDDevice' in n} - set(bindings)):
    if name in slot_of:
        slot = slot_of[name]
    else:
        matches = by_diversity.get(discriminator(name[1:]), [])
        require(len(matches) == 1, f'{name}: one original IOHIDDevice slot with its discriminator')
        slot = matches[0]
    require(slot not in pure, f'{name}: not an abstract original slot')
    bindings[name] = slots[slot][0] + slide
for vt_name in vtables:
    entries = object_vtable(raw, found, vt_name, len(slots))
    require(all(entries), f'{vt_name}: at least the {len(slots)} IOHIDDevice entries')
    for i, (name, word) in enumerate(entries):
        require(word >> 63 and word >> 48 & 1 and word >> 49 & 3 == 0 and (word >> 32) & 0xffff == slots[i][1],
                f'{vt_name} slot {i} ({name}) signed with the original diversity {slots[i][1]:#x}')
        if '11IOHIDDevice' in name:
            require(bindings.get(name) == slots[i][0] + slide, f'{vt_name} slot {i} ({name}) is the original entry')
DATA = {'__ZN11IOHIDDevice10gMetaClassE', '__ZTV11IOHIDDevice'}
text, addresses = '.text\n.p2align 2\n', ''
for name, va in sorted(bindings.items()):
    if name in DATA:
        addresses += f'.globl {name}\n.set {name}, {va:#x}\n'
    else:
        addresses += f'.globl {name}.original\n.set {name}.original, {va:#x}\n'
        text += f'.globl {name}\n{name}:\n bti c\n b {name}.original\n'
(OUT / 'hid-bindings.S').write_text(text)
(OUT / 'hid-addresses.S').write_text(addresses)
for stem in ('hid-bindings', 'hid-addresses'):
    run(['xcrun', 'clang', '-target', TRIPLE, '-isysroot', str(sdk), '-c', OUT / (stem + '.S'), '-o', OUT / (stem + '.o')])
objects += [OUT / 'hid-bindings.o', OUT / 'hid-addresses.o']

llvm = Path(run(['brew', '--prefix', 'llvm'], text=True).strip()) / 'bin'
late = OUT / 'NativeHidInput.late.o'
run([llvm / 'llvm-objcopy', '--remove-section=__DATA,__mod_init_func',
     '--globalize-symbol=__GLOBAL__sub_I_NativeHidInput.cpp', hid_object, late])
objects.append(late)

archive = OUT / 'libGoodixTouch.a'
archive.unlink(missing_ok=True)
run(['xcrun', 'ar', 'rcs', archive, *map(str, objects)])
print(f'PASS: GoodixTouch: {len(bindings)} original IOHIDDevice bindings, vtables {vtables}')
report = {
    'target': TRIPLE,
    'abi_fixture': str(ABI_PATH),
    'abi_fixture_sha256': sha(ABI_PATH),
    'hid_bindings': len(bindings),
    'hid_vtables': vtables,
    'sources': sources,
    'objects': {str(p): sha(p) for p in objects},
    'archive': str(archive),
    'archive_sha256': sha(archive),
    'runtime_qualified': False,
    'limitations': ['Compilation and the original-entry bindings are proven on the host only; HID startup, the '
                    'touch frames and the gestures are device gates.'],
}
(OUT / 'build-report.json').write_text(json.dumps(report, indent=2) + '\n')
print(f'Archive: {archive} sha256 {report["archive_sha256"]}')
