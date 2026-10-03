#ifndef MINEMU_UART_H
#define MINEMU_UART_H

#include <stddef.h>
#include <stdint.h>

void minemu_uart_init(void);

void minemu_uart_putc(uint8_t c);
size_t minemu_uart_write(const void *buf, size_t len);
int minemu_uart_getc(uint8_t *out);

/* UART0 RX interrupt handler; drains the hardware buffer into the ring. */
void minemu_uart0_irq_handler(void);

void minemu_uart_push(uint8_t byte);
int minemu_uart_try_getc(uint8_t *out);

#endif
