#include <stdint.h>
#include "keyboard.h"
#include "video.h"
#include "ports.h"
#include "kernel.h"
#include "include/commands.h"

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

uint8_t keyboard_read_scancode() {
    while (!(inb(0x64) & 0x01));
    return inb(0x60);
}

void wait_for_keypress() {
    int is_shift_pressed = 0;
    int backspace_held = 0;
    int backspace_repeat_counter = 0;

    while (1) {
        uint8_t scancode = keyboard_read_scancode();

        if (scancode == 0xE0) {
            uint8_t ext = keyboard_read_scancode();
            if (ext == 0x4B && cursor_pos > 0) cursor_pos--;
            else if (ext == 0x4D && cursor_pos < line_length) cursor_pos++;
            else if (ext == 0x48 && history_count > 0 && history_index < history_count - 1) {
                history_index++;
                for (int i = 0; i < MAX_LINE_LENGTH; i++)
                    line_buffer[i] = history[history_count - 1 - history_index][i];
                line_length = 0;
                while (line_buffer[line_length]) line_length++;
                cursor_pos = line_length;
            } else if (ext == 0x50 && history_index >= 0) {
                history_index--;
                if (history_index >= 0) {
                    for (int i = 0; i < MAX_LINE_LENGTH; i++)
                        line_buffer[i] = history[history_count - 1 - history_index][i];
                    line_length = 0;
                    while (line_buffer[line_length]) line_length++;
                    cursor_pos = line_length;
                } else {
                    line_length = 0;
                    cursor_pos = 0;
                    line_buffer[0] = '\0';
                }
            }
            refresh_line();
            continue;
        }

        if (scancode & 0x80) {
            uint8_t released = scancode & 0x7F;
            if (released == 0x2A || released == 0x36) is_shift_pressed = 0;
            if (released == 0x0E) backspace_held = 0;
        } else {
            if (scancode == 0x2A || scancode == 0x36) {
                is_shift_pressed = 1;
                continue;
            }

            if (scancode == 0x0E) {
                if (!backspace_held) {
                    backspace_char();
                    backspace_held = 1;
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
                if (key == '\n') {
                    print_char('\n');
                    if (line_length > 0 && history_count < 10) {
                        for (int i = 0; i < line_length; i++)
                            history[history_count][i] = line_buffer[i];
                        history[history_count][line_length] = '\0';
                        history_count++;
                    }
                    line_buffer[line_length] = '\0';       // Aseguramos que sea string válida
					execute_command(line_buffer); 
					 
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
