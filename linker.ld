/*
 * Linker script for ARM64 bootloader
 */

ENTRY(_start)

SECTIONS
{
    . = 0x80000000;
    
    .text : {
        *(.text.start)
        *(.text)
        *(.text.*)
    }
    
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    
    .data : {
        *(.data)
    }
    
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        __bss_end = .;
    }
    
    .stack : {
        _stack_bottom = .;
        . += 4096;
        _stack_top = .;
    }
    
    .vectors : {
        _vectors = .;
        *(.vectors)
    }
}