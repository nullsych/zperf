#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
#
# Build and run the zperf host unit tests (tests/unit), as described in README "Testing":
#   1. 64-bit with ASan + UBSan
#   2. 32-bit, as in CI (skipped with a warning if gcc-multilib is not installed)
#
# Usage: scripts/run_host_ut.sh [ctest args...]
#   e.g. scripts/run_host_ut.sh -R test_clock

set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

run_suite() {
	local name="$1" build_dir="$2"
	shift 2

	echo "=== ${name} ==="
	cmake -S tests/unit -B "$build_dir" "$@"
	cmake --build "$build_dir" -j
	ctest --test-dir "$build_dir" --output-on-failure "${ctest_args[@]}"
}

ctest_args=("$@")

run_suite "64-bit, ASan + UBSan" build/unit -DZPERF_TEST_SANITIZE=ON

# -m32 needs the 32-bit C library (gcc-multilib); check before configuring
if echo 'int main(void) { return 0; }' | ${CC:-cc} -m32 -x c - -o /dev/null 2>/dev/null; then
	run_suite "32-bit" build/unit32 -DZPERF_TEST_M32=ON
else
	echo "=== 32-bit: skipped, install gcc-multilib to run it (sudo apt install gcc-multilib) ==="
fi

echo "=== All host unit tests passed ==="
