#include <stdint.h>
#include "ports.h"

uint8_t key_pressed = 0;
uint8_t scancode_down = 0;

// Video
volatile uint16_t* video_memory = (uint16_t*) 0xB8000;
uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

#define MAX_COLS 80
#define MAX_ROWS 25

// ===== Funciones de video =====
int get_offset(int col, int row) {
    return row * MAX_COLS + col;
}

void clear_screen() {
    for (int i = 0; i < MAX_COLS * MAX_ROWS; i++) {
        video_memory[i] = (0x07 << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}

void update_cursor() {
    uint16_t offset = get_offset(cursor_x, cursor_y);

    outb(0x3D4, 14); // Seleccionar byte alto
    outb(0x3D5, (offset >> 8) & 0xFF);

    outb(0x3D4, 15); // Seleccionar byte bajo
    outb(0x3D5, offset & 0xFF);
}

void print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = MAX_COLS - 1;
        }
        int offset = get_offset(cursor_x, cursor_y);
        video_memory[offset] = (0x07 << 8) | ' ';
    } else {
        int offset = get_offset(cursor_x, cursor_y);
        video_memory[offset] = (0x07 << 8) | c;
        cursor_x++;
        if (cursor_x >= MAX_COLS) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= MAX_ROWS) {
        cursor_y = 0; // Puedes implementar scroll aquí si quieres
    }
    update_cursor(); // 👉 Esto mueve el cursor visible
}

void print_string(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        print_char(str[i]);
    }
}

// ===== Teclado =====
char scancode_to_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t', 'q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, '\\','z','x','c','v','b','n','m',',','.','/',
    0, '*', 0, ' ', 0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0,0,0,
    0,0,0,0,0,0,0,0
};
char scancode_to_ascii_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0, 'A','S','D','F','G','H','J','K','L',':','"','~',
    0, '|','Z','X','C','V','B','N','M','<','>','?',
    0, '*', 0, ' ', 0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0,0,0,
    0,0,0,0,0,0,0,0
};

int shift_pressed = 0;

uint8_t keyboard_read_scancode() {
    while (!(inb(0x64) & 0x01));  // Esperar hasta que el buffer tenga datos
    return inb(0x60);
}

void wait_for_keypress() {
    int is_shift_pressed = 0;
    int backspace_held = 0;
    int backspace_repeat_counter = 0;

    while (1) {
        uint8_t scancode = keyboard_read_scancode();

        if (scancode & 0x80) {
            uint8_t released = scancode & 0x7F;
            if (released == 0x2A || released == 0x36) {
                is_shift_pressed = 0;
            }
            if (released == 0x0E) {
                backspace_held = 0;
                backspace_repeat_counter = 0;
            }
        } else {
            if (scancode == 0x2A || scancode == 0x36) {
                is_shift_pressed = 1;
                continue;
            }

            if (scancode == 0x0E) {
                if (!backspace_held) {
                    print_char('\b');
                    update_cursor();
                    backspace_held = 1;
                    backspace_repeat_counter = 0;
                } else {
                    backspace_repeat_counter++;
                    if (backspace_repeat_counter >= 5) {
                        print_char('\b');
                        update_cursor();
                        backspace_repeat_counter = 0;
                    }
                }
                continue;
            }

            char key = is_shift_pressed ? scancode_to_ascii_shift[scancode] : scancode_to_ascii[scancode];
            if (key) {
                print_char(key);
                if (key == '\n') {
                    print_string("Bith> ");
                }
            }
        }
    }
}


// ===== Punto de entrada del kernel =====
void kernel_main() {
    clear_screen();
    print_string("BithoraOS v0.003\n\n");
    print_string("Bith> ");
    wait_for_keypress();
}
