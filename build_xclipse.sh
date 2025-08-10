#!/bin/bash
set -euo pipefail

echo "Building Xclipse Tools for Exynos 2400..."

# Configuration
ANDROID_NDK="${ANDROID_NDK:-/path/to/android-ndk}"
BUILD_DIR="build-xclipse"
INSTALL_DIR="install-xclipse"

# Clean previous builds
rm -rf "$BUILD_DIR" "$INSTALL_DIR"
mkdir -p "$BUILD_DIR" "$INSTALL_DIR"

echo "Building for ARM64..."

# Build for ARM64
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-31 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"

cmake --build "$BUILD_DIR" -- -j$(nproc)
cmake --install "$BUILD_DIR"

echo "Creating Winlator package..."

# Create Winlator package structure
mkdir -p "$INSTALL_DIR/usr/lib"
cp "$BUILD_DIR/libxeno_wrapper.so" "$INSTALL_DIR/usr/lib/"

# Create tar.zst package for Winlator
cd "$INSTALL_DIR"
tar -czf ../xclipse_tools_winlator.tar.zst usr/

echo "Creating Android package..."

# Create Android package structure
mkdir -p "../android-package/libs/arm64-v8a"
cp "$BUILD_DIR/libxeno_wrapper.so" "../android-package/libs/arm64-v8a/"
cp ../xclipse_tools.conf "../android-package/"

# Create meta.json for Android
cat > "../android-package/meta.json" << EOF
{
    "name": "Xclipse Tools",
    "version": "1.2.0",
    "description": "Vulkan wrapper optimized for Exynos 2400/Xclipse 940",
    "author": "Xclipse Tools Team",
    "license": "MIT",
    "target": "arm64-v8a",
    "features": [
        "BC4+ emulation",
        "Shader cache",
        "Performance optimization",
        "DXVK compatibility"
    ]
}
EOF

# Create README for Android
cat > "../android-package/README.txt" << EOF
Xclipse Tools v1.2.0
====================

Optimized Vulkan wrapper for Exynos 2400 with Xclipse 940 GPU.

Features:
- BC4+ texture compression emulation
- Enhanced shader cache
- Performance optimizations
- DXVK compatibility mode
- Dynamic rendering emulation

Installation:
1. Copy libs/arm64-v8a/libxeno_wrapper.so to your app's lib directory
2. Copy xclipse_tools.conf to your app's data directory
3. Configure the wrapper in your app

Based on community feedback from Reddit.
EOF

cd ..

echo "Build complete!"
echo "Files created:"
echo "- xclipse_tools_winlator.tar.zst (for Winlator)"
echo "- android-package/ (for Android apps)"
echo "- libxeno_wrapper.so (${BUILD_DIR}/libxeno_wrapper.so)"

# Show file sizes
echo ""
echo "File sizes:"
ls -lh "$BUILD_DIR/libxeno_wrapper.so"
ls -lh xclipse_tools_winlator.tar.zst