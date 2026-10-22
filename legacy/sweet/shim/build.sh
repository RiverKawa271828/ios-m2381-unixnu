#!/bin/bash
# Build the shim and wrap it in a boot.img v2 that mirrors the stock PixelOS header.
set -euo pipefail
cd "$(dirname "$0")"
CC=aarch64-linux-gnu-gcc
OBJCOPY=aarch64-linux-gnu-objcopy
CFLAGS="-O2 -ffreestanding -nostdlib -fno-pic -mgeneral-regs-only -mstrict-align -fno-stack-protector -Wall -Wextra"
mkdir -p out
for f in start.S shim.c console.c mmu.c fdt.c loader.c ufs.c; do
	$CC $CFLAGS -c $f -o out/${f%.*}.o
done
# The image runs wherever ABL puts it with no relocation pass: only the
# header's image_size (start.o, linked at 0) may be an absolute address.
abs=$(aarch64-linux-gnu-readelf -r out/shim.o out/console.o out/mmu.o out/fdt.o out/loader.o out/ufs.o | grep -c ABS64 || true)
[ "$abs" = 0 ] || { echo "error: $abs absolute relocations in C objects" >&2; exit 1; }
$CC $CFLAGS -static -T shim.ld -Wl,--build-id=none -Wl,--no-warn-rwx-segments -Wl,-N \
	out/start.o out/shim.o out/console.o out/mmu.o out/fdt.o out/loader.o out/ufs.o -o out/shim.elf
$OBJCOPY -O binary out/shim.elf out/Image
# ABL copies the file; pad to image_size so .bss/stacks exist as zeroed bytes.
size=$(aarch64-linux-gnu-nm out/shim.elf | awk '$3=="_end"{print strtonum("0x"$1)}')
truncate -s "$size" out/Image
# Ramdisk: for SWEET the xnu payload rides as the ramdisk (Pixel ABL passes
# it through). For M2381 the GKI ABL ignores the boot.img ramdisk entirely
# (initrd comes from init_boot -- proven on-device: the shim saw only the
# stock 5.7MB initramfs), so the payload embeds into the kernel image at
# the 16K-aligned _payload_start instead and the ramdisk slot stays an
# empty placeholder cpio.
if [ -n "${1:-}" ] && [ "${M2381:-0}" != 1 ]; then
	cp "$1" out/ramdisk.bin
else
	(cd out && rm -rf rd && mkdir rd && cd rd && find . | cpio -o -H newc --quiet | gzip -9 > ../ramdisk.bin)
fi
MKBOOTIMG="${MKBOOTIMG:-/home/ri/work/meizu20/meizu20_linux/tools/mkbootimg}"
if [ "${M2381:-0}" = 1 ]; then
	# Meizu m2381 ABL: header v4, gzip'd kernel payload, no dtb in the image
	# (vendor_boot keeps it). Recipe proven on-device by the meizu20-m1
	# surgery and the mu-r69 daily boots.
	if [ -n "${1:-}" ]; then
		# Embed the payload at _payload_start and fix the arm64 Linux
		# header's image_size (offset 0x10) so the ABL reserves the whole
		# decompressed image, payload included, on gunzip.
		psize=$(aarch64-linux-gnu-nm out/shim.elf | awk '$3=="_payload_start"{print strtonum("0x"$1)}')
		truncate -s "$psize" out/Image
		cat "$1" >> out/Image
		python3 - out/Image <<'PY'
import struct, sys
p = sys.argv[1]
d = bytearray(open(p, 'rb').read())
struct.pack_into('<Q', d, 0x10, (len(d) + 0x3fff) & ~0x3fff)
open(p, 'wb').write(d)
PY
	fi
	gzip -9 -c out/Image > out/Image.gz
	python3 "$MKBOOTIMG/mkbootimg.py" --kernel out/Image.gz --ramdisk out/ramdisk.bin \
		--pagesize 4096 --header_version 4 \
		--os_version 13.0.0 --os_patch_level 2023-10-05 \
		--cmdline "" -o out/shim-boot-m2381.img
	# An artifact is proven by parsing it back (K30 lesson).
	python3 "$MKBOOTIMG/unpack_bootimg.py" --boot_img out/shim-boot-m2381.img \
		--out out/verify >/dev/null
	cmp out/Image.gz out/verify/kernel || { echo "error: unpacked kernel differs" >&2; exit 1; }
	cmp out/ramdisk.bin out/verify/ramdisk || { echo "error: unpacked ramdisk differs" >&2; exit 1; }
	BOOTIMG=out/shim-boot-m2381.img
else
	mkbootimg --header_version 2 --kernel out/Image --ramdisk out/ramdisk.bin \
		--dtb ../dtb/boot_unpacked/dtb --base 0 --kernel_offset 0x8000 --ramdisk_offset 0x1000000 \
		--tags_offset 0x100 --dtb_offset 0x1f00000 --pagesize 4096 \
		--os_version 17.0.0 --os_patch_level 2035-06 \
		--cmdline "console=null" -o out/shim-boot.img
	BOOTIMG=out/shim-boot.img
fi
# Host render of the console layout.
gcc -O1 -Wall -o out/host_test console.c host_test.c && ./out/host_test
ls -l out/Image "$BOOTIMG"
