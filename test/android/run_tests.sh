#!/bin/bash

set -exuo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

if test -z "$ANDROID_HOME"; then
  echo "ERROR: The environment variable ANDROID_HOME isn't set."
  exit 1
fi

if ! test -x "$ANDROID_HOME/tools/bin/sdkmanager"; then
  echo "ERROR: Invalid environment variable ANDROID_HOME: $ANDROID_HOME"
  exit 1
fi

PATH=$ANDROID_HOME/tools/bin:$ANDROID_HOME/ndk-bundle:$ANDROID_HOME/emulator:$ANDROID_HOME/platform-tools:$PATH

# echo yes | sdkmanager tools ndk-bundle emulator
# ndk-build

API=30
TAG=google_apis
ABIS="arm64-v8a armeabi-v7a x86 x86_64"

for ABI in $ABIS; do
  AVD_NAME=android-$API-$TAG-$ABI
  if ! avdmanager list avd -c | grep "^$AVD_NAME$" > /dev/null; then
    echo no | avdmanager create avd -n $AVD_NAME -k "system-images;android-$API;$TAG;$ABI" -f
  fi

  emulator -avd $AVD_NAME -no-window -no-audio &
  EMULATOR_PID=$!
  trap "kill $EMULATOR_PID" EXIT

  adb wait-for-device
  adb root
  sleep 1
  adb push libs/$ABI/libtest.so /data/local
  adb push libs/$ABI/testprog /data/local
  adb shell "chmod 755 /data/local/libtest.so /data/local/testprog"
  adb shell "env LD_LIBRARY_PATH=/data/local /data/local/testprog open"
  adb shell "env LD_LIBRARY_PATH=/data/local /data/local/testprog open_by_address"
  adb shell "env LD_LIBRARY_PATH=/data/local /data/local/testprog open_by_handle"
  kill $EMULATOR_PID
  trap - EXIT
  wait $EMULATOR_PID || true # Ignore the exit code of the emulator process
done
