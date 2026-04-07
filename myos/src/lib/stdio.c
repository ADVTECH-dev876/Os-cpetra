#include "stdio.h"
#include "serial.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* VGA framebuffer - 80x25 text mode */
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((uint16_t*)0xB8000)

static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_color = 0x07;  /* Light grey on black */

/* Initialize VGA text mode */
void vga_init(void) {
    /* Clear screen */
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[y * VGA_WIDTH + x] = (current_color << 8) | ' ';
        }
    }
    
    cursor_x = 0;
    cursor_y = 0;
    
    /* Update hardware cursor */
    uint16_t cursor_pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(cursor_pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((cursor_pos >> 8) & 0xFF));
}

/* Write a character to VGA framebuffer at specified position */
void vga_write_char(char c, uint8_t color, int x, int y) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        VGA_MEMORY[y * VGA_WIDTH + x] = (color << 8) | (uint8_t)c;
    }
}

/* Scroll the VGA screen up by one line */
void vga_scroll(void) {
    /* Move all lines up by one */
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[(y - 1) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
        }
    }
    
    /* Clear last line */
    for (int x = 0; x < VGA_WIDTH; x++) {
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (current_color << 8) | ' ';
    }
}

/* Set VGA text color */
void print_set_color(uint8_t foreground, uint8_t background) {
    current_color = (background << 4) | (foreground & 0x0F);
}

/* Write a character to both VGA and serial */
void putchar(char c) {
    /* Send to serial for debugging */
    serial_write_char(c);
    
    /* Handle special characters */
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_write_char(' ', current_color, cursor_x, cursor_y);
        }
    } else if (c == '\t') {
        /* Tab to next 4-column boundary */
        do {
            putchar(' ');
        } while (cursor_x % 4 != 0);
        return;
    } else {
        /* Normal character */
        vga_write_char(c, current_color, cursor_x, cursor_y);
        cursor_x++;
    }
    
    /* Handle line wrapping and scrolling */
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= VGA_HEIGHT) {
        vga_scroll();
        cursor_y = VGA_HEIGHT - 1;
    }
    
    /* Update hardware cursor */
    uint16_t cursor_pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(cursor_pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((cursor_pos >> 8) & 0xFF));
}

/* Read a character from keyboard (simplified) */
char getchar(void) {
    /* This would normally read from the keyboard buffer */
    /* For now, just return a placeholder */
    /* In a real implementation, you'd have a keyboard driver 
       that fills a circular buffer and this function would 
       wait on that buffer */
    
    /* Poll keyboard controller (PS/2) */
    uint8_t scancode;
    do {
        /* Wait for keyboard data */
        while ((inb(0x64) & 0x01) == 0);
        scancode = inb(0x60);
    } while (scancode & 0x80);  /* Skip key releases */
    
    /* Simple scancode to ASCII conversion (US keyboard, lowercase only) */
    static const char scancode_to_ascii[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    
    if (scancode < sizeof(scancode_to_ascii)) {
        char c = scancode_to_ascii[scancode];
        if (c) {
            /* Echo the character */
            putchar(c);
            return c;
        }
    }
    
    return 0;
}

/* Print a string */
void puts(const char* str) {
    while (*str) {
        putchar(*str++);
    }
}

/* Print an integer (base 10) */
void print_int(int num) {
    if (num < 0) {
        putchar('-');
        num = -num;
    }
    
    if (num == 0) {
        putchar('0');
        return;
    }
    
    /* Convert to string in reverse */
    char buffer[12];
    int i = 0;
    
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    /* Print in correct order */
    while (i > 0) {
        putchar(buffer[--i]);
    }
}

/* Print unsigned integer */
void print_uint(unsigned int num) {
    if (num == 0) {
        putchar('0');
        return;
    }
    
    char buffer[12];
    int i = 0;
    
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    while (i > 0) {
        putchar(buffer[--i]);
    }
}

/* Print hex integer (32-bit) */
void print_hex(uint32_t num) {
    static const char hex_digits[] = "0123456789ABCDEF";
    
    /* Print '0x' prefix */
    putchar('0');
    putchar('x');
    
    /* Print 8 hex digits */
    for (int i = 28; i >= 0; i -= 4) {
        putchar(hex_digits[(num >> i) & 0xF]);
    }
}

/* Print hex integer (64-bit) */
void print_hex64(uint64_t num) {
    static const char hex_digits[] = "0123456789ABCDEF";
    
    putchar('0');
    putchar('x');
    
    for (int i = 60; i >= 0; i -= 4) {
        putchar(hex_digits[(num >> i) & 0xF]);
    }
}

/* Formatted string output to buffer (like vsnprintf) */
int vsnprintf(char* buffer, size_t size, const char* format, va_list args) {
    if (size == 0) return 0;
    
    char* ptr = buffer;
    size_t remaining = size - 1;
    
    while (*format && remaining > 0) {
        if (*format == '%') {
            format++;
            
            /* Handle format specifiers */
            switch (*format) {
                case 'c': {
                    char c = (char)va_arg(args, int);
                    *ptr++ = c;
                    remaining--;
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    while (*str && remaining > 0) {
                        *ptr++ = *str++;
                        remaining--;
                    }
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    /* Convert integer to string */
                    char num_buf[12];
                    int i = 0;
                    
                    if (num < 0) {
                        *ptr++ = '-';
                        remaining--;
                        num = -num;
                    }
                    
                    /* Handle zero */
                    if (num == 0) {
                        *ptr++ = '0';
                        remaining--;
                        break;
                    }
                    
                    while (num > 0 && remaining > 0) {
                        num_buf[i++] = '0' + (num % 10);
                        num /= 10;
                    }
                    
                    while (i > 0 && remaining > 0) {
                        *ptr++ = num_buf[--i];
                        remaining--;
                    }
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    char num_buf[12];
                    int i = 0;
                    
                    if (num == 0) {
                        *ptr++ = '0';
                        remaining--;
                        break;
                    }
                    
                    while (num > 0 && remaining > 0) {
                        num_buf[i++] = '0' + (num % 10);
                        num /= 10;
                    }
                    
                    while (i > 0 && remaining > 0) {
                        *ptr++ = num_buf[--i];
                        remaining--;
                    }
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    static const char hex_digits[] = "0123456789abcdef";
                    
                    /* Check if we have room for "0x" + 8 digits */
                    if (remaining >= 10) {
                        *ptr++ = '0';
                        *ptr++ = 'x';
                        remaining -= 2;
                        
                        for (int i = 28; i >= 0 && remaining > 0; i -= 4) {
                            *ptr++ = hex_digits[(num >> i) & 0xF];
                            remaining--;
                        }
                    }
                    break;
                }
                case 'X': {
                    unsigned int num = va_arg(args, unsigned int);
                    static const char hex_digits[] = "0123456789ABCDEF";
                    
                    if (remaining >= 10) {
                        *ptr++ = '0';
                        *ptr++ = 'x';
                        remaining -= 2;
                        
                        for (int i = 28; i >= 0 && remaining > 0; i -= 4) {
                            *ptr++ = hex_digits[(num >> i) & 0xF];
                            remaining--;
                        }
                    }
                    break;
                }
                case 'p': {
                    uintptr_t num = (uintptr_t)va_arg(args, void*);
                    static const char hex_digits[] = "0123456789abcdef";
                    
                    if (remaining >= 18) {  /* "0x" + 16 digits */
                        *ptr++ = '0';
                        *ptr++ = 'x';
                        remaining -= 2;
                        
                        for (int i = 60; i >= 0 && remaining > 0; i -= 4) {
                            *ptr++ = hex_digits[(num >> i) & 0xF];
                            remaining--;
                        }
                    }
                    break;
                }
                case '%': {
                    *ptr++ = '%';
                    remaining--;
                    break;
                }
                default:
                    *ptr++ = '%';
                    remaining--;
                    if (remaining > 0) {
                        *ptr++ = *format;
                        remaining--;
                    }
                    break;
            }
            format++;
        } else {
            *ptr++ = *format++;
            remaining--;
        }
    }
    
    *ptr = '\0';
    return (size - remaining - 1);
}

/* snprintf implementation */
int snprintf(char* buffer, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}

/* printf implementation using VGA and serial */
void printf(const char* format, ...) {
    char buffer[1024];
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    /* Output to both VGA and serial */
    puts(buffer);
}

/* Clear the screen */
void print_clear(void) {
    vga_init();
    cursor_x = 0;
    cursor_y = 0;
}
