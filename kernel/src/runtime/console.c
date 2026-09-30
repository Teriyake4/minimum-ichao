#include "minemu/console.h"
#include "minemu/uart.h"

void minemu_printf(const char *str) {
    for(; *str; ++str)
        minemu_uart_putc((uint8_t) *str);
}
