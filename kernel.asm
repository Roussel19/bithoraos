[bits 16]
[org 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax

    call clear_screen
    mov si, welcome_msg
    call print_string

.wait_key:
    in al, 0x64
    test al, 1
    jz .wait_key
    in al, 0x60          ; leer la tecla
    call print_hex       ; imprimir scan code en pantalla
    jmp .wait_key

; --- Imprime mensaje ---
print_string:
    mov ah, 0x0E
.next:
    lodsb
    or al, al
    jz .done
    int 0x10
    jmp .next
.done:
    ret

; --- Limpia pantalla ---
clear_screen:
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    ret

; --- Imprime en hex ---
print_hex:
    mov ah, 0x0E
    mov bx, ax
    call print_digit
    mov al, bl
    call print_digit
    ret

print_digit:
    and al, 0x0F
    add al, '0'
    cmp al, '9'
    jbe .ok
    add al, 7
.ok:
    int 0x10
    ret

welcome_msg db "Bienvenido a RouX OS - Presiona una tecla", 0

times 510 - ($ - $$) db 0
dw 0xAA55
