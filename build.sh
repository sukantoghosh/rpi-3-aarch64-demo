#!/bin/bash

set -e
set -x

CROSS_COMPILE=aarch64-linux-gnu-

${CROSS_COMPILE}gcc -c -o armstub64.o armstub64.S
${CROSS_COMPILE}ld -Bstatic --gc-sections -nostartfiles -nostdlib -o armstub64.elf -Ttext 0 -T armstub64.lds armstub64.o
${CROSS_COMPILE}objcopy --gap-fill=0xff -j .text -j .rodata -j .data -O binary armstub64.elf armstub64.bin
${CROSS_COMPILE}objdump -d armstub64.elf > armstub64.dis

${CROSS_COMPILE}gcc -c -o start.o start.S
${CROSS_COMPILE}gcc -c -o main.o main.c
${CROSS_COMPILE}ld -Bstatic --gc-sections -nostartfiles -nostdlib -o app.elf -Ttext 0x8000 -T app.lds start.o main.o
${CROSS_COMPILE}objcopy --gap-fill=0xff -j .text -j .rodata -j .data -O binary app.elf app.bin
${CROSS_COMPILE}objdump -d app.elf > app.dis

cat armstub64.bin app.bin > test.bin
