#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "types.h"

#define BOOTLOADER_BASE_ADDR   0x40000000ULL
#define KERNEL_RUNTIME_ADDR    0x52000000ULL
#define DTB_FALLBACK_ADDR      0x42000000ULL
#define QEMU_UART_BASE         0x09000000ULL

#define DEFAULT_BOOTARGS \
    "console=ttyAMA0,115200 earlycon=pl011,0x09000000 " \
    "loglevel=8 ignore_loglevel panic=3 oops=panic"

typedef struct {
    bool uart_ready;
    bool incoming_dtb_valid;
    bool embedded_dtb_valid;
    bool fallback_dtb_valid;
    bool kernel_blob_present;
    bool bootstate_backend_volatile;
} drvinfo_t;

extern volatile u64 g_incoming_dtb_ptr;

void bootloader_main(void);
int boot_try_normal(void);
void boot_jump_to_kernel(u64 kernel_addr, u64 dtb_addr);
void boot_panic_halt(const char *msg);
void boot_exception_panic(void);
void boot_reset(void);

void log_info(const char *msg);
void log_ok(const char *msg);
void log_err(const char *msg);
void log_warn(const char *msg);

void bl_delay_cycles(u64 cycles);
void bl_puthex(u64 val);
void bl_putdec(u32 val);

void drvinfo_collect(drvinfo_t *out);
void drvinfo_print(void);

#endif