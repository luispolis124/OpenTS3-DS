.global FUN_00004d06
.type FUN_00004d06, %function
FUN_00004d06:
    push {r4, r5, r6, r7, lr}
    sub sp, sp, #16
_loop_do:
    ldr r0, [r5, #0x2c]
_while_1:
    lsl r1, r0, #10
    str r1, [r2]
    str r2, [r2, #4]
    str r3, [r2, #8]
    str r4, [r2, #12]
    cmp r4, #0
    bge _break_while_1
    strb r2, [r5, #0xb]
    
    @ Mascara 0x3fffff em modo Thumb
    ldr r7, =0x3fffff
    mov r1, r0
    and r1, r7
    
    lsl r5, r1, #6
    lsl r0, r1, #9
_while_2:
    mov r1, r0
    lsl r6, r1, #14
    str r6, [r2]
    str r2, [r2, #4]
    str r3, [r2, #8]
    str r4, [r2, #12]
    mov r0, r4
    cmp r4, #0
    bge _break_while_1
    ldr r2, [r5, #0xc]
    
    @ Mascara 0x3ffff em modo Thumb
    ldr r7, =0x3ffff
    mov r6, r1
    and r6, r7
    
    lsl r5, r6, #10
    lsl r6, r1, #31
    str r6, [r2]
    str r2, [r2, #4]
    str r3, [r2, #8]
    str r4, [r2, #12]
    cmp r4, #0
    blt _halt_block
    b _while_2
_break_while_1:
    str r4, [r4]
    str r5, [r4, #4]
    str r6, [r4, #8]
    str r7, [r4, #12]
    mov r2, #0x68
    lsl r3, r5, #10
    str r2, [r6]
    ldr r1, [sp, #0]
    str r1, [r1]
    str r5, [r1, #4]
    str r6, [r1, #8]
    str r7, [r1, #12]
    mov r4, r1
    b _loop_do
_halt_block:
    mov r1, #0x68
    str r1, [r6]
    b _halt_block
    
