#include "drivers/uart_pl011.h"
#include "generated/autoconf.h"
#include "mmio.h"

#define UART_DR    (UART_BASE + 0x000)
#define UART_FR    (UART_BASE + 0x018)
#define UART_IBRD  (UART_BASE + 0x024)
#define UART_FBRD  (UART_BASE + 0x028)
#define UART_LCRH  (UART_BASE + 0x02C)
#define UART_CR    (UART_BASE + 0x030)

#define UART_FR_TXFF (1U << 5)
#define UART_FR_RXFE (1U << 4)

static bool g_uart_ready;

static void uart_gpio_init_if_needed(void)
{
#if CONFIG_TARGET_RPI3
    u32 ra;

    ra = readl(GPFSEL1);
    ra &= ~((7U << 12) | (7U << 15));
    ra |=  (4U << 12) | (4U << 15); /* GPIO14/15 -> ALT0 (TXD0/RXD0) */
    writel(ra, GPFSEL1);

    writel(0, GPPUD);
    for (volatile u32 i = 0; i < 150; i++) { __asm__ volatile("nop"); }
    writel((1U << 14) | (1U << 15), GPPUDCLK0);
    for (volatile u32 i = 0; i < 150; i++) { __asm__ volatile("nop"); }
    writel(0, GPPUDCLK0);
#endif
}

void uart_pl011_init(void)
{
    u32 baud_div;
    u32 rem;
    u32 frac;

    uart_gpio_init_if_needed();

    baud_div = (u32)(CONFIG_UART_CLOCK_HZ / (16U * 115200U));
    rem = (u32)(CONFIG_UART_CLOCK_HZ % (16U * 115200U));
    frac = (u32)(((rem * 64U) + (115200U / 2U)) / 115200U);

    writel(0x0, UART_CR);
    writel(baud_div, UART_IBRD);
    writel(frac, UART_FBRD);
    writel(0x70, UART_LCRH);
    writel(0x301, UART_CR);
    g_uart_ready = true;
}

void uart_pl011_putc(char c)
{
    while (readl(UART_FR) & UART_FR_TXFF) {}
    writel((u32)(u8)c, UART_DR);
}

void uart_pl011_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') {
            uart_pl011_putc('\r');
        }
        uart_pl011_putc(*s++);
    }
}

char uart_pl011_getc(void)
{
    while (readl(UART_FR) & UART_FR_RXFE) {}
    return (char)(readl(UART_DR) & 0xFFU);
}

int uart_pl011_try_getc(char *out)
{
    if (readl(UART_FR) & UART_FR_RXFE) {
        return 0;
    }
    *out = (char)(readl(UART_DR) & 0xFFU);
    return 1;
}

bool uart_pl011_is_ready(void)
{
    return g_uart_ready;
}