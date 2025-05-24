#include <stdint.h>
#include "video.h"
#include "ports.h"

#define MAX_COLS 80
#define MAX_ROWS 25
#define COLOR_ATTRIBUTE 0x3F
#define MAX_LINE_LENGTH 128
#define MAX_HISTORY 10

volatile uint16_t* video_memory = (uint16_t*) 0xB8000;
uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

char line_buffer[MAX_LINE_LENGTH];
int line_length = 0;
int cursor_pos = 0;

char history[MAX_HISTORY][MAX_LINE_LENGTH];
int history_count = 0;
int history_index = -1;

int get_offset(int col, int row) {
    return row * MAX_COLS + col;
}

void clear_screen() {
    for (int i = 0; i < MAX_COLS * MAX_ROWS; i++) {
        video_memory[i] = (COLOR_ATTRIBUTE << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void update_cursor() {
    uint16_t offset = get_offset(cursor_x, cursor_y);
    outb(0x3D4, 14);
    outb(0x3D5, (offset >> 8) & 0xFF);
    outb(0x3D4, 15);
    outb(0x3D5, offset & 0xFF);
}

void scroll_screen() {
    for (int row = 1; row < MAX_ROWS; row++) {
        for (int col = 0; col < MAX_COLS; col++) {
            int from = get_offset(col, row);
            int to = get_offset(col, row - 1);
            video_memory[to] = video_memory[from];
        }
    }

    for (int col = 0; col < MAX_COLS; col++) {
        int offset = get_offset(col, MAX_ROWS - 1);
        video_memory[offset] = (COLOR_ATTRIBUTE << 8) | ' ';
    }

    cursor_y = MAX_ROWS - 1;
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
        video_memory[offset] = (COLOR_ATTRIBUTE << 8) | ' ';
    } else {
        int offset = get_offset(cursor_x, cursor_y);
        video_memory[offset] = (COLOR_ATTRIBUTE << 8) | c;
        cursor_x++;
        if (cursor_x >= MAX_COLS) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= MAX_ROWS) {
        scroll_screen();
    }
    update_cursor();
}

void print_string(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        print_char(str[i]);
    }
}

void refresh_line() {
    const char* prompt = "Bith> ";

    for (int i = 0; prompt[i] != '\0'; i++) {
        int offset = get_offset(i, cursor_y);
        video_memory[offset] = (COLOR_ATTRIBUTE << 8) | prompt[i];
    }

    for (int i = 5; i < MAX_COLS; i++) {
        int offset = get_offset(i, cursor_y);
        video_memory[offset] = (COLOR_ATTRIBUTE << 8) | ' ';
    }

    for (int i = 0; i < line_length; i++) {
        int offset = get_offset(5 + i, cursor_y);
        video_memory[offset] = (COLOR_ATTRIBUTE << 8) | line_buffer[i];
    }

    cursor_x = 5 + cursor_pos;
    update_cursor();
}

void insert_char(char c) {
    if (line_length >= MAX_LINE_LENGTH - 1) return;

    for (int i = line_length; i > cursor_pos; i--) {
        line_buffer[i] = line_buffer[i - 1];
    }

    line_buffer[cursor_pos] = c;
    line_length++;
    cursor_pos++;
    refresh_line();
}

void backspace_char() {
    if (cursor_pos == 0) return;

    for (int i = cursor_pos - 1; i < line_length - 1; i++) {
        line_buffer[i] = line_buffer[i + 1];
    }

    line_length--;
    cursor_pos--;
    refresh_line();
}
