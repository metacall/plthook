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
ABIS=(
  x86_64
  x86
  arm64-v8a
  armeabi-v7a
)

# Dependencies
sdkmanager "platform-tools" "emulator"

for ABI in "${ABIS[@]}"; do
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

  # Start emulator (ABI-specific tuning)
  EMULATOR_ARGS=(
    -avd "$AVD_NAME"
    -no-window
    -no-audio
    -no-snapshot
    -gpu off
  )
  emulator "${EMULATOR_ARGS[@]}" &
  EMULATOR_PID=$!

  cleanup() {
    kill "$EMULATOR_PID" 2>/dev/null || true
  }
  trap cleanup EXIT

  adb wait-for-device

  # Root only works on emulator images
  adb root || true
  sleep 2

  # Push and run native tests
  adb push "libs/$ABI/libtest.so" /data/local/tmp/
  adb push "libs/$ABI/testprog"  /data/local/tmp/

  adb shell chmod 755 /data/local/tmp/libtest.so /data/local/tmp/testprog
  adb shell "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testprog open"
  adb shell "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testprog open_by_address"
  adb shell "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/testprog open_by_handle"

  # Shutdown emulator
  kill "$EMULATOR_PID"
  wait "$EMULATOR_PID" || true
  trap - EXIT
done
