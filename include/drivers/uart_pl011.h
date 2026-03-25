#ifndef UART_PL011_H
#define UART_PL011_H

#include "types.h"

void uart_pl011_init(void);
void uart_pl011_putc(char c);
void uart_pl011_puts(const char *s);
char uart_pl011_getc(void);
int uart_pl011_try_getc(char *out);
bool uart_pl011_is_ready(void);

#endif