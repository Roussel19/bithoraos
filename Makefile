all: os.img

os.img: bootloader.asm
	nasm -f bin bootloader.asm -o os.img

run: os.img
	qemu-system-i386 -drive format=raw,file=os.img

clean:
	rm -f *.img
