.section ".text.boot"
.align 7
.global _start

_start:
    /* Save incoming DTB pointer from x0 */
    mov x19, x0

    /* Setup stack */
    ldr x0, =_stack_top
    mov sp, x0

    /* Clear .bss */
    ldr x0, =__bss_start
    ldr x1, =__bss_end
    mov x2, xzr
1:
    cmp x0, x1
    b.hs 2f
    str x2, [x0], #8
    b 1b
2:
    /* Restore and store incoming dtb ptr */
    ldr x0, =g_incoming_dtb_ptr
    str x19, [x0]

    /* Set VBAR_EL1 */
    ldr x0, =vector_table
    msr VBAR_EL1, x0
    isb

    bl bootloader_main

hang:
    wfe
    b hang

.section ".text.vectors"
.align 11
.global vector_table
vector_table:
    b vector_panic  /* Current EL SP0: Sync */
    b vector_panic  /* Current EL SP0: IRQ  */
    b vector_panic  /* Current EL SP0: FIQ  */
    b vector_panic  /* Current EL SP0: SError */

    b vector_panic  /* Current EL SPx: Sync */
    b vector_panic  /* Current EL SPx: IRQ  */
    b vector_panic  /* Current EL SPx: FIQ  */
    b vector_panic  /* Current EL SPx: SError */

    b vector_panic  /* Lower EL AArch64: Sync */
    b vector_panic  /* Lower EL AArch64: IRQ  */
    b vector_panic  /* Lower EL AArch64: FIQ  */
    b vector_panic  /* Lower EL AArch64: SError */

    b vector_panic  /* Lower EL AArch32: Sync */
    b vector_panic  /* Lower EL AArch32: IRQ  */
    b vector_panic  /* Lower EL AArch32: FIQ  */
    b vector_panic  /* Lower EL AArch32: SError */

vector_panic:
    bl boot_exception_panic
    b vector_panic
