# ARM64 QEMU Bootloader Core

Custom AArch64 bootloader core for QEMU `virt`, focused on clean architecture and extensibility.

## Implemented features

- ARM64 low-level entry (`src/arch/arm64_boot.s`)
  - stack setup
  - `.bss` clear
  - incoming DTB pointer capture from `x0`
  - EL1 vector table installation
- PL011 UART driver (`src/drivers/uart_pl011.c`)
  - blocking TX/RX
  - non-blocking RX (`uart_pl011_try_getc`)
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
  - copy kernel to runtime address `0x52000000`
  - ARM64 Linux boot ABI jump (`x0=dtb`, `x1..x3=0`)
- DTB handling (`src/drivers/dtb.c`)
  - DTB selection priority:
    1) incoming DTB (`x0`)
    2) embedded DTB blob section
    3) fallback address (`0x42000000`)
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

## Build

Requirements:
- `aarch64-linux-gnu-gcc`
- `aarch64-linux-gnu-ld`
- `aarch64-linux-gnu-objcopy`
- `qemu-system-aarch64` (for run)

Commands:

```bash
make
make run
make clean
```

`make run` uses:

```bash
qemu-system-aarch64 -machine virt -cpu cortex-a72 -m 2048 -nographic -kernel bootloader
```

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

- Bootloader base: `0x40000000`
- Kernel runtime load: `0x52000000`
- DTB fallback: `0x42000000`
- UART base: `0x09000000`

Default bootargs:

```text
console=ttyAMA0,115200 earlycon=pl011,0x09000000 loglevel=8 ignore_loglevel panic=3 oops=panic
```

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
- `boot/`
- `tools/`

## Roadmap

1. Add binary embedding flow for kernel and optional DTB from `boot/`
2. Improve DTB editing to create missing `/chosen/bootargs`
3. Add persistent env + bootstate backends
4. Add GPT/filesystem loading
5. Add fastboot + USB framework
