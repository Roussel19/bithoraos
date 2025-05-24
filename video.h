#ifndef VIDEO_H
#define VIDEO_H

void clear_screen();
void print_char(char c);
void print_string(const char* str);
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
