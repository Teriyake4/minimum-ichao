#include <stdint.h>
#include <stddef.h>

#include "minemu/platform.h"
#include "minemu/uart.h"

void minemu_uart_putc(uint8_t c) {
    while(!(MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY)) {
        continue;
    }
    MINEMU_UART0->tx_data = c;
}
size_t minemu_uart_write(const void *buf, size_t len) {
    const uint8_t *p = buf;
    for(size_t i = 0; i < len; ++i) {
        minemu_uart_putc(p[i]);
    }
    return len;
}
