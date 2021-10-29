    .global blCallout
blCallout:
    push    {sp,lr}
    stmdb   sp!, {r0-r12}
    mrs     r0, cpsr /* r0 is scratch */
    push    {r0}
    mov     r0, sp

    bl handleBlCallout

    pop     {r0}
    msr     cpsr, r0
    ldmia   sp!, {r0-r12}
    add     sp, sp, #4
    pop     {pc}


    .global bCallout
bCallout:
    push    {sp,lr}
    stmdb   sp!, {r0-r12}
    mrs     r0, cpsr /* r0 is scratch */
    push    {r0}
    mov     r0, sp

    bl handleBCallout

    pop     {r0}
    msr     cpsr, r0
    ldmia   sp!, {r0-r12}
    add     sp, sp, #4
    pop     {pc}


    .global bxCallout
bxCallout:
    push    {sp,lr,pc}  /* sp, pc not needed, just for data structure compliance */
    push    {r0-r12}

    mrs     r0, cpsr    /* r0 is scratch, temporarily storing cspr */
    push    {r0}
    mov     r0, sp      /* setting the first arg (*SaveRegs) */

    bl handleBxCallout

    pop     {r0}
    msr     cpsr, r0
    
    pop     {r0-r12}
    add     sp, sp, #4  /* removing $sp from stack */
    pop     {lr}
    add     sp, sp, #4  /* removing $pc from stack */
    bx      lr


    .global bccCallout
bccCallout:
    push    {sp,lr}
    stmdb   sp!, {r0-r12}
    mrs     r0, cpsr
    push    {r0}
    mov     r0, sp

    bl handleBccCallout

    pop     {r0}
    msr     cpsr, r0
    ldmia   sp!, {r0-r12}
    add     sp, sp, #4
    pop     {pc}

@
@ For part3-5.
@
    .global condCallout
condCallout:
    /* IMPLEMENT */

@
@ For an extra credit assignment
@

    .global popCallout
popCallout:
    /* IMPLEMENT */
