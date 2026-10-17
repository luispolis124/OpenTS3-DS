@ src/arm9_crt0.s - STUB do crt0 do ARM9.
@
@ Isto NÃO é o crt0 original do jogo. É um substituto mínimo e legível que faz o que o
@ crt0 do SDK faz, na mesma ordem, para que o projeto linke e o layout possa ser testado:
@   1. modo SVC, interrupções desligadas
@   2. liga ITCM e DTCM (CP15)
@   3. copia os autoloads (ITCM/DTCM) da imagem para o destino
@   4. zera o .bss estático
@   5. pilha e salto para Arm9_Main
@
@ Quando decompilar o _start real (0x02004800), troque este arquivo pelo resultado.
@ NÃO TESTADO em hardware nem em emulador: escrito sem acesso a um assembler ARM.
@ Os valores de CP15 abaixo são os usuais de ARM946E-S/DS, mas confirme contra o
@ crt0 original antes de confiar neles.

    .section .secure, "a"
    @ Secure Area (2 KB) vem pronta da ROM: asm/bin/arm9_secure.bin (make extract).
    .incbin "asm/bin/arm9_secure.bin"

    .section .crt0, "ax"
    .arm
    .global _start
_start:
    @ 1. modo SVC (0x13), IRQ e FIQ desligados
    mov     r0, #0xD3
    msr     cpsr_c, r0

    @ 2. TCM. ITCM: base 0, 32 MB (cobre os espelhos até 0x01FFFFFF).
    @    DTCM: base 0x02FE0000 (TWL-SDK), 16 KB. Depois liga os dois no registrador de controle.
    mov     r0, #0x20
    mcr     p15, 0, r0, c9, c1, 1
    ldr     r0, =0x02FE000A
    mcr     p15, 0, r0, c9, c1, 0
    mrc     p15, 0, r0, c1, c0, 0
    orr     r0, r0, #0x50000            @ bit 16 = DTCM, bit 18 = ITCM
    mcr     p15, 0, r0, c1, c0, 0

    @ 3. Autoloads: lista de entradas {destino, tamanho, ?, bss} (16 bytes cada).
    @    A origem é a imagem carregada em RAM, logo após o bloco estático.
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

    @ 4. .bss estático (começa em cima da imagem de autoload: por isso só agora)
4:  ldr     r0, =__static_bss_start
    ldr     r1, =__static_bss_end
    mov     r2, #0
5:  cmp     r0, r1
    strlo   r2, [r0], #4
    blo     5b

    @ 5. Pilha (topo da DTCM, abaixo da área do vetor de IRQ) e salto para o C
    ldr     sp, =__sys_stack_top
    ldr     r0, =Arm9_Main
    bx      r0

    .pool
