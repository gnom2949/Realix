; © Realix > Thirdix > Entry trampoline
; =====================================
; Первое что выполняется после jmp.

bits 32

global _start
extern thirdix_entry
extern _bss_start
extern _bss_end

section .text.entry
_start:
    mov edi, _bss_start
    mov ecx, _bss_end
    sub ecx, edi
    xor eax, eax
    cld
    rep stosb

    push ebx   
    call thirdix_entry

    cli
    hlt
    jmp $