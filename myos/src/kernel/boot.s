.set MULTIBOOT2_MAGIC, 0xe85250d6
.set MULTIBOOT2_ARCH, 0
.set MULTIBOOT2_HEADER_LEN, (multiboot2_header_end - multiboot2_header_start)
.set MULTIBOOT2_CHECKSUM, -(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCH + MULTIBOOT2_HEADER_LEN)

.section .multiboot
multiboot2_header_start:
    .long MULTIBOOT2_MAGIC
    .long MULTIBOOT2_ARCH
    .long MULTIBOOT2_HEADER_LEN
    .long MULTIBOOT2_CHECKSUM

    /* Information request tag */
    .word 1
    .word 0
    .long 24
    .long 6   /* memory map */
    .long 8   /* framebuffer */
    
    /* End tag */
    .word 0
    .word 0
    .long 8
multiboot2_header_end:

.section .boot
.code32
.global _start
.type _start, @function
_start:
    /* Set up stack */
    mov $stack_top, %esp
    
    /* Save multiboot info pointer */
    push %ebx
    push %eax
    
    /* Call kernel main */
    call kernel_early_init
    
    /* Never reached */
    cli
1:  hlt
    jmp 1b

.section .bss
.space 16384
stack_top:
