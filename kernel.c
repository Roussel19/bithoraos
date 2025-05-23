// kernel.c

#include <stdint.h>

// Posición del cursor
uint16_t* video_memory = (uint16_t*) 0xB8000;
uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

// Tamaño de la pantalla (80x25)
#define MAX_COLS 80
#define MAX_ROWS 25

// Calcular posición en memoria
int get_offset(int col, int row) {
    return row * MAX_COLS + col;
}

// Imprimir un solo carácter en pantalla
void print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        int offset = get_offset(cursor_x, cursor_y);
        video_memory[offset] = (0x07 << 8) | c;
        cursor_x++;
        if (cursor_x >= MAX_COLS) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    // No dejar que se pase del límite
    if (cursor_y >= MAX_ROWS) {
        cursor_y = 0;
    }
}

// Imprimir cadena completa
void print_string(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        print_char(str[i]);
    }
}

// Función principal del kernel
void kernel_main() {
    print_string("Bienvenido a RouX OS\n");
    print_string("Ya tienes control total del hardware.\n");
}
