#include <stdint.h>
#include <stddef.h>

#include "minemu/irq.h"
#include "minemu/platform.h"
#include "minemu/uart.h"

static uint8_t gbuf[MINEMU_UART_RX_CAPACITY];
static size_t gbuf_head;
static size_t gbuf_tail;

void minemu_uart_init(void) {
    gbuf_head = 0;
    gbuf_tail = 0;
    MINEMU_UART0->control |= MINEMU_UART_CONTROL_RX_IRQ_ENABLE;
    MINEMU_INTERRUPT->enable |= UINT32_C(1) << MINEMU_IRQ_UART0;
}

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

int minemu_uart_getc(uint8_t *out) {
    if(!(MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY)) {
        return 0;
    }
    *out = (uint8_t)MINEMU_UART0->rx_data;
    return 1;
}

void minemu_uart_push(uint8_t byte) {
    size_t next = (gbuf_head + 1) % MINEMU_UART_RX_CAPACITY;
    if(next != gbuf_tail) {
        gbuf[gbuf_head] = byte;
        gbuf_head = next;
    }
}

int minemu_uart_try_getc(uint8_t *out) {
    minemu_irq_disable();
    if(gbuf_head == gbuf_tail) {
        minemu_irq_enable();
        return 0;
    }
    *out = gbuf[gbuf_tail];
    gbuf_tail = (gbuf_tail + 1) % MINEMU_UART_RX_CAPACITY;
    minemu_irq_enable();
    return 1;
}
