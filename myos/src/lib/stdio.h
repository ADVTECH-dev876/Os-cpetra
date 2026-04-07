#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>
#include <stdarg.h>

/* Color codes for VGA text mode */
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN   14
#define VGA_COLOR_WHITE         15

/* Console I/O */
void putchar(char c);
char getchar(void);
void puts(const char* str);
void printf(const char* format, ...);
void print_clear(void);
void print_set_color(uint8_t foreground, uint8_t background);

/* String formatting */
int vsnprintf(char* buffer, size_t size, const char* format, va_list args);
int snprintf(char* buffer, size_t size, const char* format, ...);

/* VGA framebuffer functions */
void vga_init(void);
void vga_write_char(char c, uint8_t color, int x, int y);
void vga_scroll(void);

/* Formatted output helpers */
void print_int(int num);
void print_hex(uint32_t num);
void print_hex64(uint64_t num);
void print_uint(unsigned int num);

#endif /* STDIO_H */
