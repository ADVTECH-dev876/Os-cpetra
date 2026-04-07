/* multiboot2.h - Multiboot 2 header. */
#ifndef MULTIBOOT_HEADER
#define MULTIBOOT_HEADER 1

#define MULTIBOOT_HEADER_MAGIC 0xe85250d6
#define MULTIBOOT_BOOTLOADER_MAGIC 0x36d76289

#define MULTIBOOT_TAG_TYPE_END 0
#define MULTIBOOT_TAG_TYPE_MMAP 6
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER 8
#define MULTIBOOT_TAG_TYPE_ACPI_OLD 14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW 15

#ifndef ASM_FILE

#include <stdint.h>

struct multiboot_header {
    uint32_t magic;
    uint32_t architecture;
    uint32_t header_length;
    uint32_t checksum;
};

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_mmap_entry {
        uint64_t addr;
        uint64_t len;
        uint32_t type;
        uint32_t reserved;
    } entries[];
};

#endif /* !ASM_FILE */
#endif /* !MULTIBOOT_HEADER */
