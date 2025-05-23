BUILD_DIR = build

all: iso

# Crear carpeta build si no existe
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilar el header Multiboot (ASM)
$(BUILD_DIR)/multiboot.o: multiboot_header.asm | $(BUILD_DIR)
	nasm -f elf32 multiboot_header.asm -o $(BUILD_DIR)/multiboot.o

# Compilar el kernel en C
$(BUILD_DIR)/kernel.o: kernel.c | $(BUILD_DIR)
	i686-elf-gcc -ffreestanding -m32 -c kernel.c -o $(BUILD_DIR)/kernel.o

# Enlazar ambos objetos al binario final
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/multiboot.o $(BUILD_DIR)/kernel.o linker.ld
	i686-elf-ld -T linker.ld -o $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/multiboot.o $(BUILD_DIR)/kernel.o -nostdlib

# Crear ISO booteable con GRUB
iso: $(BUILD_DIR)/kernel.bin
	mkdir -p $(BUILD_DIR)/iso/boot/grub
	cp $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/iso/boot/kernel.bin
	cp grub/grub.cfg $(BUILD_DIR)/iso/boot/grub/grub.cfg
	grub-mkrescue -o RouX.iso $(BUILD_DIR)/iso

# Ejecutar en QEMU
run: iso
	qemu-system-i386 -cdrom RouX.iso

# Limpiar todo
clean:
	rm -rf build RouX.iso
