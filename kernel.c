#include "video.h"
#include "keyboard.h"

void kernel_main() {
    clear_screen();
    print_string("BITHORA V. 0.004.9\n");
    print_string("Bith> ");
    wait_for_keypress();
}
