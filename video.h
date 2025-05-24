#include <stdint.h>
#ifndef VIDEO_H
#define VIDEO_H

#define GREEN_ON_BLACK  0x0A
#define COLOR_ATTRIBUTE GREEN_ON_BLACK

void clear_screen();
void print_char(char c);
void print_string(const char* str);
void print_colored_string(const char* str, uint8_t color);
void update_cursor();
void refresh_line();
void insert_char(char c);
void scroll_screen();
void backspace_char();

extern char line_buffer[];
extern int line_length;
extern int cursor_pos;

extern int history_count;
extern int history_index;
extern char history[10][128];

#endif
