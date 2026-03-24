/*
 * ARM64 startup code
 */

.section ".text.start"
.global _start

_start:
    // Set up stack pointer
    ldr x0, =_stack_top
    mov sp, x0
    
    // Clear BSS
    ldr x0, =__bss_start
    ldr x1, =__bss_end
    mov x2, #0
    
bss_clear_loop:
    cmp x0, x1
    b.ge bss_clear_done
    str x2, [x0], #8
    b bss_clear_loop
    
bss_clear_done:
    // Jump to main
    bl main
    
    // Should not return
    b .

// Exception vectors (minimal)
.section ".vectors"
.global _vectors

_vectors:
    // Current EL with SP0
    b .  // Synchronous
    b .  // IRQ
    b .  // FIQ
    b .  // SError
    
    // Current EL with SPx
    b .  // Synchronous
    b .  // IRQ
    b .  // FIQ
    b .  // SError
    
    // Lower EL using AArch64
    b .  // Synchronous
    b .  // IRQ
    b .  // FIQ
    b .  // SError
    
    // Lower EL using AArch32
    b .  // Synchronous
    b .  // IRQ
    b .  // FIQ
    b .  // SError

.section ".stack"
_stack_bottom:
    .space 4096
_stack_top: