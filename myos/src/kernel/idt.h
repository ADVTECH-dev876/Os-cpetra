#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t base_mid;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_init(void);
void idt_set_gate(uint8_t num, uint64_t base, uint16_t selector, uint8_t flags);

/* ISR handlers */
void isr0(void);
void isr1(void);
void isr2(void);
void isr3(void);
void isr4(void);
void isr5(void);
void isr6(void);
void isr7(void);
void isr8(void);
void isr10(void);
void isr11(void);
void isr12(void);
void isr13(void);
void isr14(void);
void isr16(void);
void isr17(void);
void isr18(void);
void isr32(void);  /* Timer */
void isr33(void);  /* Keyboard */

#endif
