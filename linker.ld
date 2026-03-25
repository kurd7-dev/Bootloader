ENTRY(_start)

SECTIONS
{
    . = BOOT_BASE;

    .text : ALIGN(16) {
        KEEP(*(.text.boot))
        KEEP(*(.text.vectors))
        *(.text*)
    }

    .rodata : ALIGN(16) {
        *(.rodata*)
    }

    .kernel_blob : ALIGN(8) {
        __kernel_blob_start = .;
        KEEP(*(.kernel_blob))
        __kernel_blob_end = .;
    }

    .dtb_blob : ALIGN(8) {
        __dtb_blob_start = .;
        KEEP(*(.dtb_blob))
        __dtb_blob_end = .;
    }

    .data : ALIGN(16) {
        *(.data*)
    }

    .bss (NOLOAD) : ALIGN(16) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    .stack (NOLOAD) : ALIGN(16) {
        _stack_bottom = .;
        . += 0x8000;
        _stack_top = .;
    }

    /DISCARD/ : { *(.comment) *(.note*) }
}