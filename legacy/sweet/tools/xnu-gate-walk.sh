#!/bin/bash
# gate-walk.sh — STRICT full-series walk: git apply only, zero fallback.
# This is the CI gate: dxb build.sh silently skips any patch that fails here.
set -uo pipefail
WS=$(cd "$(dirname "$0")/../../.." && pwd)
PATCHES=$WS/legacy/sweet/xnu/patches
XNU=$WS/research/legacy-sweet/xnu-8796
cd "$XNU"
git reset -q --hard 258d99a
git clean -q -fdx

export MACHINE_CONFIG=M2381 MACOS_VERSION=13.5
for s in "$PATCHES"/*.sh; do bash "$s" >/dev/null 2>&1; done

n=0; bad=0
for P in "$PATCHES"/*.patch; do
	b=$(basename "$P")
	n=$((n+1))
	if ! git apply "$P" 2>/tmp/apply-err; then
		echo "FAIL $b: $(head -1 /tmp/apply-err)"
		bad=$((bad+1))
	fi
done
echo "=== gate: $n patches, $bad failures"
[ "$bad" -eq 0 ]
