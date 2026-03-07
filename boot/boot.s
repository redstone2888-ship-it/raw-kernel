; boot.s (NASM syntax)
section .multiboot
align 4
    dd 0x1BADB002                 ; magic
    dd 0x00000003                 ; flags
    dd -(0x1BADB002 + 0x00000003) ; checksum

section .text
extern kernel_main
global _start

_start:
    push ebx
    push eax
    
    call kernel_main

.loop:
    hlt
    jmp .loop