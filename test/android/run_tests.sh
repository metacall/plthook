#!/usr/bin/env bash

set -exuo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

# Environment validation
if [[ -z "${ANDROID_SDK_ROOT:-}" ]]; then
  echo "ERROR: ANDROID_SDK_ROOT is not set"
  exit 1
fi

CMDLINE_TOOLS="$ANDROID_SDK_ROOT/cmdline-tools/latest/bin"

if [[ ! -x "$CMDLINE_TOOLS/sdkmanager" ]]; then
  echo "ERROR: Android cmdline-tools not found at $CMDLINE_TOOLS"
  exit 1
fi

export PATH="$CMDLINE_TOOLS:$ANDROID_SDK_ROOT/emulator:$ANDROID_SDK_ROOT/platform-tools:$PATH"

# Configuration
API=34
TAG=google_apis

# Use only x86_64 on CI for reliability (ARM emulators on x86_64 runners are
# extremely slow and often fail to boot). All four ABIs are still built by
# ndk-build so compilation is verified; runtime testing uses x86_64.
ABIS=(
  x86_64
)

BOOT_TIMEOUT=300  # seconds to wait for emulator boot

# Initialize EMULATOR_PID before setting the trap so that the cleanup
# function does not hit an unbound variable error under set -u if the
# script fails before the emulator is started.
EMULATOR_PID=""

cleanup() {
  if [ -n "$EMULATOR_PID" ]; then
    kill "$EMULATOR_PID" 2>/dev/null || true
  fi
}
trap cleanup EXIT

# Dependencies
sdkmanager "platform-tools" "emulator"

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

# Wait for emulator to fully boot with a timeout
wait_for_boot() {
  local timeout=$1
  local elapsed=0
  echo "Waiting for emulator to boot (timeout: ${timeout}s)..."
  adb wait-for-device
  while [ "$elapsed" -lt "$timeout" ]; do
    BOOT_COMPLETED=$(adb shell getprop sys.boot_completed 2>/dev/null | tr -d '\r' || true)
    if [ "$BOOT_COMPLETED" = "1" ]; then
      echo "Emulator booted after ${elapsed}s"
      return 0
    fi
    sleep 5
    elapsed=$((elapsed + 5))
  done
  echo "ERROR: Emulator failed to boot within ${timeout}s"
  return 1
}

for ABI in "${ABIS[@]}"; do
  echo "===== Testing ABI: $ABI ====="

  # Image
  sdkmanager "system-images;android-${API};${TAG};${ABI}"

  # Create AVD if missing
  AVD_NAME="android-${API}-${TAG}-${ABI}"
  if ! avdmanager list avd -c | grep -qx "$AVD_NAME"; then
    echo "Creating AVD $AVD_NAME"
    echo no | avdmanager create avd \
      --name "$AVD_NAME" \
      --package "system-images;android-${API};${TAG};${ABI}" \
      --device "pixel"
  fi

  # List the AVDs
  avdmanager list avd -c

  # Start emulator
  EMULATOR_ARGS=(
    -avd "$AVD_NAME"
    -no-window
    -no-audio
    -no-snapshot
    -gpu off
  )
  emulator "${EMULATOR_ARGS[@]}" &
  EMULATOR_PID=$!

  # Wait for full boot with timeout
  wait_for_boot "$BOOT_TIMEOUT"

  # Root only works on emulator images (not google_apis_playstore).
  # adb root restarts adbd, which drops the connection, so we must
  # wait for the device to reconnect before continuing.
  adb root || true
  adb wait-for-device

  # Push and run native tests
  adb push "libs/$ABI/libtest.so" /data/local/tmp/
  adb push "libs/$ABI/testprog"  /data/local/tmp/

  adb shell chmod 755 /data/local/tmp/libtest.so /data/local/tmp/testprog
  run_on_device "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testprog open"
  run_on_device "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testprog open_by_address"
  run_on_device "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testprog open_by_handle"

  echo "===== ABI $ABI: ALL TESTS PASSED ====="

  # Shutdown emulator
  kill "$EMULATOR_PID" 2>/dev/null || true
  wait "$EMULATOR_PID" 2>/dev/null || true
  EMULATOR_PID=""
done

echo "All Android tests passed!"
