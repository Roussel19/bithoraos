BUILD_DIR = build
ISO_NAME = BithoraOS.iso

all: iso

# Crear carpeta build si no existe
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilar el header Multiboot (ASM)
$(BUILD_DIR)/multiboot.o: multiboot_header.asm | $(BUILD_DIR)
	nasm -f elf32 multiboot_header.asm -o $(BUILD_DIR)/multiboot.o

# Compilar kernel
$(BUILD_DIR)/kernel.o: kernel.c | $(BUILD_DIR)
	i686-elf-gcc -ffreestanding -m32 -c kernel.c -o $(BUILD_DIR)/kernel.o

# Compilar puertos
$(BUILD_DIR)/ports.o: ports.c | $(BUILD_DIR)
	i686-elf-gcc -ffreestanding -m32 -c ports.c -o $(BUILD_DIR)/ports.o

# Compilar video
$(BUILD_DIR)/video.o: video.c | $(BUILD_DIR)
	i686-elf-gcc -ffreestanding -m32 -c video.c -o $(BUILD_DIR)/video.o

# Compilar teclado
$(BUILD_DIR)/keyboard.o: keyboard.c | $(BUILD_DIR)
	i686-elf-gcc -ffreestanding -m32 -c keyboard.c -o $(BUILD_DIR)/keyboard.o

# Enlazar todo en kernel.bin
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/multiboot.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/ports.o $(BUILD_DIR)/video.o $(BUILD_DIR)/keyboard.o linker.ld
	i686-elf-ld -T linker.ld -o $(BUILD_DIR)/kernel.bin \
		$(BUILD_DIR)/multiboot.o \
		$(BUILD_DIR)/kernel.o \
		$(BUILD_DIR)/ports.o \
		$(BUILD_DIR)/video.o \
		$(BUILD_DIR)/keyboard.o \
		-nostdlib

# Crear ISO booteable
iso: $(BUILD_DIR)/kernel.bin
	mkdir -p $(BUILD_DIR)/iso/boot/grub
	cp $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/iso/boot/kernel.bin
	cp grub/grub.cfg $(BUILD_DIR)/iso/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_NAME) $(BUILD_DIR)/iso

# Ejecutar en QEMU
run: iso
	qemu-system-i386 -cdrom $(ISO_NAME)

# Limpiar todo
clean:
	rm -rf $(BUILD_DIR) $(ISO_NAME)
