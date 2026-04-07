#include "gdt.h"
#include "serial.h"

#define GDT_ENTRIES 5

struct gdt_entry gdt[GDT_ENTRIES];
struct gdt_ptr gp;

/* Access bits */
#define GDT_ACCESS_PRESENT    0x80
#define GDT_ACCESS_PRIV_0     0x00
#define GDT_ACCESS_PRIV_3     0x60
#define GDT_ACCESS_CODE       0x0A
#define GDT_ACCESS_DATA       0x02
#define GDT_ACCESS_TSS        0x09

/* Granularity bits */
#define GDT_GRAN_PAGE         0x80
#define GDT_GRAN_32BIT        0x40
#define GDT_GRAN_LONG         0x20

extern void gdt_flush(uint64_t);

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);
    
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_init(void) {
    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base = (uint64_t)&gdt;
    
    /* NULL descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);
    
    /* Kernel code segment */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 
                 GDT_ACCESS_PRESENT | GDT_ACCESS_PRIV_0 | GDT_ACCESS_CODE,
                 GDT_GRAN_PAGE | GDT_GRAN_32BIT | GDT_GRAN_LONG);
    
    /* Kernel data segment */
    gdt_set_gate(2, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_PRIV_0 | GDT_ACCESS_DATA,
                 GDT_GRAN_PAGE | GDT_GRAN_32BIT);
    
    /* User code segment (for syscalls) */
    gdt_set_gate(3, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_PRIV_3 | GDT_ACCESS_CODE,
                 GDT_GRAN_PAGE | GDT_GRAN_32BIT | GDT_GRAN_LONG);
    
    /* User data segment */
    gdt_set_gate(4, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_PRIV_3 | GDT_ACCESS_DATA,
                 GDT_GRAN_PAGE | GDT_GRAN_32BIT);
    
    /* Load GDT */
    gdt_flush((uint64_t)&gp);
    serial_write_string("GDT loaded\n");
}
