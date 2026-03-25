#ifndef MMIO_H
#define MMIO_H

#include "generated/autoconf.h"

#define MMIO_PERIPH_BASE CONFIG_PERIPH_BASE

#define GPIO_BASE   (MMIO_PERIPH_BASE + 0x00200000ULL)
#define GPFSEL1     (GPIO_BASE + 0x04)
#define GPPUD       (GPIO_BASE + 0x94)
#define GPPUDCLK0   (GPIO_BASE + 0x98)

#define UART_BASE   CONFIG_UART_BASE

#endif