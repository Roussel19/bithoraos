#include "video.h"
#include "keyboard.h"

void kernel_main() {
    clear_screen();
    print_string("Bith> ");
    wait_for_keypress();
}
