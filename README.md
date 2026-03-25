# ARM64 Bootloader Core (Raspberry Pi 3 + QEMU via config)

Custom AArch64 bootloader core with a Kconfig-style target selection flow.

Primary hardware target is **Raspberry Pi 3** (as requested), while retaining **QEMU virt** as an optional config target.

## Implemented features

- ARM64 low-level entry (`src/arch/arm64_boot.s`)
  - stack setup
  - secondary core parking
  - `.bss` clear
  - incoming DTB pointer capture from `x0`
  - EL1 vector table installation
- PL011 UART driver (`src/drivers/uart_pl011.c`)
  - blocking TX/RX
  - non-blocking RX (`uart_pl011_try_getc`)
  - Raspberry Pi 3 GPIO14/15 ALT0 setup when `CONFIG_TARGET_RPI3=y`
- Boot monitor logs (`[ OK ]`, `[FAIL]`, etc.)
- Interactive shell (`src/shell.c`, `src/commands.c`)
  - prompt
  - command parsing
  - simple line editing (backspace)
  - command history
  - `!!` repeat-last
  - autoboot timeout via `bootdelay`
- Environment subsystem (`src/env.c`, in-memory only)
  - `printenv`, `setenv`, `run`, `saveenv` (stub)
  - `bootdelay`, `bootcmd`, `bootargs`
- Kernel boot flow (`src/bootloader.c`)
  - embedded raw kernel blob path
  - simple Android legacy header detection (`ANDROID!`) path
  - copy kernel to runtime address from config
  - ARM64 Linux boot ABI jump (`x0=dtb`, `x1..x3=0`)
- DTB handling (`src/drivers/dtb.c`)
  - DTB selection priority:
    1) incoming DTB (`x0`)
    2) embedded DTB blob section
    3) fallback address (from config)
  - `/chosen/bootargs` patching (when existing property is present and large enough)
- Bootstate + recovery framework (`src/bootstate.c`)
  - `boot_in_progress`, `boot_success`, `fail_count`, `fail_limit`, `mode`
  - `bootstate`, `bootsuccess` commands
  - recovery menu framework (`try normal boot`, `reboot`, `factory reset hook stub`)
- Driver information command
  - `drvinfo`
- Future expansion stubs (`src/stubs.c`)
  - persistent bootstate backend, ramdisk, GPT, fastboot, framebuffer/splash, USB, FS

## Current limitations

- No ramdisk support (intentionally not implemented yet)
- No persistent environment backend
- No persistent bootstate backend
- No partition/filesystem loading
- Embedded kernel/DTB blobs are not provided by default
- DTB bootargs patch currently updates existing `bootargs` property; it does not create missing nodes/properties
- Linker currently emits a non-fatal RWX LOAD segment warning

## Build

Requirements:
- `python3`
- `aarch64-none-elf-*` toolchain (preferred) or `aarch64-linux-gnu-*` (fallback)
- `qemu-system-aarch64` (only if using QEMU target)

### Select target configuration

```bash
# QEMU virt target
make qemu_virt_defconfig

# Raspberry Pi 3 target
make rpi3_defconfig
```

Commands:

```bash
make
make clean
```

If QEMU defconfig is active, `make run` uses:

```bash
qemu-system-aarch64 -machine virt -cpu cortex-a72 -m 2048 -nographic -kernel bootloader
```

If RPi3 defconfig is active, `make` outputs **`kernel8.img`** and `make run` is intentionally disabled.

### Raspberry Pi 3 deployment

1. Run:

```bash
make rpi3_defconfig
make
```

2. Copy `kernel8.img` to FAT boot partition on SD card.
3. Copy `configs/config.txt.example` as `config.txt` (or merge its settings).
4. Connect UART:
   - TX: GPIO14 (pin 8)
   - RX: GPIO15 (pin 10)
   - GND: any ground pin
5. Open serial console at `115200 8N1`.

## Shell commands

- `help`
- `history`
- `!!`
- `boot`
- `reset`
- `printenv`
- `setenv <key> <value>`
- `run <key>`
- `saveenv`
- `bootstate`
- `bootsuccess`
- `drvinfo`
- `recovery`

## Memory/config defaults

Values are driven from `.config` (generated from defconfig), not hardcoded.

Typical QEMU values:
- Bootloader base: `0x40000000`
- Kernel runtime load: `0x52000000`
- DTB fallback: `0x42000000`
- UART base: `0x09000000`

Typical Raspberry Pi 3 values:
- Bootloader base: `0x00080000`
- Kernel runtime load: `0x00280000`
- DTB fallback: `0x02600000`
- UART base: `0x3F201000`
- Peripheral base: `0x3F000000`

Default bootargs:

Configured per-target via defconfig.

## Kconfig/defconfig files

- `Kconfig`
- `configs/qemu_virt_defconfig`
- `configs/rpi3_defconfig`
- `scripts/genconfig.py` (generates `include/generated/autoconf.h` and `include/generated/config.mk`)

## Repository layout

- `src/arch/arm64_boot.s`
- `src/bootloader.c`
- `src/commands.c`
- `src/env.c`
- `src/bootstate.c`
- `src/shell.c`
- `src/drivers/uart_pl011.c`
- `src/drivers/dtb.c`
- `src/lib/string.c`
- `src/stubs.c`
- `include/bootloader.h`
- `include/bootloader_ext.h`
- `include/drivers/uart_pl011.h`
- `include/drivers/dtb.h`
- `include/lib/string.h`
- `include/mmio.h`
- `boot/`
- `tools/`

## Roadmap

1. Add binary embedding flow for kernel and optional DTB from `boot/`
2. Improve DTB editing to create missing `/chosen/bootargs`
3. Add persistent env + bootstate backends
4. Add GPT/filesystem loading
5. Add fastboot + USB framework

## Raspberry Pi 3 firmware note

Example firmware config is provided at:

- `configs/config.txt.example`

Required key lines:

```text
enable_uart=1
dtoverlay=disable-bt
```
