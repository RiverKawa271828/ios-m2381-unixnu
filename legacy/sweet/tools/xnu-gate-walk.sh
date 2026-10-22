#!/bin/bash
# gate-walk.sh — STRICT full-series walk: git apply only, zero fallback.
# This is the CI gate: dxb build.sh silently skips any patch that fails here.
set -uo pipefail
WS=/home/ri/work/ios-meizu-m2381
XNU=$WS/research/xnu-8796
cd "$XNU"
git reset -q --hard 258d99a
git clean -q -fdx

export MACHINE_CONFIG=M2381 MACOS_VERSION=13.5
for s in "$WS"/xnu/patches/*.sh; do bash "$s" >/dev/null 2>&1; done

n=0; bad=0
for P in "$WS"/xnu/patches/*.patch; do
	b=$(basename "$P")
	n=$((n+1))
	if ! git apply "$P" 2>/tmp/apply-err; then
		echo "FAIL $b: $(head -1 /tmp/apply-err)"
		bad=$((bad+1))
	fi
done
echo "=== gate: $n patches, $bad failures"
[ "$bad" -eq 0 ]
