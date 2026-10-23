#!/bin/sh
# Overlay devices/m2381/uefi (m2381Pkg + m2381.toml) into the Mu-Silicium
# checkout given as $1 (default: out/m2381/uefi). Run after every edit here,
# before `unixnu build firmware --uefi <checkout>`.
set -e
UEFI="${1:-$(dirname "$0")/../../../out/m2381/uefi}"
ROOT="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$UEFI/Platforms/Meizu" "$UEFI/Resources/Configs" "$UEFI/BootShim"
rm -rf "$UEFI/Platforms/Meizu/m2381Pkg"
cp -r "$ROOT/Platforms/Meizu/m2381Pkg" "$UEFI/Platforms/Meizu/"
cp "$ROOT/Resources/Configs/m2381.toml" "$UEFI/Resources/Configs/"
# m2381 BootShim: the nura-proven variant with the Gunyah watchdog disable
# (the stock ABL leaves a Gunyah VM watchdog running; BOOT-CHAIN-RE §4.1).
# Overrides the uefi-sheng shim; compiles fine without the flag too (cpp #if).
cp "$ROOT/BootShim/BootShim.S" "$ROOT/BootShim/Makefile" "$UEFI/BootShim/"
cp "$ROOT/Resources/ramdisk" "$UEFI/Resources/ramdisk"
echo "synced m2381Pkg -> $UEFI"
