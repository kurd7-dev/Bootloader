#include "bootloader.h"
#include "bootloader_ext.h"
#include "drivers/dtb.h"
#include "drivers/uart_pl011.h"
#include "lib/string.h"

volatile u64 g_incoming_dtb_ptr;

extern u8 __kernel_blob_start[];
extern u8 __kernel_blob_end[];

typedef struct {
    u8 magic[8];
    u32 kernel_size;
    u32 kernel_addr;
    u32 ramdisk_size;
    u32 ramdisk_addr;
    u32 second_size;
    u32 second_addr;
    u32 tags_addr;
    u32 page_size;
    u32 header_version;
    u32 os_version;
    u8 name[16];
    u8 cmdline[512];
} android_boot_img_hdr_t;

static bool parse_kernel_blob(const u8 **kernel_src, usize *kernel_sz)
{
    const u8 *blob = __kernel_blob_start;
    usize blob_sz = (usize)(__kernel_blob_end) - (usize)(__kernel_blob_start);

    if (blob_sz == 0) {
        return false;
    }

    if (blob_sz >= sizeof(android_boot_img_hdr_t) &&
        bl_strncmp((const char *)blob, "ANDROID!", 8) == 0) {
        const android_boot_img_hdr_t *hdr = (const android_boot_img_hdr_t *)blob;
        u32 page_size = hdr->page_size ? hdr->page_size : 4096U;
        u32 kernel_size = hdr->kernel_size;
        u32 kernel_off = page_size;
        if ((usize)kernel_off + (usize)kernel_size > blob_sz) {
            return false;
        }
        *kernel_src = blob + kernel_off;
        *kernel_sz = kernel_size;
        log_ok("Android boot header detected");
        return true;
    }

    *kernel_src = blob;
    *kernel_sz = blob_sz;
    return true;
}

static void print_tag(const char *tag, const char *msg)
{
    uart_pl011_puts(tag);
    uart_pl011_puts(msg);
    uart_pl011_puts("\n");
}

void log_info(const char *msg) { print_tag("[ .. ] ", msg); }
void log_ok(const char *msg) { print_tag("[ OK ] ", msg); }
void log_err(const char *msg) { print_tag("[FAIL] ", msg); }
void log_warn(const char *msg) { print_tag("[WARN] ", msg); }

void bl_puthex(u64 val)
{
    static const char *hex = "0123456789abcdef";
    int i;
    uart_pl011_puts("0x");
    for (i = 60; i >= 0; i -= 4) {
        uart_pl011_putc(hex[(val >> i) & 0xF]);
    }
}

void bl_putdec(u32 val)
{
    char buf[11];
    int i = 0;
    if (val == 0) {
        uart_pl011_putc('0');
        return;
    }
    while (val > 0 && i < 10) {
        buf[i++] = (char)('0' + (val % 10U));
        val /= 10U;
    }
    while (--i >= 0) {
        uart_pl011_putc(buf[i]);
    }
}

void bl_delay_cycles(u64 cycles)
{
    while (cycles--) {
        __asm__ volatile("nop");
    }
}

void boot_exception_panic(void)
{
    boot_panic_halt("Unhandled exception");
}

void boot_panic_halt(const char *msg)
{
    log_err(msg);
    for (;;) {
        __asm__ volatile("wfe");
    }
}

void boot_reset(void)
{
    /* PSCI SYSTEM_RESET: fid=0x84000009 */
    register u64 x0 __asm__("x0") = 0x84000009ULL;
    register u64 x1 __asm__("x1") = 0;
    register u64 x2 __asm__("x2") = 0;
    register u64 x3 __asm__("x3") = 0;
    __asm__ volatile("smc #0" : "+r"(x0) : "r"(x1), "r"(x2), "r"(x3) : "x4", "x5", "x6", "x7", "memory");
    boot_panic_halt("reset failed");
}

void boot_jump_to_kernel(u64 kernel_addr, u64 dtb_addr)
{
    __asm__ volatile(
        "mov x0, %0\n"
        "mov x1, xzr\n"
        "mov x2, xzr\n"
        "mov x3, xzr\n"
        "br %1\n"
        :
        : "r"(dtb_addr), "r"(kernel_addr)
        : "x0", "x1", "x2", "x3", "memory");
}

void drvinfo_collect(drvinfo_t *out)
{
    bl_memset(out, 0, sizeof(*out));
    out->uart_ready = uart_pl011_is_ready();
    out->incoming_dtb_valid = dtb_is_valid((void *)(usize)g_incoming_dtb_ptr);
    out->embedded_dtb_valid = dtb_has_embedded_blob();
    out->fallback_dtb_valid = dtb_is_valid((void *)(usize)DTB_FALLBACK_ADDR);
    out->kernel_blob_present = ((usize)(__kernel_blob_end) > (usize)(__kernel_blob_start));
    out->bootstate_backend_volatile = true;
}

void drvinfo_print(void)
{
    drvinfo_t i;
    drvinfo_collect(&i);
    uart_pl011_puts("Driver info:\n");
    uart_pl011_puts("  UART: "); uart_pl011_puts(i.uart_ready ? "ready\n" : "not ready\n");
    uart_pl011_puts("  DTB(incoming): "); uart_pl011_puts(i.incoming_dtb_valid ? "yes\n" : "no\n");
    uart_pl011_puts("  DTB(embedded): "); uart_pl011_puts(i.embedded_dtb_valid ? "yes\n" : "no\n");
    uart_pl011_puts("  DTB(fallback): "); uart_pl011_puts(i.fallback_dtb_valid ? "yes\n" : "no\n");
    uart_pl011_puts("  Kernel blob: "); uart_pl011_puts(i.kernel_blob_present ? "yes\n" : "no\n");
    uart_pl011_puts("  Bootstate backend: volatile\n");
}

int boot_try_normal(void)
{
    void *dtb;
    const char *bootargs;
    const u8 *kernel_src;
    usize kernel_sz;

    bootstate_on_boot_start();
    log_info("normal boot path");

    dtb = dtb_select(g_incoming_dtb_ptr);
    if (!dtb) {
        log_err("No valid DTB found");
        bootstate_on_boot_failure();
        return -1;
    }
    log_ok("DTB selected");

    bootargs = env_get("bootargs");
    if (!bootargs || !*bootargs) {
        bootargs = DEFAULT_BOOTARGS;
    }
    if (dtb_patch_bootargs(dtb, bootargs) == 0) {
        log_ok("DTB /chosen/bootargs patched");
    } else {
        log_warn("DTB bootargs patch failed (continuing)");
    }

    if (!parse_kernel_blob(&kernel_src, &kernel_sz)) {
        log_err("No embedded kernel blob present");
        bootstate_on_boot_failure();
        return -1;
    }

    bl_memcpy((void *)(usize)KERNEL_RUNTIME_ADDR, kernel_src, kernel_sz);
    log_ok("Kernel copied");

    bootstate_mark_success();
    log_ok("Jumping to kernel");
    boot_jump_to_kernel(KERNEL_RUNTIME_ADDR, (u64)(usize)dtb);
    return 0;
}

void bootloader_main(void)
{
    uart_pl011_init();
    log_ok("UART initialized");
    env_init();
    bootstate_init();

    (void)bootstate_persistent_backend_init();
    (void)ramdisk_support_init();
    (void)partition_gpt_init();
    (void)fastboot_init();
    (void)framebuffer_splash_init();
    (void)usb_stack_init();
    (void)filesystem_loader_init();

    if (bootstate_should_enter_recovery()) {
        log_warn("Fail limit reached, entering recovery menu");
        recovery_menu_run();
    }

    shell_run();
    boot_panic_halt("shell returned unexpectedly");
}