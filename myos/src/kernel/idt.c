#include "idt.h"
#include "serial.h"
#include "scheduler.h"

#define IDT_ENTRIES 256

struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

/* Exception handler */
void fault_handler(struct regs* r) {
    serial_write_string("Exception! ");
    char buf[16];
    /* Print exception number */
    serial_write_string("\n");
    
    while(1);  /* Halt */
}

/* IRQ handler */
void irq_handler(struct regs* r) {
    if (r->int_no >= 32 && r->int_no <= 47) {
        /* Hardware IRQ */
        if (r->int_no == 32) {
            /* Timer interrupt - call scheduler */
            scheduler_tick();
        }
        else if (r->int_no == 33) {
            /* Keyboard interrupt */
            /* Handle keyboard input */
        }
        
        /* Send EOI to PIC */
        if (r->int_no >= 40) {
            outb(0xA0, 0x20);
        }
        outb(0x20, 0x20);
    }
}

void idt_set_gate(uint8_t num, uint64_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_mid = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].selector = selector;
    idt[num].ist = 0;
    idt[num].flags = flags;
    idt[num].reserved = 0;
}

void idt_init(void) {
    idtp.limit = sizeof(struct idt_entry) * IDT_ENTRIES - 1;
    idtp.base = (uint64_t)&idt;
    
    /* Clear IDT */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    /* Set exception handlers */
    idt_set_gate(0, (uint64_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint64_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint64_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint64_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint64_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint64_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint64_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint64_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint64_t)isr8, 0x08, 0x8E);
    idt_set_gate(10, (uint64_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint64_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint64_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint64_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint64_t)isr14, 0x08, 0x8E);
    idt_set_gate(16, (uint64_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint64_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint64_t)isr18, 0x08, 0x8E);
    
    /* Set IRQ handlers */
    idt_set_gate(32, (uint64_t)isr32, 0x08, 0x8E);
    idt_set_gate(33, (uint64_t)isr33, 0x08, 0x8E);
    
    /* Load IDT */
    __asm__ volatile ("lidt %0" : : "m"(idtp));
    
    /* Remap PIC */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x00);
    outb(0xA1, 0x00);
    
    serial_write_string("IDT initialized\n");
}
