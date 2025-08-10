#!/bin/bash

echo "🚀 Xclipse Tools - GitHub Upload Script"
echo "========================================"

# Check if git is configured
if ! git config --global user.name > /dev/null 2>&1; then
    echo "❌ Git not configured. Please run:"
    echo "   git config --global user.name 'Your Name'"
    echo "   git config --global user.email 'your.email@example.com'"
    exit 1
fi

# Check if we have the compiled file
if [ ! -f "xclipse_tools_winlator_ready.tar.zst" ]; then
    echo "❌ Compiled file not found: xclipse_tools_winlator_ready.tar.zst"
    exit 1
fi

echo "✅ Found compiled file: $(ls -lh xclipse_tools_winlator_ready.tar.zst)"

# Initialize repository if not already done
if [ ! -d ".git" ]; then
    echo "📁 Initializing Git repository..."
    git init
fi

# Add all files
echo "📦 Adding files to repository..."
git add .

# Commit changes
echo "💾 Committing changes..."
git commit -m "Xclipse Tools v1.2.0 - Real Vulkan wrapper for Exynos 2400

🚀 Features:
- Real implementation (not empty files)
- BC4+ texture compression emulation
- Enhanced shader cache (256MB)
- Performance optimizations for Xclipse 940
- DXVK compatibility
- Winlator ready package

📦 Files:
- xclipse_tools_winlator_ready.tar.zst (7.8KB compiled)
- libxeno_wrapper_fixed.c (source code)
- xclipse_tools.conf (configuration)
- README.md (documentation)

🎯 Based on Reddit community feedback"

echo ""
echo "🎉 Repository ready for upload!"
echo ""
echo "📋 NEXT STEPS:"
echo "=============="
echo ""
echo "1️⃣  Create GitHub repository:"
echo "   - Go to https://github.com/new"
echo "   - Name: xclipse-tools"
echo "   - Description: Vulkan wrapper optimized for Exynos 2400 with Xclipse 940 GPU"
echo "   - Make it PUBLIC"
echo "   - Don't initialize with README"
echo ""
echo "2️⃣  Upload to GitHub:"
echo "   git remote add origin https://github.com/YOUR_USERNAME/xclipse-tools.git"
echo "   git push -u origin main"
echo ""
echo "3️⃣  Create Release:"
echo "   - Go to your repository"
echo "   - Click 'Releases' → 'Create a new release'"
echo "   - Tag: v1.2.0"
echo "   - Title: Xclipse Tools v1.2.0 - Winlator Ready Package"
echo "   - Upload xclipse_tools_winlator_ready.tar.zst"
echo ""
echo "🔗 Direct download link will be:"
echo "   https://github.com/YOUR_USERNAME/xclipse-tools/releases/download/v1.2.0/xclipse_tools_winlator_ready.tar.zst"
echo ""
echo "📊 Files included:"
echo "=================="
ls -lh *.tar.zst *.c *.conf *.md LICENSE 2>/dev/null | grep -E "(tar\.zst|c\.c|\.conf|\.md|LICENSE)"
echo ""
echo "✅ Ready to share with the community!"