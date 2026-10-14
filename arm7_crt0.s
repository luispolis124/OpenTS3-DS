@ src/arm7_crt0.s - STUB do crt0 do ARM7.
@
@ NÃO é o crt0 original do jogo (0x02380000). Faz o mínimo, na ordem do SDK:
@   1. modo SVC, interrupções desligadas
@   2. copia os autoloads (WRAM do ARM7 e topo da RAM principal) da imagem para o destino
@   3. zera o .bss de cada autoload
@   4. pilha e salto para Arm7_Main
@
@ O ARM7 não tem CP15 (ARM7TDMI): nada de TCM/cache aqui.
@ NÃO TESTADO em hardware nem em emulador: escrito sem acesso a um assembler ARM.

    .section .crt0, "ax"
    .arm
    .global _start
_start:
    @ 1. modo SVC (0x13), IRQ e FIQ desligados
    mov     r0, #0xD3
    msr     cpsr_c, r0

    @ 2/3. Autoloads: entradas {destino, tamanho, ?, bss} (16 bytes cada)
    ldr     r4, =__autoload_start
    ldr     r5, =__autoload_list_start
    ldr     r6, =__autoload_list_end
1:  cmp     r5, r6
    bhs     4f
    ldmia   r5!, {r0, r1, r2, r3}       @ r0=destino r1=tamanho r2=(?) r3=bss
    add     r7, r4, r1                  @ fim da origem
2:  cmp     r4, r7
    ldrlo   r8, [r4], #4
    strlo   r8, [r0], #4
    blo     2b
    mov     r8, #0
    add     r7, r0, r3                  @ fim do .bss deste autoload
3:  cmp     r0, r7
    strlo   r8, [r0], #4
    blo     3b
    b       1b

    @ 4. Pilha e salto para o C
4:  ldr     sp, =__sys_stack_top
    ldr     r0, =Arm7_Main
    bx      r0

    .pool
