#include "serial.h"

void serial_init(int port) {
    outb(port + 1, 0x00);
    outb(port + 3, 0x80);
    outb(port + 0, 0x03);
    outb(port + 1, 0x00);
    outb(port + 3, 0x03);
    outb(port + 2, 0xC7);
    outb(port + 4, 0x0B);
}

int serial_received(int port) {
    return inb(port + 5) & 1;
}

char serial_read_char(void) {
    while (serial_received(COM1) == 0);
    return inb(COM1);
}

int is_transmit_empty(int port) {
    return inb(port + 5) & 0x20;
}

void serial_write_char(char a) {
    while (is_transmit_empty(COM1) == 0);
    outb(COM1, a);
}

void serial_write_string(char* str) {
    while (*str) {
        serial_write_char(*str++);
    }
}
