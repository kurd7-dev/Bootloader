#!/bin/bash

# Setup script for Custom ARM64 Bootloader project
# Installs required dependencies

set -e

echo "Setting up Custom ARM64 Bootloader project..."

# Install system dependencies
echo "Installing system dependencies..."
sudo apt update
sudo apt install -y \
    build-essential \
    gcc-aarch64-linux-gnu \
    device-tree-compiler \
    python3 \
    git \
    wget \
    cpio \
    gzip

echo "Dependencies installed successfully."

echo "Setup complete!"
echo "Next steps:"
echo "1. Place your compiled Android kernel Image in the kernel/ directory as 'Image'"
echo "2. Optionally place your device tree blob as 'kernel/generic_arm64.dtb'"
echo "3. Run ./build.sh to build the bootloader"