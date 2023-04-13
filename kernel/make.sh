arm-none-eabi-as -mcpu=arm926ej-s -g kernel.s -o ts.o
arm-none-eabi-gcc -c -mcpu=arm926ej-s -g main.c -o t.o -Wall
arm-none-eabi-ld -T t.ld ts.o t.o -o t.elf
arm-none-eabi-objcopy -O binary t.elf t.bin

rm *.elf *.o
echo ready to go?
read dummy

qemu-system-arm -M versatilepb -m 128M -kernel t.bin -serial mon:stdio









 





