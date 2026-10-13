/*
 * linkscript.ld - script de link do ARM9 e do ARM7 (The Sims 3 DS, VS3E).
 *
 * Um arquivo só, preprocessado com cpp antes do link (ver Makefile):
 *     cpp -P -x c -undef -DARM9 linkscript.ld > build/arm9.ld
 *     cpp -P -x c -undef -DARM7 linkscript.ld > build/arm7.ld
 * Opcional: -DSTRICT_LAYOUT liga as ASSERTs que exigem tamanhos IDÊNTICOS aos do
 * binário original (use quando estiver fazendo matching; no desenvolvimento do dia a
 * dia deixe desligado).
 *
 * Todos os endereços vêm de asm/memory_layout.txt (gerado por tools/extract_code.py
 * a partir da ROM) e de include/memory_map.h.
 *
 * COMO O BINÁRIO DO SDK É MONTADO (confirmado na ROM)
 *   O arm9.bin/arm7.bin da ROM é uma imagem carregada de uma vez em RAM:
 *       [estático][dados do autoload 0][dados do autoload 1][lista de autoload]
 *   O crt0 copia cada autoload para o seu destino (ITCM, DTCM...) e depois zera o
 *   .bss. O .bss estático COMEÇA em __autoload_start, ou seja, SOBREPÕE a área onde
 *   os autoloads foram carregados; por isso a cópia tem que vir antes de zerar.
 *   Aqui isso é expresso com VMA (onde roda) != LMA (onde fica na imagem): as seções
 *   .itcm/.dtcm têm VMA na TCM e LMA (AT) logo após o bloco estático. "objcopy -O
 *   binary" gera a imagem em ordem de LMA, que é exatamente o arm9.bin da ROM.
 *
 * ModuleParams (9 palavras, formato do SDK) é gerado aqui com LONG(), nos mesmos
 * endereços da ROM (ARM9 0x02004FB0, ARM7 0x023805D4).
 *
 * LIMITES CONHECIDOS
 *   - Overlays não são tratados aqui (cada um precisa do seu script; endereços em
 *     memory_map.h).
 *   - O 3o campo de cada entrada da lista de autoload é desconhecido. No ARM9 original
 *     vale o próprio endereço; no ARM7 varia. Aqui sempre emitimos o endereço.
 *   - Seções de C++ (.init_array/.ctors) não estão previstas; o jogo parece usar
 *     inicializadores estáticos (veja entity.h), então isso entra quando for preciso.
 */

#if !defined(ARM9) && !defined(ARM7)
#error "Defina -DARM9 ou -DARM7 ao preprocessar o linkscript.ld"
#endif

ENTRY(_start)

/* ======================================================================== */
#if defined(ARM9)
/* ======================================================================== */

MEMORY
{
    SECURE (rx)  : ORIGIN = 0x02004000, LENGTH = 0x00000800
    /* estático + .bss: vai até o início do overlay 0 (0x022503A0) */
    MAIN   (rwx) : ORIGIN = 0x02004800, LENGTH = 0x0024BBA0
    ITCM   (rwx) : ORIGIN = 0x01FF8000, LENGTH = 0x00008000
    /* DTCM na base do TWL-SDK; as pontas (vetor de IRQ em +0x3FFC) ficam reservadas */
    DTCM   (rw)  : ORIGIN = 0x02FE0000, LENGTH = 0x00004000
}

SECTIONS
{
    /* Secure Area: 2 KB. Vem pronta da ROM (src/arm9_crt0.s faz .incbin). */
    .secure 0x02004000 : { KEEP(*(.secure)) } > SECURE

    /* crt0 (_start) logo após a Secure Area. */
    .crt0 0x02004800 : { KEEP(*(.crt0)) } > MAIN

    /* ModuleParams: endereço fixo da ROM original. */
    .module_params 0x02004FB0 : {
        __module_params = .;
        LONG(__autoload_list_start)   /* autoload_list_start */
        LONG(__autoload_list_end)     /* autoload_list_end   */
        LONG(__autoload_start)        /* autoload_start      */
        LONG(__static_bss_start)      /* static_bss_start    */
        LONG(__static_bss_end)        /* static_bss_end      */
        LONG(0)                       /* compressed_end (0 = não comprimido) */
        LONG(0x05047531)              /* sdk_version, igual ao da ROM        */
        LONG(0xDEC00621)              /* marca NITRO, parte 1 */
        LONG(0x2106C0DE)              /* marca NITRO, parte 2 */
    } > MAIN

    .text : ALIGN(4) {
        *(.text .text.*)
        *(.glue_7 .glue_7t .vfp11_veneer .v4_bx)
        . = ALIGN(4);
    } > MAIN

    .rodata : ALIGN(4) {
        *(.rodata .rodata.* .rodata1)
        . = ALIGN(4);
    } > MAIN

    .data : ALIGN(4) {
        *(.data .data.*)
        . = ALIGN(4);
    } > MAIN

    /* Fim do bloco estático = onde a imagem de autoload começa na ROM/RAM. */
    . = ALIGN(4);
    __static_end     = .;
    __autoload_start = .;

    /* Autoload 0 -> ITCM */
    .itcm ORIGIN(ITCM) : AT(__autoload_start) ALIGN(4) {
        *(.itcm .itcm.*)
        . = ALIGN(4);
    } > ITCM
    .itcm_bss (NOLOAD) : ALIGN(4) {
        *(.itcm_bss .itcm_bss.*)
        . = ALIGN(4);
    } > ITCM

    /* Autoload 1 -> DTCM */
    .dtcm ORIGIN(DTCM) : AT(__autoload_start + SIZEOF(.itcm)) ALIGN(4) {
        *(.dtcm .dtcm.*)
        . = ALIGN(4);
    } > DTCM
    .dtcm_bss (NOLOAD) : ALIGN(4) {
        *(.dtcm_bss .dtcm_bss.*)
        . = ALIGN(4);
    } > DTCM
    __dtcm_end = .;

    /* Lista de autoload: 16 bytes por entrada = {endereço, tamanho, ?, bss}. */
    __autoload_list_start = __autoload_start + SIZEOF(.itcm) + SIZEOF(.dtcm);
    .autoload_list __autoload_list_start : {
        LONG(ADDR(.itcm)); LONG(SIZEOF(.itcm)); LONG(ADDR(.itcm)); LONG(SIZEOF(.itcm_bss));
        LONG(ADDR(.dtcm)); LONG(SIZEOF(.dtcm)); LONG(ADDR(.dtcm)); LONG(SIZEOF(.dtcm_bss));
        __autoload_list_end = .;
    } > MAIN

    /* .bss estático: começa no mesmo endereço do autoload (como no original). */
    .bss __autoload_start (NOLOAD) : ALIGN(4) {
        __static_bss_start = .;
        *(.bss .bss.* COMMON)
        . = ALIGN(4);
        __static_bss_end = .;
    } > MAIN

    /* Pilha do modo SYS/SVC no topo da DTCM, abaixo da área do vetor de IRQ. */
    __sys_stack_top = ORIGIN(DTCM) + LENGTH(DTCM) - 0x400;

    ASSERT(__static_bss_end <= 0x022503A0, "ARM9: .bss estatico invade o overlay 0")
    ASSERT(__dtcm_end <= __sys_stack_top - 0x800, "ARM9: DTCM sem espaco para a pilha (2 KB)")

#if defined(STRICT_LAYOUT)
    ASSERT(__autoload_start       == 0x0217CAE0, "ARM9: fim do bloco estatico difere do original")
    ASSERT(SIZEOF(.itcm)          == 0x00003AC0, "ARM9: tamanho do autoload ITCM difere do original")
    ASSERT(SIZEOF(.itcm_bss)      == 0x00000020, "ARM9: .bss da ITCM difere do original")
    ASSERT(SIZEOF(.dtcm)          == 0x00000100, "ARM9: tamanho do autoload DTCM difere do original")
    ASSERT(SIZEOF(.dtcm_bss)      == 0x00000E00, "ARM9: .bss da DTCM difere do original")
    ASSERT(__autoload_list_start  == 0x021806A0, "ARM9: lista de autoload fora do lugar original")
    ASSERT(__static_bss_end       == 0x022503A0, "ARM9: fim do .bss estatico difere do original")
#endif
}

/* ======================================================================== */
#elif defined(ARM7)
/* ======================================================================== */

MEMORY
{
    /* RAM principal (DS: 4 MB). O ARM7 original carrega em 0x02380000. */
    MAIN   (rwx) : ORIGIN = 0x02380000, LENGTH = 0x00080000
    /* 0x037F8000..0x03810000: 32 KB compartilhados (espelho) + 64 KB do ARM7 */
    WRAM   (rwx) : ORIGIN = 0x037F8000, LENGTH = 0x00018000
    /* topo da RAM principal; no DS comum espelha 0x023E0000 */
    HIGH   (rwx) : ORIGIN = 0x02FE0000, LENGTH = 0x00020000
}

SECTIONS
{
    .crt0 0x02380000 : { KEEP(*(.crt0)) } > MAIN

    .module_params 0x023805D4 : {
        __module_params = .;
        LONG(__autoload_list_start)
        LONG(__autoload_list_end)
        LONG(__autoload_start)
        LONG(__static_bss_start)
        LONG(__static_bss_end)
        LONG(0)
        LONG(0x05047531)
        LONG(0xDEC00621)
        LONG(0x2106C0DE)
    } > MAIN

    /* No ARM7 original quase todo o código está nos autoloads. Enquanto não se sabe
     * qual objeto vai para onde, o .text/.data de C ficam no bloco estático. Para
     * mandar um módulo para a WRAM, use o atributo de seção (.wram) ou liste o objeto
     * em .wram abaixo, p.ex.:   *src/arm7/sound*.o(.text .text.*)  */
    .text : ALIGN(4) {
        *(.text .text.*)
        *(.glue_7 .glue_7t .vfp11_veneer .v4_bx)
        . = ALIGN(4);
    } > MAIN

    .rodata : ALIGN(4) {
        *(.rodata .rodata.* .rodata1)
        . = ALIGN(4);
    } > MAIN

    .data : ALIGN(4) {
        *(.data .data.*)
        . = ALIGN(4);
    } > MAIN

    . = ALIGN(4);
    __static_end     = .;
    __autoload_start = .;

    /* Autoload 0 -> WRAM do ARM7 */
    .wram ORIGIN(WRAM) : AT(__autoload_start) ALIGN(4) {
        *(.wram .wram.*)
        . = ALIGN(4);
    } > WRAM
    .wram_bss (NOLOAD) : ALIGN(4) {
        *(.wram_bss .wram_bss.*)
        /* O ARM7 original não tem .bss estático: globais sem inicializador vão para a WRAM. */
        *(.bss .bss.* COMMON)
        . = ALIGN(4);
    } > WRAM
    __wram_end = .;

    /* Autoload 1 -> topo da RAM principal */
    .high ORIGIN(HIGH) : AT(__autoload_start + SIZEOF(.wram)) ALIGN(4) {
        *(.high .high.*)
        . = ALIGN(4);
    } > HIGH
    .high_bss (NOLOAD) : ALIGN(4) {
        *(.high_bss .high_bss.*)
        . = ALIGN(4);
    } > HIGH

    __autoload_list_start = __autoload_start + SIZEOF(.wram) + SIZEOF(.high);
    .autoload_list __autoload_list_start : {
        LONG(ADDR(.wram)); LONG(SIZEOF(.wram)); LONG(ADDR(.wram)); LONG(SIZEOF(.wram_bss));
        LONG(ADDR(.high)); LONG(SIZEOF(.high)); LONG(ADDR(.high)); LONG(SIZEOF(.high_bss));
        __autoload_list_end = .;
    } > MAIN

    /* ARM7 original: static_bss_start == static_bss_end == autoload_start (vazio). */
    __static_bss_start = __autoload_start;
    __static_bss_end   = __autoload_start;

    /* Pilha no topo da WRAM, abaixo do vetor de IRQ (0x0380FFFC). */
    __sys_stack_top = 0x0380FF00;

    ASSERT(__wram_end <= __sys_stack_top - 0x400, "ARM7: WRAM sem espaco para a pilha (1 KB)")

#if defined(STRICT_LAYOUT)
    ASSERT(__autoload_start       == 0x02380644, "ARM7: fim do bloco estatico difere do original")
    ASSERT(SIZEOF(.wram)          == 0x0001152C, "ARM7: tamanho do autoload WRAM difere do original")
    ASSERT(SIZEOF(.wram_bss)      == 0x00002954, "ARM7: .bss da WRAM difere do original")
    ASSERT(SIZEOF(.high)          == 0x0001750C, "ARM7: tamanho do autoload alto difere do original")
    ASSERT(SIZEOF(.high_bss)      == 0x000028F8, "ARM7: .bss do autoload alto difere do original")
    ASSERT(__autoload_list_start  == 0x023A907C, "ARM7: lista de autoload fora do lugar original")
#endif
}

#endif

SECTIONS
{
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
        *(.ARM.exidx* .ARM.extab*)
        *(.ARM.attributes)
    }
}
