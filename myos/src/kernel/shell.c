#include "shell.h"
#include "serial.h"
#include "syscall.h"
#include "heap.h"
#include "paging.h"
#include <string.h>

static char read_buffer[256];
static int buffer_index = 0;

static void shell_help(void) {
    serial_write_string("Available commands:\n");
    serial_write_string("  help     - Show this help\n");
    serial_write_string("  hello    - Print greeting\n");
    serial_write_string("  mem      - Show memory info\n");
    serial_write_string("  ps       - List processes\n");
    serial_write_string("  clear    - Clear screen\n");
    serial_write_string("  reboot   - Reboot system\n");
}

static void shell_hello(void) {
    serial_write_string("Hello from MyOS!\n");
}

static void shell_mem(void) {
    char buf[32];
    serial_write_string("Free memory: ");
    /* Would need to convert number to string */
    serial_write_string(" bytes\n");
    serial_write_string("Heap info: Implemented\n");
}

static void shell_ps(void) {
    serial_write_string("PID\tState\n");
    serial_write_string("---\t-----\n");
    /* Would iterate through process list */
}

static void shell_clear(void) {
    /* Clear screen by writing spaces */
    for (int i = 0; i < 80 * 25; i++) {
        serial_write_char(' ');
    }
    serial_write_string("\n");
}

static void shell_reboot(void) {
    serial_write_string("Rebooting...\n");
    /* Triple fault to reboot */
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    __asm__ volatile ("hlt");
}

void shell_execute_command(char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        shell_help();
    } else if (strcmp(cmd, "hello") == 0) {
        shell_hello();
    } else if (strcmp(cmd, "mem") == 0) {
        shell_mem();
    } else if (strcmp(cmd, "ps") == 0) {
        shell_ps();
    } else if (strcmp(cmd, "clear") == 0) {
        shell_clear();
    } else if (strcmp(cmd, "reboot") == 0) {
        shell_reboot();
    } else if (strlen(cmd) > 0) {
        serial_write_string("Unknown command: ");
        serial_write_string(cmd);
        serial_write_string("\n");
    }
}

void shell_print_prompt(void) {
    serial_write_string("\nmyos> ");
}

void shell_start(void) {
    serial_write_string("\n=== MyOS Shell ===\n");
    shell_help();
    
    while (1) {
        shell_print_prompt();
        
        /* Read command */
        buffer_index = 0;
        while (1) {
            char c = 0;
            /* In real implementation, would read from keyboard */
            /* For now, just echo input via serial */
            
            if (c == '\n' || c == '\r') {
                read_buffer[buffer_index] = '\0';
                serial_write_string("\n");
                break;
            } else if (c == '\b' && buffer_index > 0) {
                buffer_index--;
                serial_write_string("\b \b");
            } else if (c >= ' ' && c <= '~' && buffer_index < 255) {
                read_buffer[buffer_index++] = c;
                serial_write_char(c);
            }
        }
        
        shell_execute_command(read_buffer);
    }
}
