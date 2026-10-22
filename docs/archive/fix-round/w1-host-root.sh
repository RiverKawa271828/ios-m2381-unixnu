#!/bin/bash
# W1: apply the full sweet root-tweak set + launchd console-logging patch to
# iOSRoot-s.img. RUN ON THE FEDORA HOST (the ZCode container cannot elevate:
# NoNewPrivs=1), which sees /home/ri natively:
#
#   sudo bash /home/ri/work/ios-meizu-m2381/fix-round/w1-host-root.sh
#
# Needs: hfsplus-tools (fsck.hfsplus), python3. Idempotent (both tools are).
set -euo pipefail

IMG=/home/ri/mkiosroot-work/iOSRoot-raw.img
REPO=/home/ri/work/ios-meizu-m2381
MNT=/mnt/m2381-root
LABELS="com.apple.backboardd com.apple.ReportCrash com.apple.SpringBoard com.apple.geod"

[ -f "$IMG" ] || { echo "!! $IMG missing (container side: simg2img iOSRoot-s.img iOSRoot-raw.img)"; exit 1; }
xxd -s 1024 -l 2 "$IMG" | /usr/bin/grep -q 'HX\|H+' || { echo "!! $IMG is not a bare HFS+ volume"; exit 1; }

echo "== 1/6 fsck (pre) =="
fsck.hfsplus -fy "$IMG"

echo "== 2/6 mount rw =="
mkdir -p "$MNT"
mountpoint -q "$MNT" && umount "$MNT"
mount -t hfsplus -o loop,rw,force "$IMG" "$MNT" || {
    echo "!! mount failed -- if journal refused, try: mount -t hfsplus -o loop,rw,force,journal_buffer_size=8388608"
    exit 1
}

echo "== 3/6 sweet-root-tweaks (containers/data_ark/Setup/BKNoWatchdogs/...) =="
python3 "$REPO/tools/sweet-root-tweaks.py" "$MNT"

echo "== 4/6 launchd console logging (backboardd/ReportCrash/SpringBoard/geod stderr -> /dev/console) =="
python3 "$REPO/tools/patch-launchd-plist.py" \
    "$MNT/System/Library/xpc/launchd.plist" /tmp/launchd.plist.new $LABELS
cat /tmp/launchd.plist.new > "$MNT/System/Library/xpc/launchd.plist"   # in-place: keeps owner/perms
rm -f /tmp/launchd.plist.new

echo "== 5/6 verify =="
for p in private/var/containers/Bundle private/var/containers/Data/System \
         private/var/containers/Shared/SystemGroup private/var/containers/Temp; do
    [ -d "$MNT/$p" ] || { echo "!! expected tweak artifact missing: $p"; umount "$MNT"; exit 1; }
    echo "   ok: $p"
done
# data_ark lives inside the mobileactivationd system container; on an image
# without that container the tweaks script skips it with a warning (sweet
# lineage behavior) -- report, don't fail:
find "$MNT/private/var/containers" -name data_ark.plist 2>/dev/null | head -1 || true
grep -a -c BKNoWatchdogs "$MNT/private/var/mobile/Library/Preferences/com.apple.backboardd.plist" 2>/dev/null \
    || echo "   note: backboardd prefs plist not found at expected path"

echo "== 6/6 unmount + final fsck =="
umount "$MNT"
fsck.hfsplus -fy "$IMG"

echo "W1-DONE: $IMG tweaked. Container side now: 7z verify + img2simg -> flashable simg + sha bookkeeping."
