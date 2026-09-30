#include <stdint.h>
#include <stddef.h>

void minemu_uart_putc(uint8_t c);
size_t minemu_uart_write(const void *buf, size_t len);
