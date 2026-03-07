; boot.s - GRUB Bootloader entry point

; Copyright (c) 2026, Redstone2888
; Read LICENSE.txt for details

; This file sets up the environment for the kernel and is required
; for proper booting via GRUB. 
; DO NOT DELETE THIS FILE — removing it will prevent the kernel from booting.

section .multiboot
align 4
    dd 0x1BADB002                 ; magic
    dd 0x00000003                 ; flags
    dd -(0x1BADB002 + 0x00000003) ; checksum

section .text
extern kernel_main
global _start

; We push eax, ebx onto the stack and pass them to the kernel
_start:
    push ebx
    push eax
    
    call kernel_main

; If kernel_main crashed (inpossible), hanging out
.loop:
    hlt
    jmp .loop