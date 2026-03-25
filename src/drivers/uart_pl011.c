#include "drivers/uart_pl011.h"

#define UART_BASE  0x09000000ULL
#define UART_DR    (UART_BASE + 0x000)
#define UART_FR    (UART_BASE + 0x018)
#define UART_IBRD  (UART_BASE + 0x024)
#define UART_FBRD  (UART_BASE + 0x028)
#define UART_LCRH  (UART_BASE + 0x02C)
#define UART_CR    (UART_BASE + 0x030)

#define UART_FR_TXFF (1U << 5)
#define UART_FR_RXFE (1U << 4)

static bool g_uart_ready;

void uart_pl011_init(void)
{
    writel(0x0, UART_CR);
    writel(13, UART_IBRD);
    writel(1, UART_FBRD);
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