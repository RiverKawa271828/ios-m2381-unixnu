#!/bin/bash
# W1c: fabricate the mobileactivationd system container (topaz v14 recipe,
# fixed UUID) so sweet-root-tweaks activation() finds it and seeds data_ark
# -> mobileactivationd takes its Activated path instead of the SIGTRAP loop.
# RUN ON THE FEDORA HOST:
#
#   sudo bash /home/ri/work/ios-meizu-m2381/fix-round/w1c-host-dataark.sh
set -euo pipefail

IMG=/home/ri/mkiosroot-work/iOSRoot-raw.img
REPO=/home/ri/work/ios-meizu-m2381
MNT=/mnt/m2381-root
UUID=D90C94C4-F94E-4FCF-8A28-A93BE3846D8E
C="$MNT/private/var/containers/Data/System/$UUID"

[ -f "$IMG" ] || { echo "!! $IMG missing"; exit 1; }

echo "== 1/3 mount rw =="
mkdir -p "$MNT"
mountpoint -q "$MNT" && umount "$MNT"
mount -t hfsplus -o loop,rw,force "$IMG" "$MNT"

echo "== 2/3 fabricate mobileactivationd system container =="
mkdir -p "$C/Library/internal"
python3 - "$C/.com.apple.mobile_container_manager.metadata.plist" <<'PY'
import plistlib, sys
plistlib.dump({
    'MCMMetadataVersion': 1,
    'MCMMetadataIdentifier': 'com.apple.mobileactivationd',
    'MCMContainerType': 'System',
}, open(sys.argv[1], 'wb'), fmt=plistlib.FMT_BINARY)
print('metadata written')
PY

echo "== 3/3 re-run tweaks (idempotent; activation() now finds the container) =="
python3 "$REPO/tools/sweet-root-tweaks.py" "$MNT" | /usr/bin/grep -a -e 'data ark' -e 'warning' || true
ls -la "$C/Library/internal/"
umount "$MNT"
fsck.hfsplus -fy "$IMG"
echo "W1C-DONE: container side now: img2simg -> flash"
