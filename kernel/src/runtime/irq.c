#include <stdint.h>

#include "minemu/irq.h"
#include "minemu/platform.h"
#include "minemu/trap.h"
#include "minemu/uart.h"

typedef void (*minemu_irq_handler_t)(void);

void minemu_uart_handler() {
    uint8_t byte;
    while(minemu_uart_getc(&byte)) {
        minemu_uart_push(byte);
    }
}

static const minemu_irq_handler_t handlers[] = {
    [MINEMU_IRQ_SYSTICK] = 0,
    [MINEMU_IRQ_UART0] = minemu_irq_uart0_handler,
    [MINEMU_IRQ_UART1] = 0,
    [MINEMU_IRQ_BLOCK] = 0,
};

struct minemu_trap_frame *minemu_irq_dispatch(struct minemu_trap_frame *frame) {
    uint32_t source = (uint32_t)frame->exception_id;
    if(source < sizeof(handlers) / sizeof(handlers[0]) && handlers[source] != 0) {
        handlers[source]();
    }
    MINEMU_INTERRUPT->eoi = source;
    return frame;
}
