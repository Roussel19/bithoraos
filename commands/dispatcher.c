#include "../include/commands.h"
#include "../include/lib.h"
#include "../video.h"

void execute_command(const char* input) {
    if (strcmp(input, "vacuum") == 0) {
        vacuum();
    } else {
        print_string("Comando no reconocido.\n");
    }
}
