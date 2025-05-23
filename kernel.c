// kernel.c
void print_char(char c) {
    char* video = (char*) 0xb8000;
    video[0] = c;
    video[1] = 0x07;
}

void kernel_main() {
    print_char('R');
}
