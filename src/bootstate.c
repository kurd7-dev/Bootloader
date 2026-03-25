#include "bootloader.h"
#include "bootloader_ext.h"
#include "drivers/uart_pl011.h"
#include "lib/string.h"

static bootstate_t g_bootstate;

const bootstate_t *bootstate_get(void)
{
    return &g_bootstate;
}

void bootstate_init(void)
{
    bl_memset(&g_bootstate, 0, sizeof(g_bootstate));
    g_bootstate.fail_limit = 3;
    bl_strcpy(g_bootstate.mode, "normal");
    log_ok("Bootstate initialized");
}

void bootstate_on_boot_start(void)
{
    g_bootstate.boot_in_progress = true;
    g_bootstate.boot_success = false;
    bl_strcpy(g_bootstate.mode, "normal");
}

void bootstate_on_boot_failure(void)
{
    if (g_bootstate.boot_in_progress && !g_bootstate.boot_success) {
        g_bootstate.fail_count++;
    }
    g_bootstate.boot_in_progress = false;
}

void bootstate_mark_success(void)
{
    g_bootstate.boot_success = true;
    g_bootstate.boot_in_progress = false;
    g_bootstate.fail_count = 0;
}

bool bootstate_should_enter_recovery(void)
{
    if (g_bootstate.fail_count >= g_bootstate.fail_limit) {
        bl_strcpy(g_bootstate.mode, "recovery");
        return true;
    }
    return false;
}

void bootstate_print(void)
{
    uart_pl011_puts("boot_in_progress: ");
    uart_pl011_puts(g_bootstate.boot_in_progress ? "true\n" : "false\n");
    uart_pl011_puts("boot_success: ");
    uart_pl011_puts(g_bootstate.boot_success ? "true\n" : "false\n");
    uart_pl011_puts("fail_count: ");
    bl_putdec(g_bootstate.fail_count);
    uart_pl011_puts("\n");
    uart_pl011_puts("fail_limit: ");
    bl_putdec(g_bootstate.fail_limit);
    uart_pl011_puts("\n");
    uart_pl011_puts("mode: ");
    uart_pl011_puts(g_bootstate.mode);
    uart_pl011_puts("\n");
}

void recovery_menu_run(void)
{
    for (;;) {
        uart_pl011_puts("\n=== Recovery Menu ===\n");
        uart_pl011_puts("1) Try normal boot\n");
        uart_pl011_puts("2) Reboot\n");
        uart_pl011_puts("3) Factory reset (stub)\n");
        uart_pl011_puts("Select> ");

        switch (uart_pl011_getc()) {
        case '1':
            uart_pl011_puts("\nTrying normal boot...\n");
            if (boot_try_normal() == 0) {
                return;
            }
            break;
        case '2':
            uart_pl011_puts("\nRebooting...\n");
            boot_reset();
            return;
        case '3':
            uart_pl011_puts("\nFactory reset hook not implemented yet.\n");
            break;
        default:
            uart_pl011_puts("\nInvalid choice.\n");
            break;
        }
    }
}