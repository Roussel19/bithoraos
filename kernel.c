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

#define MAX_LINE_LENGTH 128
char line_buffer[MAX_LINE_LENGTH];
int line_length = 0;    // Cuántos caracteres hay en la línea
int cursor_pos = 0;     // Dónde está el cursor dentro del buffer

#define MAX_HISTORY 10
char history[MAX_HISTORY][MAX_LINE_LENGTH];
int history_count = 0;
int history_index = -1; // -1 cuando no estás navegando

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

void refresh_line() {
    const char* prompt = "Bith> ";

    // Imprimir el prompt desde la columna 0
    for (int i = 0; prompt[i] != '\0'; i++) {
        int offset = get_offset(i, cursor_y);
        video_memory[offset] = (0x07 << 8) | prompt[i];
    }

    // Limpiar el resto de la línea desde después del prompt (col 5) hasta el final
    for (int i = 5; i < MAX_COLS; i++) {
        int offset = get_offset(i, cursor_y);
        video_memory[offset] = (0x07 << 8) | ' ';
    }

    // Imprimir el buffer a partir de la columna 5
    for (int i = 0; i < line_length; i++) {
        int offset = get_offset(5 + i, cursor_y);
        video_memory[offset] = (0x07 << 8) | line_buffer[i];
    }

    // Poner el cursor en la posición correcta (col 5 + cursor_pos)
    cursor_x = 5 + cursor_pos;
    update_cursor();
}



void insert_char(char c) {
    if (line_length >= MAX_LINE_LENGTH - 1) return;  // evitar overflow

    // Mover los caracteres a la derecha para hacer espacio
    for (int i = line_length; i > cursor_pos; i--) {
        line_buffer[i] = line_buffer[i - 1];
    }

    line_buffer[cursor_pos] = c;
    line_length++;
    cursor_pos++;
    refresh_line();
}


void backspace_char() {
    if (cursor_pos == 0) return;  // No se puede borrar antes del inicio

    // Mover los caracteres hacia la izquierda para borrar el de la izquierda del cursor
    for (int i = cursor_pos - 1; i < line_length - 1; i++) {
        line_buffer[i] = line_buffer[i + 1];
    }

    line_length--;
    cursor_pos--;
    refresh_line();
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

        if (scancode == 0xE0) { // Tecla extendida
            uint8_t ext_scancode = keyboard_read_scancode();

            if (ext_scancode == 0x4B) { // Flecha izquierda
                if (cursor_pos > 0) {
                    cursor_pos--;
                    refresh_line();
                }
            } else if (ext_scancode == 0x4D) { // Flecha derecha
                if (cursor_pos < line_length) {
                    cursor_pos++;
                    refresh_line();
                }
            } else if (ext_scancode == 0x48) { // Flecha ↑
                if (history_count > 0) {
                    if (history_index < history_count - 1) {
                        history_index++;
                        for (int i = 0; i < MAX_LINE_LENGTH; i++) {
                            line_buffer[i] = history[history_count - 1 - history_index][i];
                        }
                        line_length = 0;
                        while (line_buffer[line_length] != '\0') line_length++;
                        cursor_pos = line_length;
                        refresh_line();
                    }
                }
            } else if (ext_scancode == 0x50) { // Flecha ↓
                if (history_index > 0) {
                    history_index--;
                    for (int i = 0; i < MAX_LINE_LENGTH; i++) {
                        line_buffer[i] = history[history_count - 1 - history_index][i];
                    }
                    line_length = 0;
                    while (line_buffer[line_length] != '\0') line_length++;
                    cursor_pos = line_length;
                    refresh_line();
                } else if (history_index == 0) {
                    history_index = -1;
                    line_length = 0;
                    cursor_pos = 0;
                    line_buffer[0] = '\0';
                    refresh_line();
                }
            }

            continue;
        }

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
                    backspace_char();
                    backspace_held = 1;
                    backspace_repeat_counter = 0;
                } else {
                    backspace_repeat_counter++;
                    if (backspace_repeat_counter >= 5) {
                        backspace_char();
                        backspace_repeat_counter = 0;
                    }
                }
                continue;
            }

            char key = is_shift_pressed ? scancode_to_ascii_shift[scancode] : scancode_to_ascii[scancode];
            if (key) {
                if (key == '\b') {
                    backspace_char();
                } else if (key == '\n') {
                    print_char('\n');

                    if (line_length > 0 && history_count < MAX_HISTORY) {
                        for (int i = 0; i < line_length; i++) {
                            history[history_count][i] = line_buffer[i];
                        }
                        history[history_count][line_length] = '\0';
                        history_count++;
                    }

                    history_index = -1;
                    line_length = 0;
                    cursor_pos = 0;
                    line_buffer[0] = '\0';
                    print_string("Bith> ");
                } else {
                    insert_char(key);
                }
            }
        }
    }
}



void kernel_main() {
    clear_screen();
    print_string("BithoraOS v0.003\n\n");
    print_string("Bith>  ");
    wait_for_keypress();
}
