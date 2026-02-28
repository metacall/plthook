#!/usr/bin/env bash

set -exuo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

# Run a command on the device via adb shell and reliably propagate the exit
# code. Older adb/Android versions silently return 0 even when the remote
# command fails, so we append "; echo EXITCODE=$?" and parse the output.
# See https://issuetracker.google.com/issues/36908392
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

# Get root access for /data/local/tmp.
# adb root restarts adbd, dropping the connection, so wait for reconnect.
adb root || true
adb wait-for-device

# Push test binaries
adb push "libs/$ABI/libtest.so" /data/local/tmp/
adb push "libs/$ABI/testprog"   /data/local/tmp/
adb shell chmod 755 /data/local/tmp/libtest.so /data/local/tmp/testprog

# Run tests
run_on_device "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testprog open"
run_on_device "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testprog open_by_address"
run_on_device "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testprog open_by_handle"

echo "All Android tests passed!"
