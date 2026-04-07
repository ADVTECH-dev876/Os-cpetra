ENTRY(_start)

SECTIONS
{
    . = 1M;

    .boot : {
        *(.multiboot)
        *(.boot)
        *(.boot.data)
    }

    .text : {
        *(.text)
        *(.text.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    /* Kernel end - used for heap start */
    kernel_end = .;
}
