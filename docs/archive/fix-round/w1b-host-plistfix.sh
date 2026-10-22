#!/bin/bash
# W1b: undo the v8-trap on backboardd/SpringBoard (stderr=/dev/console stops
# launchd from starting them at all -- documented in 0048's comment), keep the
# env vars (os_log mirror -> stderr -> /dev/null -> 0048 kernel tee), and
# (re)patch only ReportCrash + geod whose stderr we DO want on the console.
# RUN ON THE FEDORA HOST:
#
#   sudo bash /home/ri/work/ios-meizu-m2381/fix-round/w1b-host-plistfix.sh
set -euo pipefail

IMG=/home/ri/mkiosroot-work/iOSRoot-raw.img
REPO=/home/ri/work/ios-meizu-m2381
MNT=/mnt/m2381-root
LP="$MNT/System/Library/xpc/launchd.plist"

[ -f "$IMG" ] || { echo "!! $IMG missing"; exit 1; }

echo "== 1/4 mount rw =="
mkdir -p "$MNT"
mountpoint -q "$MNT" && umount "$MNT"
mount -t hfsplus -o loop,rw,force "$IMG" "$MNT"

echo "== 2/4 surgical: strip stdout/stderr=/dev/console from backboardd + SpringBoard jobs =="
python3 - "$LP" <<'PY'
import plistlib, sys
p = sys.argv[1]
d = plistlib.load(open(p, 'rb'))
jobs = d['LaunchDaemons']
for path, j in jobs.items():
    label = j.get('Label', '')
    if label in ('com.apple.backboardd', 'com.apple.SpringBoard'):
        removed = []
        for k in ('StandardErrorPath', 'StandardOutPath'):
            if k in j:
                del j[k]
                removed.append(k)
        env = j.get('EnvironmentVariables') or {}
        print(f'{label}: removed={removed} env-kept={sorted(env)}')
plistlib.dump(d, open(p, 'wb'), fmt=plistlib.FMT_BINARY)
PY

echo "== 3/4 patch ReportCrash + geod stderr -> console =="
python3 "$REPO/tools/patch-launchd-plist.py" "$LP" /tmp/launchd.plist.new \
    com.apple.ReportCrash com.apple.geod
cat /tmp/launchd.plist.new > "$LP"
rm -f /tmp/launchd.plist.new

echo "== 4/4 verify + unmount + fsck =="
python3 - "$LP" <<'PY'
import plistlib, sys
d = plistlib.load(open(sys.argv[1], 'rb'))
jobs = d['LaunchDaemons']
for path, j in jobs.items():
    label = j.get('Label', '')
    if label in ('com.apple.backboardd', 'com.apple.SpringBoard'):
        assert 'StandardErrorPath' not in j and 'StandardOutPath' not in j, label
        print(f'{label}: stderr clean (launchd default) OK')
    if label in ('com.apple.ReportCrash', 'com.apple.geod'):
        print(f"{label}: stderr={j.get('StandardErrorPath')} OK")
PY
umount "$MNT"
fsck.hfsplus -fy "$IMG"
echo "W1B-DONE: container side now: img2simg -> flash"
