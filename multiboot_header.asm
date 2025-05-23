; multiboot_header.asm

section .multiboot
align 4
    dd 0x1BADB002          ; Magic number
    dd 0x00                ; Flags
    dd - (0x1BADB002 + 0x00) ; Checksum

section .text
global _start
extern kernel_main

_start:
    cli                    ; Desactivar interrupciones
    call kernel_main       ; Llamar al kernel en C
hang:
    hlt                    ; Detener CPU
    jmp hang               ; Bucle infinito
