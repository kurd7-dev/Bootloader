/*
 * UART driver for ARM64 PL011
 */

#include "uart.h"
#include "types.h"

// PL011 UART registers
#define UART_BASE 0x9000000
#define UART_DR   (UART_BASE + 0x00)
#define UART_FR   (UART_BASE + 0x18)
#define UART_IBRD (UART_BASE + 0x24)
#define UART_FBRD (UART_BASE + 0x28)
#define UART_LCRH (UART_BASE + 0x2C)
#define UART_CR   (UART_BASE + 0x30)

#define UART_FR_TXFF (1 << 5)
#define UART_FR_RXFE (1 << 4)

void uart_init(void) {
    // Disable UART
    writel(0, UART_CR);
    
    // Set baud rate (115200 @ 24MHz)
    writel(13, UART_IBRD);
    writel(1, UART_FBRD);
    
    // 8N1, FIFO enabled
    writel(0x70, UART_LCRH);
    
    // Enable UART, TX, RX
    writel(0x301, UART_CR);
}

void uart_putc(char c) {
    while (readl(UART_FR) & UART_FR_TXFF);
    writel(c, UART_DR);
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}

char uart_getc(void) {
    while (readl(UART_FR) & UART_FR_RXFE);
    return readl(UART_DR) & 0xFF;
}