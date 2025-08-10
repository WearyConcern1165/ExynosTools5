#!/bin/bash

echo "Creating Winlator-ready package for Xclipse Tools..."

# Create temporary directory structure
TEMP_DIR="winlator_package_temp"
rm -rf "$TEMP_DIR"
mkdir -p "$TEMP_DIR/usr/lib"

# Copy the wrapper library
cp libxeno_wrapper.so "$TEMP_DIR/usr/lib/"

# Make it executable
chmod +x "$TEMP_DIR/usr/lib/libxeno_wrapper.so"

# Create the tar.zst package for Winlator
cd "$TEMP_DIR"
tar -czf ../xclipse_tools_winlator_ready.tar.zst usr/
cd ..

# Clean up
rm -rf "$TEMP_DIR"

echo "Winlator package created: xclipse_tools_winlator_ready.tar.zst"
echo ""
echo "To install in Winlator:"
echo "1. Copy xclipse_tools_winlator_ready.tar.zst to your Winlator directory"
echo "2. Extract it: tar -xzf xclipse_tools_winlator_ready.tar.zst"
echo "3. The wrapper will be available at /usr/lib/libxeno_wrapper.so"
echo ""
echo "Package contents:"
echo "- usr/lib/libxeno_wrapper.so (Xclipse Tools wrapper)"
echo ""
echo "Ready for Winlator installation!"