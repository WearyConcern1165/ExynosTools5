#!/usr/bin/env bash
set -euo pipefail
ANDROID_NDK="/path/to/android-ndk"  # edit this
BUILD_DIR=build-android
rm -rf "$BUILD_DIR" && mkdir -p "$BUILD_DIR"
cmake -S . -B "$BUILD_DIR" -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-31 -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -- -j$(nproc)
