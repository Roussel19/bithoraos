#include "video.h"
#include "keyboard.h"
#include "include/commands.h"

void kernel_main() {
    clear_screen();
    print_string("Bithora V.004.9\n");
    print_string("Bith> ");
    
    while (1) {
        wait_for_keypress();
        line_buffer[line_length] = '\0';  // asegúrate de terminar la cadena
        execute_command(line_buffer);
        print_string("Bith> ");
        line_length = 0;
        cursor_pos = 0;
    }
}
