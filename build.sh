#!/bin/bash

# Main build script for Baremetal Bootloader project
# Builds U-Boot, kernel, and creates boot images

set -e

export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

echo "Building Baremetal Bootloader for ARM64 Android..."

# Check prerequisites
if [ ! -d "kernel" ] || [ ! -f "kernel/Makefile" ]; then
    echo "Error: Android kernel source not found in kernel/ directory"
    echo "Please place your kernel source there and run again."
    exit 1
fi

if [ ! -d "u-boot" ]; then
    echo "Error: U-Boot source not found. Run ./setup.sh first."
    exit 1
fi

# Create output directory
mkdir -p output

# Build U-Boot
echo "Building U-Boot..."
cd u-boot

# Copy our defconfig
cp ../configs/generic_arm64_defconfig configs/

# Configure
make generic_arm64_defconfig

# Build SPL
make spl

# Build U-Boot proper
make

# Copy artifacts
cp spl/u-boot-spl.bin ../output/
cp u-boot.bin ../output/

cd ..

# Build Device Tree
echo "Building device tree..."
dtc -I dts -O dtb -o output/generic_arm64.dtb dts/generic_arm64.dts

# Build Kernel
echo "Building Android kernel..."
cd kernel

# Generate defconfig (generic ARM64)
make defconfig ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-

# Build kernel image
make Image ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc)

# Build modules if needed
make modules ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc)

# Copy kernel image
cp arch/arm64/boot/Image ../output/

cd ..

# Create initramfs (minimal for testing)
echo "Creating minimal initramfs..."
mkdir -p initramfs
cd initramfs
mkdir -p bin sbin etc proc sys dev
# Create minimal init script
cat > init << 'EOF'
#!/bin/sh
mount -t proc proc /proc
mount -t sysfs sys /sys
echo "Android kernel booted successfully!"
exec /bin/sh
EOF
chmod +x init

# Create cpio archive
find . | cpio -H newc -o | gzip > ../output/initramfs.img
cd ..
rm -rf initramfs

# Create Android boot image
echo "Creating Android boot image..."
if command -v mkbootimg &> /dev/null; then
    mkbootimg \
        --kernel output/Image \
        --ramdisk output/initramfs.img \
        --dtb output/generic_arm64.dtb \
        --cmdline "console=ttyAMA0,115200 androidboot.hardware=generic_arm64" \
        --base 0x80000000 \
        --kernel_offset 0x00080000 \
        --ramdisk_offset 0x01000000 \
        --dtb_offset 0x00000000 \
        --pagesize 4096 \
        --output output/boot.img
else
    echo "Warning: mkbootimg not found. Skipping boot.img creation."
    echo "Install with: sudo apt install android-tools-mkbootimg"
fi

echo "Build complete!"
echo "Output files in output/ directory:"
ls -la output/

echo ""
echo "To test with QEMU:"
echo "qemu-system-aarch64 -M virt -cpu cortex-a72 -m 1024 -kernel output/Image -dtb output/generic_arm64.dtb -initrd output/initramfs.img -append \"console=ttyAMA0\" -nographic"