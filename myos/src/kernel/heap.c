#include "heap.h"
#include "paging.h"
#include "serial.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct heap_block {
    size_t size;
    bool used;
    struct heap_block* next;
    struct heap_block* prev;
};

#define HEAP_START 0x1000000  /* 16MB */
#define HEAP_MIN_BLOCK 32
#define HEAP_ALIGNMENT 8

static struct heap_block* heap_head = NULL;
static uintptr_t heap_brk = HEAP_START;

static void expand_heap(size_t size) {
    size_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (size_t i = 0; i < pages_needed; i++) {
        void* page = vmm_alloc_page();
        if (!page) {
            serial_write_string("Heap expansion failed!\n");
            return;
        }
        vmm_map_page(page, (void*)heap_brk, PAGE_PRESENT | PAGE_WRITE);
        heap_brk += PAGE_SIZE;
    }
}

static struct heap_block* find_free_block(size_t size) {
    struct heap_block* current = heap_head;
    
    while (current) {
        if (!current->used && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static struct heap_block* create_block(void* addr, size_t size) {
    struct heap_block* block = (struct heap_block*)addr;
    block->size = size;
    block->used = false;
    block->next = NULL;
    block->prev = NULL;
    return block;
}

void heap_init(void) {
    /* Create initial block */
    heap_head = create_block((void*)HEAP_START, HEAP_MIN_BLOCK);
    expand_heap(HEAP_MIN_BLOCK + sizeof(struct heap_block));
    heap_head->used = false;
    
    serial_write_string("Heap initialized\n");
}

void* malloc(size_t size) {
    if (size == 0) return NULL;
    
    /* Align size */
    size = (size + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1);
    
    struct heap_block* block = find_free_block(size);
    
    if (!block) {
        /* Allocate new block */
        size_t total_size = size + sizeof(struct heap_block);
        expand_heap(total_size);
        
        block = create_block((void*)heap_brk - total_size, size);
        block->used = true;
        
        /* Add to list */
        block->next = heap_head;
        if (heap_head) heap_head->prev = block;
        heap_head = block;
        
        return (void*)(block + 1);
    }
    
    /* Split block if too large */
    if (block->size > size + sizeof(struct heap_block) + HEAP_MIN_BLOCK) {
        struct heap_block* new_block = (struct heap_block*)((uintptr_t)block + sizeof(struct heap_block) + size);
        new_block->size = block->size - size - sizeof(struct heap_block);
        new_block->used = false;
        new_block->next = block->next;
        new_block->prev = block;
        
        if (block->next) block->next->prev = new_block;
        block->next = new_block;
        block->size = size;
    }
    
    block->used = true;
    return (void*)(block + 1);
}

void free(void* ptr) {
    if (!ptr) return;
    
    struct heap_block* block = (struct heap_block*)ptr - 1;
    block->used = false;
    
    /* Coalesce with next block */
    if (block->next && !block->next->used) {
        block->size += sizeof(struct heap_block) + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }
    
    /* Coalesce with previous block */
    if (block->prev && !block->prev->used) {
        block->prev->size += sizeof(struct heap_block) + block->size;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
    }
}

void* calloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = malloc(total);
    if (ptr) {
        uint8_t* p = ptr;
        for (size_t i = 0; i < total; i++) {
            p[i] = 0;
        }
    }
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    struct heap_block* block = (struct heap_block*)ptr - 1;
    if (block->size >= size) return ptr;
    
    void* new_ptr = malloc(size);
    if (new_ptr) {
        uint8_t* src = ptr;
        uint8_t* dst = new_ptr;
        for (size_t i = 0; i < block->size; i++) {
            dst[i] = src[i];
        }
        free(ptr);
    }
    return new_ptr;
}
