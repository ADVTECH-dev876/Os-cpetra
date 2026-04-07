#include <stdint.h>
#include <stddef.h>
#include "gdt.h"
#include "idt.h"
#include "paging.h"
#include "heap.h"
#include "scheduler.h"
#include "shell.h"
#include "serial.h"

/* Multiboot structures */
struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_info {
    uint32_t total_size;
    uint32_t reserved;
    struct multiboot_tag tags[];
};

/* Kernel entry point */
void kernel_main(uint32_t magic, struct multiboot_info* info) {
    /* Initialize serial for debugging */
    serial_init(COM1);
    serial_write_string("Kernel starting...\n");
    
    /* 1. Initialize Global Descriptor Table */
    gdt_init();
    serial_write_string("GDT initialized\n");
    
    /* 2. Initialize Interrupt Descriptor Table */
    idt_init();
    serial_write_string("IDT initialized\n");
    
    /* 3. Initialize physical memory manager */
    pmm_init();
    serial_write_string("Physical memory manager initialized\n");
    
    /* 4. Initialize virtual memory (paging) */
    vmm_init();
    serial_write_string("Virtual memory manager initialized\n");
    
    /* 5. Initialize kernel heap */
    heap_init();
    serial_write_string("Heap initialized\n");
    
    /* 6. Initialize process scheduler */
    scheduler_init();
    serial_write_string("Scheduler initialized\n");
    
    /* 7. Initialize syscalls */
    syscall_init();
    serial_write_string("Syscalls initialized\n");
    
    /* 8. Enable interrupts */
    __asm__ volatile ("sti");
    
    /* 9. Start shell */
    serial_write_string("Starting shell...\n");
    shell_start();
    
    /* Should never return */
    while(1);
}

/* Early initialization (called from boot.S) */
void kernel_early_init(uint32_t magic, struct multiboot_info* info) {
    /* Parse memory map from multiboot */
    uint8_t* tag_ptr = (uint8_t*)info->tags;
    
    while (1) {
        struct multiboot_tag* tag = (struct multiboot_tag*)tag_ptr;
        
        if (tag->type == 0) {  /* END tag */
            break;
        }
        
        if (tag->type == 6) {  /* MMAP tag */
            /* Store memory map for PMM */
            struct multiboot_tag_mmap* mmap = (struct multiboot_tag_mmap*)tag;
            /* Process memory regions */
        }
        
        tag_ptr += (tag->size + 7) & ~7;
    }
    
    /* Jump to kernel_main */
    kernel_main(magic, info);
}
