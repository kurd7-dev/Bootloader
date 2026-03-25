Recreate my ARM64 Android bootloader project from scratch in a clean, organized way.

Project goal:
Build a custom ARM64 bootloader for QEMU `virt` that can load and boot an Android/Linux kernel image, provide an interactive shell, track boot failures, and support recovery-style fallback logic.

IMPORTANT:
- Do NOT implement ramdisk support for now.
- Do NOT require a real Android initramfs.
- Do NOT depend on recovery ramdisk, boot ramdisk, or GSI system images yet.
- Focus only on the bootloader core and kernel boot flow.
- Keep the architecture modular and easy to extend later.

Target environment:
- QEMU `virt`
- ARM64 / AArch64
- UART console using PL011
- Kernel passed as a raw `Image` or embedded kernel blob
- DTB support required
- Memory size default: 2048 MB

Main features to implement:
1. ARM64 low-level entry in assembly
   - set up stack
   - save incoming DTB pointer from x0
   - initialize UART
   - jump into C main()
   - basic exception vector table
   - simple panic halt / reboot support

2. UART console
   - PL011 UART driver
   - putchar / getchar
   - non-blocking getchar for shell timeout
   - logging helpers

3. Bootloader shell
   - prompt
   - command parsing
   - history
   - basic line editing
   - autoboot timeout
   - `help`
   - `history`
   - `!!`
   - `reset`

4. Environment variable subsystem
   - in-memory only for now
   - commands:
     - `printenv`
     - `setenv`
     - `run`
     - `saveenv` (stub only, print message)
   - bootdelay support

5. Boot image / kernel loading
   - For now, support either:
     A) embedded raw kernel `Image`
     OR
     B) a simple embedded legacy Android-style boot image header containing only kernel info
   - No ramdisk loading yet
   - Copy kernel to runtime address
   - choose DTB from:
     1) incoming x0 DTB
     2) embedded DTB
     3) fallback DTB address
   - patch DTB `/chosen/bootargs`
   - jump to kernel with ARM64 Linux boot ABI:
     - x0 = dtb pointer
     - x1 = 0
     - x2 = 0
     - x3 = 0

6. Boot arguments
   - default bootargs:
     `console=ttyAMA0,115200 earlycon=pl011,0x09000000 loglevel=8 ignore_loglevel panic=3 oops=panic`
   - allow overriding through env var `bootargs`
   - patch DTB with the selected bootargs before boot

7. Boot state / failure detector
   - track:
     - boot_in_progress
     - boot_success
     - fail_count
     - fail_limit
     - mode
   - volatile backend only for now
   - commands:
     - `bootstate`
     - `bootsuccess`
   - if fail_count >= fail_limit:
     - enter recovery menu mode (menu only for now; no ramdisk boot yet)

8. Recovery menu framework
   - keyboard-driven menu with:
     - try normal boot
     - reboot
     - factory reset hook (stub only)
   - no real recovery kernel or ramdisk yet
   - just implement framework and commands cleanly

9. Boot monitor / debug output
   - BIOS-like stage logs:
     - `[ OK ] UART initialized`
     - `[ OK ] DTB selected`
     - `[ OK ] Kernel copied`
     - etc.
   - keep logs readable

10. Driver info command
   - `drvinfo`
   - print status of:
     - UART
     - DTB
     - kernel blob presence
     - boot state backend

11. Clean repository structure
Create a maintainable structure like:
- `src/arch/arm64_boot.s`
- `src/bootloader.c`
- `src/commands.c`
- `src/env.c`
- `src/bootstate.c`
- `src/drivers/uart_pl011.c`
- `src/drivers/dtb.c`
- `src/lib/string.c`
- `include/bootloader.h`
- `include/bootloader_ext.h`
- `include/drivers/uart_pl011.h`
- `boot/` for embedded blobs
- `tools/` for helper scripts
- `Makefile`
- `linker.ld`
- `README.md`

12. Build system
   - GNU Make
   - cross compile with:
     - `aarch64-linux-gnu-gcc`
     - `aarch64-linux-gnu-as`
     - `aarch64-linux-gnu-ld`
   - build target:
     - `make`
   - run target:
     - `make run`
   - QEMU run command should use:
     - `qemu-system-aarch64 -machine virt -cpu cortex-a72 -m 2048 -nographic -kernel bootloader`
   - add `make clean`

13. Configuration
   - default addresses:
     - bootloader base: `0x40000000`
     - kernel runtime load: `0x52000000`
     - DTB fallback: `0x42000000`
   - keep these in headers / config constants

14. Stubs to prepare future work
Add clean placeholder APIs for future expansion:
- persistent bootstate backend
- ramdisk support
- GPT / partition loading
- fastboot
- framebuffer / splash screen
- USB
- filesystem loading
But do NOT implement them now.

15. Documentation
Write a clear README describing:
- current implemented features
- current limitations
- build commands
- run commands
- shell commands
- future roadmap

Behavior requirements:
- must compile cleanly
- avoid unnecessary complexity
- keep code readable
- keep functions small
- organize for extension
- do not include fake Android userspace or ramdisk logic yet

What I want from you:
1. Create the full project files
2. Implement the bootloader core
3. Implement the shell and bootstate logic
4. Implement DTB bootargs patching
5. Implement the Makefile and linker script
6. Add README
7. Make sure `make` and `make run` are wired correctly

Do not add ramdisk support yet.
Do not add TWRP/GSI logic yet.
Do not add Samsung-specific recovery logic yet.
Just rebuild the clean bootloader core with the features above.