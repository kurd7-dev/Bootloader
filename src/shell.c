#include "bootloader.h"
#include "bootloader_ext.h"
#include "drivers/uart_pl011.h"
#include "lib/string.h"

#define SHELL_LINE_MAX 160

static void shell_prompt(void)
{
    uart_pl011_puts("boot> ");
}

static int read_line(char *buf, usize cap)
{
    usize pos = 0;
    for (;;) {
        char c = uart_pl011_getc();
        if (c == '\r' || c == '\n') {
            uart_pl011_puts("\n");
            buf[pos] = '\0';
            return (int)pos;
        }
        if ((c == 0x08 || c == 0x7F) && pos > 0) {
            pos--;
            uart_pl011_puts("\b \b");
            continue;
        }
        if (c >= 0x20 && c <= 0x7E && pos + 1 < cap) {
            buf[pos++] = c;
            uart_pl011_putc(c);
        }
    }
}

static void autoboot_wait_or_break(void)
{
    int delay = env_get_bootdelay();
    int remaining;

    if (delay < 0) {
        return;
    }

    uart_pl011_puts("Autoboot in ");
    bl_putdec((u32)delay);
    uart_pl011_puts(" seconds. Press any key to stop.\n");

    for (remaining = delay; remaining > 0; remaining--) {
        u64 loops;
        uart_pl011_puts("  ");
        bl_putdec((u32)remaining);
        uart_pl011_puts("...\r");

        for (loops = 0; loops < 1500000ULL; loops++) {
            char c;
            if (uart_pl011_try_getc(&c)) {
                uart_pl011_puts("\nAutoboot interrupted.\n");
                return;
            }
        }
    }
    uart_pl011_puts("\n");
    (void)commands_execute_line("boot", false);
}

void shell_run(void)
{
    char line[SHELL_LINE_MAX];

    autoboot_wait_or_break();
    uart_pl011_puts("Interactive shell ready. Type 'help'.\n");

    for (;;) {
        shell_prompt();
        if (read_line(line, sizeof(line)) < 0) {
            continue;
        }
        (void)commands_execute_line(line, false);
    }
}