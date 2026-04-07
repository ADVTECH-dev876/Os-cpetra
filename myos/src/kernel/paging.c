#include "paging.h"
#include "serial.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Physical memory bitmap (assuming 128MB max for simplicity) */
#define MEMORY_SIZE (128 * 1024 * 1024)
#define BLOCK_SIZE PAGE_SIZE
#define BLOCK_COUNT (MEMORY_SIZE / BLOCK_SIZE)

static uint8_t memory_bitmap[BLOCK_COUNT / 8];
static uint32_t used_blocks = 0;

/* Page tables - one level 4 page for demo (actual x86-64 uses 4 levels) */
struct page_entry {
    uint64_t value;
};

#define PAGE_DIRECTORY_ENTRIES 512

static struct page_entry page_table[PAGE_DIRECTORY_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static struct page_entry page_directory[PAGE_DIRECTORY_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

void pmm_init(void) {
    /* Initialize bitmap - all blocks free */
    for (int i = 0; i < BLOCK_COUNT / 8; i++) {
        memory_bitmap[i] = 0;
    }
    
    /* Mark kernel memory as used (first 16MB) */
    for (int i = 0; i < (16 * 1024 * 1024 / BLOCK_SIZE); i++) {
        pmm_alloc_block();
    }
    
    serial_write_string("Physical memory manager initialized\n");
}

void* pmm_alloc_block(void) {
    for (size_t i = 0; i < BLOCK_COUNT; i++) {
        size_t byte_index = i / 8;
        size_t bit_index = i % 8;
        
        if (!(memory_bitmap[byte_index] & (1 << bit_index))) {
            memory_bitmap[byte_index] |= (1 << bit_index);
            used_blocks++;
            return (void*)(i * BLOCK_SIZE);
        }
    }
    return NULL;  /* Out of memory */
}

void pmm_free_block(void* p) {
    uintptr_t addr = (uintptr_t)p;
    size_t block_index = addr / BLOCK_SIZE;
    
    if (block_index < BLOCK_COUNT) {
        size_t byte_index = block_index / 8;
        size_t bit_index = block_index % 8;
        memory_bitmap[byte_index] &= ~(1 << bit_index);
        used_blocks--;
    }
}

size_t pmm_get_free_memory(void) {
    return (BLOCK_COUNT - used_blocks) * BLOCK_SIZE;
}

void vmm_init(void) {
    /* Clear page directory and tables */
    for (int i = 0; i < PAGE_DIRECTORY_ENTRIES; i++) {
        page_directory[i].value = 0;
        page_table[i].value = 0;
    }
    
    /* Identity map first 16MB for kernel */
    for (uintptr_t addr = 0; addr < 16 * 1024 * 1024; addr += PAGE_SIZE) {
        vmm_map_page((void*)addr, (void*)addr, PAGE_PRESENT | PAGE_WRITE);
    }
    
    /* Load page directory into CR3 */
    __asm__ volatile ("mov %0, %%cr3" : : "r"(&page_directory));
    
    /* Enable paging (set PG bit in CR0) */
    uint64_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  /* Enable paging */
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
    
    serial_write_string("Virtual memory manager initialized\n");
}

void vmm_map_page(void* phys, void* virt, uint64_t flags) {
    uintptr_t virtual_addr = (uintptr_t)virt;
    uintptr_t page_dir_index = (virtual_addr >> 12) & 0x1FF;
    
    /* Map in page table (simplified - assumes single level for demo) */
    page_table[page_dir_index].value = (uintptr_t)phys | flags | PAGE_PRESENT;
    
    /* Set page directory entry */
    page_directory[page_dir_index].value = (uintptr_t)&page_table[page_dir_index] | PAGE_PRESENT | PAGE_WRITE;
    
    /* Invalidate TLB */
    __asm__ volatile ("invlpg (%0)" : : "r"(virt));
}

void* vmm_alloc_page(void) {
    void* phys = pmm_alloc_block();
    if (!phys) return NULL;
    
    /* Find free virtual address (simplified - just return phys for identity mapping) */
    return phys;
}

void vmm_free_page(void* addr) {
    pmm_free_block(addr);
}
