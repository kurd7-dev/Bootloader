# Custom ARM64 Baremetal Bootloader

This project provides a custom baremetal bootloader written in C/C++ for ARM64 architecture that can load and run a real Android kernel from storage with drivers.

## Features

- **Custom C/C++ Implementation**: No external dependencies like U-Boot
- **ARM64 Baremetal**: Direct hardware access with minimal overhead
- **Storage Drivers**: eMMC/SD card support for loading kernel
- **UART Debug**: Serial console output for debugging
- **Android Kernel Support**: Proper ARM64 kernel boot protocol
- **Device Tree**: Support for hardware description
- **Minimal Footprint**: Optimized for embedded systems

## Architecture

```
BootROM → Custom Bootloader (C/C++) → Android Kernel → Android System
    ↓              ↓
Hardware        Storage Access
Initialization  (MMC/SD)
```

## Project Structure

```
.
├── src/                    # Source code
│   ├── main.c             # Main bootloader logic
│   ├── uart.c             # UART driver
│   ├── mmc.c              # MMC/SD card driver
│   └── startup.s          # ARM64 startup assembly
├── include/               # Header files
│   ├── types.h           # Type definitions
│   ├── uart.h            # UART interface
│   └── mmc.h             # MMC interface
├── kernel/                # Kernel files (user provided)
│   ├── Image             # Android kernel image
│   └── generic_arm64.dtb # Device tree blob (optional)
├── dts/                   # Device tree sources
├── output/                # Build artifacts
├── Makefile              # Build configuration
├── linker.ld             # Linker script
├── build.sh              # Main build script
├── setup.sh              # Dependency setup
└── README.md             # This file
```

## Prerequisites

### System Dependencies
```bash
sudo apt install -y build-essential gcc-aarch64-linux-gnu device-tree-compiler cpio gzip
```

## Setup

1. Clone this repository
2. Run setup script:
   ```bash
   ./setup.sh
   ```
3. **Place your compiled Android kernel Image in the `kernel/` directory as `Image`**
4. Optionally place your device tree blob as `kernel/generic_arm64.dtb`
5. Run the build:
   ```bash
   ./build.sh
   ```

## Build Process

The build script performs the following steps:

1. Compiles the custom bootloader C/C++ code
2. Links with ARM64 startup code
3. Copies kernel image and device tree
4. Creates minimal initramfs if needed
5. Prepares files for deployment

## Kernel Preparation

### Getting Android Kernel Image

You need a compiled Android kernel Image for ARM64. You can:

1. **From AOSP**: Build your own kernel
   ```bash
   cd android-kernel-source
   make defconfig ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
   make Image ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
   cp arch/arm64/boot/Image /path/to/bootloader/kernel/
   ```

2. **From Device Source**: Extract from device firmware
3. **Pre-compiled**: Use from device vendor (if available)

### Device Tree

The bootloader uses a device tree blob (DTB) to describe hardware:

- If you have a specific DTB for your device, place it as `kernel/generic_arm64.dtb`
- Otherwise, the generic DTB from `dts/generic_arm64.dts` will be used

## Storage Layout

The bootloader expects the following layout on the SD card:

```
Offset          Size        Content
0x000000        2KB         Bootloader
0x100000        ~20MB       Kernel Image
0x200000        ~1MB        Device Tree Blob
0x300000        ~10MB       Initramfs (optional)
```

## Deployment to Hardware

For real hardware deployment:

1. Identify your storage device (e.g., `/dev/sdb`)
2. Write the bootloader:
   ```bash
   sudo dd if=output/bootloader.bin of=/dev/sdb bs=512 seek=4
   ```
3. Write the kernel:
   ```bash
   sudo dd if=output/Image of=/dev/sdb bs=512 seek=2048
   ```
4. Write device tree:
   ```bash
   sudo dd if=output/generic_arm64.dtb of=/dev/sdb bs=512 seek=4096
   ```
5. Write initramfs (optional):
   ```bash
   sudo dd if=output/initramfs.img of=/dev/sdb bs=512 seek=6144
   ```

## Testing with QEMU

After building, test the kernel directly with QEMU:

```bash
./test_qemu.sh
```

This bypasses the bootloader for testing purposes.

## Customization

### Hardware Support

To add support for different hardware:

1. **Modify Device Tree** (`dts/generic_arm64.dts`):
   - Update memory map
   - Add peripheral nodes
   - Configure interrupts

2. **Update Drivers** (`src/mmc.c`, `src/uart.c`):
   - Change register addresses
   - Modify initialization sequences
   - Add new peripherals

3. **Storage Layout** (`src/main.c`):
   - Adjust `KERNEL_OFFSET`, `DTB_OFFSET`, etc.
   - Change load addresses if needed

### Boot Arguments

Modify `CMDLINE` in `src/main.c` for different kernel parameters.

## Debug Output

The bootloader provides debug output via UART:

- Hardware initialization status
- Storage access results
- Kernel loading progress
- Boot success/failure

Connect a serial console at 115200 baud to see the output.

## Limitations

- **Simplified MMC Driver**: Full SD card protocol not implemented
- **No USB Support**: Only MMC/SD storage
- **Basic Error Handling**: Minimal recovery mechanisms
- **Single Core**: No SMP support in bootloader

## Extending the Bootloader

### Adding Features

1. **USB Boot**: Add USB driver for loading from USB devices
2. **Network Boot**: Add Ethernet driver for PXE-like booting
3. **Secure Boot**: Add signature verification
4. **Menu System**: Add boot menu for multiple kernels
5. **Recovery Mode**: Add fallback boot options

### Code Structure

- `startup.s`: ARM64 exception handling and initialization
- `main.c`: Boot flow and kernel loading
- `uart.c`: Debug console
- `mmc.c`: Storage access
- `types.h`: Common type definitions

## Troubleshooting

### Common Issues

1. **No UART Output**: Check UART base address in `uart.c`
2. **Storage Not Detected**: Verify MMC base address and pinmux
3. **Kernel Not Loading**: Check storage layout and offsets
4. **Boot Hangs**: Verify DTB compatibility and kernel config

### Debug Tips

- Add more `uart_puts()` calls for debugging
- Check register values with custom debug functions
- Use QEMU with `-S -s` for gdb debugging
- Verify kernel Image with `file` command

## License

This bootloader implementation is provided as-is for educational and development purposes.