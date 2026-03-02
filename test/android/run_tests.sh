#!/usr/bin/env bash

set -exuo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

run_on_device() {
  local output
  output=$(adb shell "$1; echo EXITCODE=\$?" 2>&1) || true
  echo "$output"
  local exit_code
  exit_code=$(echo "$output" | grep -o 'EXITCODE=[0-9]*' | tail -1 | cut -d= -f2) || true
  if [ -z "$exit_code" ]; then
    echo "ERROR: Could not determine exit code from device"
    return 1
  fi
  return "$exit_code"
}

ABI=x86_64

adb root || true
adb wait-for-device

adb push "libs/$ABI/libtest.so" /data/local/tmp/
adb push "libs/$ABI/testprog"   /data/local/tmp/
adb shell chmod 755 /data/local/tmp/libtest.so /data/local/tmp/testprog

run_on_device "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testprog open"
run_on_device "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testprog open_by_address"
run_on_device "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testprog open_by_handle"

echo "All Android tests passed!"
