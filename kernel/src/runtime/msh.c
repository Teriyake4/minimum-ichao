#include <stddef.h>
#include <stdint.h>

#include "minemu/console.h"
#include "minemu/msh.h"
#include "minemu/uart.h"

#define MSH_BACKSPACE_CTRL_H 0x08
#define MSH_BACKSPACE_DEL 0x7f

static int msh_eq(const char *a, const char *b) {
    for(;;) {
        if(*a != *b) {
            return 0;
        }
        if(*a == '\0') {
            return 1;
        }
        ++a;
        ++b;
    }
}

static void msh_read_line(char *line, int max) {
    int len = 0;
    for(;;) {
        uint8_t c;
        if(!minemu_uart_try_getc(&c)) {
            continue;
        }
        if(c == '\n') {
            line[len] = '\0';
            break;
        } else if(c == MSH_BACKSPACE_CTRL_H || c == MSH_BACKSPACE_DEL) {
            if(len > 0) {
                --len;
            }
        } else if(c == 0x0d) {
            // swallow CR from CRLF input; the LF that follows terminates
        } else if(len < max) {
            line[len++] = (char)c;
        }
    }
}

static void msh_exec(const char *line) {
    const char *p = line;
    while(*p == ' ') {
        ++p;
    }
    if(*p == '\0') {
        return;
    }

    char cmd[MSH_MAX_LINE + 1];
    int cmd_len = 0;
    while(*p != '\0' && *p != ' ') {
        if(cmd_len < MSH_MAX_LINE) {
            cmd[cmd_len++] = *p;
        }
        ++p;
    }
    cmd[cmd_len] = '\0';

    if(msh_eq(cmd, "echo")) {
        while(*p == ' ') {
            ++p;
        }
        minemu_printf(p);
        minemu_uart_putc('\n');
    } else {
        minemu_printf("command not found: ");
        minemu_printf(cmd);
        minemu_uart_putc('\n');
    }
}

void minemu_msh_run(void) {
    char line[MSH_MAX_LINE + 1];
    for(;;) {
        minemu_printf("msh> ");
        msh_read_line(line, MSH_MAX_LINE);
        msh_exec(line);
    }
}
