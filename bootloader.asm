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

    in al, 0x60
    cmp al, 0x80         ; ignorar si es "key release"
    jae .wait_key

    movzx bx, al
    mov al, [keymap + bx]
    call print_char
    jmp .wait_key

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

print_char:
    cmp al, 0
    je .skip
    mov ah, 0x0E
    int 0x10
.skip:
    ret

clear_screen:
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    ret

welcome_msg db "RouX OS - Presiona teclas", 0

; Keymap simple (us layout)
keymap:
    db 0,  27,'1','2','3','4','5','6','7','8','9','0','-','=', 8, 9
    db 'q','w','e','r','t','y','u','i','o','p','[',']', 13, 0,'a','s'
    db 'd','f','g','h','j','k','l',';','\'','`', 0,'\\','z','x','c','v'
    db 'b','n','m',',','.','/', 0,'*', 0,' ',0,0,0,0,0,0
    times 128-($ - keymap) db 0

times 510 - ($ - $$) db 0
dw 0xAA55
