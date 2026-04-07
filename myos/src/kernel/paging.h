#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define PAGE_PRESENT 0x1
#define PAGE_WRITE 0x2
#define PAGE_USER 0x4

/* Physical Memory Manager */
void pmm_init(void);
void* pmm_alloc_block(void);
void pmm_free_block(void* p);
size_t pmm_get_free_memory(void);

/* Virtual Memory Manager */
void vmm_init(void);
void* vmm_alloc_page(void);
void vmm_free_page(void* addr);
void vmm_map_page(void* phys, void* virt, uint64_t flags);

#endif
