#include <stddef.h>
#include <stdint.h>

#include "minemu/msh.h"
#include "minemu/user_abi.h"

#define MSH_BACKSPACE_CTRL_H 0x08
#define MSH_BACKSPACE_DEL 0x7f

static int msh_write(const void *buffer, uint32_t length) {
    if(length == 0) {
        return 1;
    }
    int32_t result = ioctl(MINEMU_FD_UART_OUT, MINEMU_IOCTL_UART_WRITE, (void *)buffer, length);
    return result == (int32_t)length;
}

static int msh_print(const char *text) {
    size_t length = 0;
    while(text[length] != '\0') {
        ++length;
    }
    return msh_write(text, (uint32_t)length);
}

static int msh_putc(char character) {
    return msh_write(&character, 1U);
}

static int msh_try_getc(uint8_t *character) {
    return ioctl(MINEMU_FD_UART_IN, MINEMU_IOCTL_UART_READ, (void *)character, 1U);
}

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

static int msh_read_line(char *line, int max) {
    int len = 0;
    uint32_t extra = 0;

    for (;;) {
        uint8_t c;
        int result = msh_try_getc(&c);
        if (result < 0) {
            return -1;
        }
        if (result == 0) {
            continue;
        }
        if (c == '\n') {
            line[len] = '\0';
            return extra != 0;
        } else if(c == MSH_BACKSPACE_CTRL_H || c == MSH_BACKSPACE_DEL) {
            if(extra != 0) {
                --extra;
            } else if(len > 0) {
                --len;
            }
        } else if(c == 0x0d) {
            // swallow CR from CRLF input; the LF that follows terminates
        } else if(len < max && extra == 0) {
            line[len++] = (char)c;
        } else if(extra != UINT32_MAX) {
            ++extra;
        }
    }
}

static int msh_exec(const char *line) {
    const char *p = line;
    while(*p == ' ') {
        ++p;
    }
    if(*p == '\0') {
        return 1;
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

        char output[MSH_MAX_LINE + 2];
        int output_len = 0;
        int pending_space = 0;
        while(*p != '\0') {
            if(*p == ' ') {
                pending_space = output_len != 0;
            } else {
                if(pending_space) {
                    output[output_len++] = ' ';
                    pending_space = 0;
                }
                output[output_len++] = *p;
            }
            ++p;
        }
        output[output_len++] = '\n';
        return msh_write(output, (uint32_t)output_len);
    }

    return msh_print("command not found: ") && msh_print(cmd) && msh_putc('\n');
}

static void msh_fail_stop(void) __attribute__((noreturn));

static void msh_fail_stop(void) {
    for (;;) {
        __asm__ volatile("nop");
    }
}

void minemu_msh_run(void) {
    char line[MSH_MAX_LINE + 1];
    for (;;) {
        if(!msh_print("msh> ")) {
            msh_fail_stop();
        }

        int result = msh_read_line(line, MSH_MAX_LINE);
        if(result < 0) {
            msh_fail_stop();
        }
        if(result == 0 && !msh_exec(line)) {
            msh_fail_stop();
        }
    }
}
