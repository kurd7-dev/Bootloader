#!/bin/bash

# Test script for QEMU with custom bootloader
# Note: This loads the kernel directly, bypassing the bootloader for testing

if [ ! -f "output/Image" ]; then
    echo "Error: Kernel image not found. Run ./build.sh first."
    exit 1
fi

echo "Starting QEMU test (direct kernel boot)..."

qemu-system-aarch64 \
    -M virt \
    -cpu cortex-a72 \
    -m 1024 \
    -kernel output/Image \
    -dtb output/generic_arm64.dtb \
    -initrd output/initramfs.img \
    -append "console=ttyAMA0,115200 androidboot.hardware=generic_arm64" \
    -nographic \
    -no-reboot

echo "QEMU test completed."