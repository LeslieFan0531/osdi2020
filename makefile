SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

all: kernel8.img

start.o: start.S
	aarch64-linux-gnu-gcc -c start.S

kearnel8.elf kernel8.img: start.o $(OBJS)
	aarch64-linux-gnu-ld -T linker.ld -o kernel8.elf start.o $(OBJS)
	aarch64-linux-gnu-objcopy -O binary kernel8.elf kernel8.img

%.o: %.c
	aarch64-linux-gnu-gcc $(CFLAGS) -c $< -o $@

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio

run-detail:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio

run-uart0:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -d in_asm -serial null -serial stdio

connect:
	sudo screen /dev/ttyUSB0 115200

clean:
	rm *.o kernel8.img kernel8.elf
