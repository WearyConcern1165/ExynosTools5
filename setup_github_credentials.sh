#!/bin/bash

echo "🔑 GitHub Credentials Setup"
echo "============================"

# Check if git is installed
if ! command -v git &> /dev/null; then
    echo "❌ Git is not installed. Please install git first."
    exit 1
fi

echo "📋 Current Git configuration:"
echo "============================="
echo "User name: $(git config --global user.name || echo 'Not set')"
echo "User email: $(git config --global user.email || echo 'Not set')"
echo ""

# Ask for GitHub credentials
echo "🚀 Let's set up your GitHub credentials:"
echo ""

read -p "Enter your GitHub username: " github_username
read -p "Enter your GitHub email: " github_email

# Configure Git
echo ""
echo "⚙️  Configuring Git..."
git config --global user.name "$github_username"
git config --global user.email "$github_email"

echo "✅ Git configured successfully!"
echo ""

# Check if we have the repository ready
if [ -f "xclipse_tools_winlator_ready.tar.zst" ]; then
    echo "📦 Repository files found:"
    echo "=========================="
    ls -lh *.tar.zst *.c *.conf *.md LICENSE 2>/dev/null | grep -E "(tar\.zst|c\.c|\.conf|\.md|LICENSE)"
    echo ""
    
    echo "🎯 Ready to upload to GitHub!"
    echo ""
    echo "📋 NEXT STEPS:"
    echo "=============="
    echo ""
    echo "1️⃣  Create GitHub repository:"
    echo "   - Go to: https://github.com/new"
    echo "   - Repository name: xclipse-tools"
    echo "   - Description: Vulkan wrapper optimized for Exynos 2400 with Xclipse 940 GPU"
    echo "   - Make it PUBLIC"
    echo "   - Don't initialize with README"
    echo ""
    echo "2️⃣  Get Personal Access Token:"
    echo "   - Go to: https://github.com/settings/tokens"
    echo "   - Click 'Generate new token (classic)'"
    echo "   - Name: xclipse-tools-upload"
    echo "   - Expiration: 90 days"
    echo "   - Scopes: ✅ repo"
    echo "   - Copy the token (you won't see it again!)"
    echo ""
    echo "3️⃣  Upload to GitHub:"
    echo "   git remote add origin https://github.com/$github_username/xclipse-tools.git"
    echo "   git push -u origin main"
    echo "   # When prompted:"
    echo "   # Username: $github_username"
    echo "   # Password: YOUR_TOKEN"
    echo ""
    echo "4️⃣  Create Release:"
    echo "   - Go to your repository"
    echo "   - Click 'Releases' → 'Create a new release'"
    echo "   - Tag: v1.2.0"
    echo "   - Title: Xclipse Tools v1.2.0 - Winlator Ready Package"
    echo "   - Upload xclipse_tools_winlator_ready.tar.zst"
    echo ""
    echo "🔗 Direct download link will be:"
    echo "   https://github.com/$github_username/xclipse-tools/releases/download/v1.2.0/xclipse_tools_winlator_ready.tar.zst"
    echo ""
    echo "✅ Ready to share with the community!"
else
    echo "❌ Repository files not found. Please run the build script first."
    exit 1
fi