Recreate my ARM64 Android-style bootloader project, but target Raspberry Pi 3 instead of QEMU `virt`.

Project goal:
Build a custom ARM64 bootloader for Raspberry Pi 3 that can load and boot a Linux/Android-style kernel image, provide an interactive shell, track boot failures, and support recovery-style fallback logic.

IMPORTANT:
- Do NOT implement ramdisk support for now.
- Do NOT depend on Android initramfs or recovery images yet.
- Focus on bootloader core + kernel boot flow.
- Keep architecture modular and extendable.
- This must be Raspberry Pi 3 specific, NOT generic ARM64.

Target environment:
- Board: Raspberry Pi 3 Model B / B+
- Architecture: ARM64 / AArch64
- Boot method: Raspberry Pi firmware loads `kernel8.img`
- UART: PL011 via GPIO (115200)
- Kernel format: raw `Image` (no ramdisk yet)
- DTB support required
- Memory: assume standard Pi 3 layout

---

CHANGES FROM QEMU VERSION (VERY IMPORTANT):
- Remove ALL QEMU assumptions (`virt` machine, 0x09000000 UART, etc.)
- Replace with Raspberry Pi 3 MMIO addresses and peripherals
- Replace QEMU boot flow with Pi firmware → kernel8.img flow
- Entry point is `_start` in `kernel8.img`, not QEMU loader

---

Main features to implement:

1. ARM64 low-level entry (assembly)
   - entry from Pi firmware
   - set up stack
   - park secondary cores
   - clear .bss
   - preserve incoming DTB pointer (x0)
   - initialize UART (Pi 3 specific)
   - jump into C main()

2. UART console (Pi 3 specific)
   - PL011 UART driver using Pi 3 MMIO base
   - configure GPIO pins for UART
   - putchar / getchar
   - logging helpers

3. Bootloader shell
   - prompt
   - command parsing
   - history
   - line editing (basic)
   - autoboot timeout
   - commands:
     - help
     - history
     - !!
     - reset

4. Environment variable subsystem
   - in-memory only
   - commands:
     - printenv
     - setenv
     - run
     - saveenv (stub)
   - bootdelay support

5. Boot image / kernel loading
   - support:
     A) embedded raw kernel `Image`
     B) simple embedded boot image header (kernel only)
   - copy kernel to runtime address
   - DTB selection:
     1) incoming x0 DTB (from firmware)
     2) embedded DTB
     3) fallback address
   - patch DTB `/chosen/bootargs`
   - jump to kernel using ARM64 Linux ABI:
     - x0 = dtb pointer
     - x1 = 0
     - x2 = 0
     - x3 = 0

6. Boot arguments
   - default:
     console=ttyAMA0,115200 earlycon loglevel=8 panic=3
   - override via env var `bootargs`
   - patch DTB before boot

7. Boot state / failure detection
   - track:
     - boot_in_progress
     - boot_success
     - fail_count
     - fail_limit
     - mode
   - in-memory backend
   - commands:
     - bootstate
     - bootsuccess
   - if fail_count >= fail_limit:
     - enter recovery menu

8. Recovery menu framework
   - simple keyboard-driven menu over UART
   - options:
     - normal boot
     - reboot
     - factory reset (stub)
   - no real recovery image yet

9. Boot monitor logs
   - print clear logs:
     [ OK ] UART initialized
     [ OK ] DTB selected
     [ OK ] Kernel loaded

10. Driver info command
   - drvinfo
   - print:
     - UART status
     - DTB presence
     - kernel presence
     - boot state backend

---

Raspberry Pi 3 specific requirements:

- Output binary must be:
  kernel8.img

- Use Raspberry Pi 3 peripheral base address:
  (define properly in mmio.h)

- UART must use PL011 with GPIO configuration

- Provide config.txt.example:
  enable_uart=1
  dtoverlay=disable-bt

- Document UART pins:
  TX: GPIO14 (pin 8)
  RX: GPIO15 (pin 10)

---

Project structure (same as original but adapted):

- src/arch/arm64_boot.s
- src/bootloader.c
- src/commands.c
- src/env.c
- src/bootstate.c
- src/drivers/uart_pl011.c
- src/drivers/dtb.c
- src/lib/string.c
- include/*
- boot/
- tools/
- Makefile
- linker.ld
- README.md

---

Build system:

- GNU Make
- toolchain:
  aarch64-none-elf-gcc (preferred)
  OR aarch64-linux-gnu-gcc (fallback)

- build target:
  make → kernel8.img

- DO NOT use QEMU run target
- Instead document how to copy kernel8.img to SD card

---

Linker / memory:

- define Pi 3 load address (firmware loads kernel8.img)
- define:
  bootloader base
  kernel load address
  DTB fallback address

---

Stubs for future work (same as original):
- persistent bootstate
- ramdisk
- GPT / partitions
- fastboot
- framebuffer
- USB
- filesystem

Do NOT implement them yet.

---

Documentation:

README must include:
- how Pi 3 boot flow works
- how kernel8.img is loaded
- how to build
- how to copy to SD card
- how to wire UART
- how to test using serial console
- limitations vs QEMU version

---

Behavior requirements:

- must compile cleanly
- no generic ARM placeholders
- must be Pi 3 specific
- code must be real, not pseudo
- keep modular and extendable

---

What I want from you:

1. Convert the existing QEMU design into Raspberry Pi 3 version
2. Replace all QEMU assumptions with Pi 3 hardware
3. Implement working UART for Pi 3
4. Keep shell + bootstate + DTB logic
5. Output kernel8.img
6. Provide full project structure
7. Provide README + config.txt example

---

Do NOT:
- keep QEMU-specific addresses
- use generic ARM placeholders
- assume virt machine
- implement ramdisk yet

Focus on:
Raspberry Pi 3 + real hardware bootloader + UART debug + kernel loading core