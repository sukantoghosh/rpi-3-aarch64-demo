#!/bin/bash

set -e

CROSS_COMPILE=aarch64-linux-gnu-

${CROSS_COMPILE}gcc -c -o start.o start.S
${CROSS_COMPILE}gcc -c -O2 -o main.o main.c
${CROSS_COMPILE}ld -Bstatic --gc-sections -nostartfiles -nostdlib -o test -Ttext 0 -T test.lds start.o main.o
${CROSS_COMPILE}objcopy --gap-fill=0xff -j .text -j .rodata -j .data -O binary test test.bin
${CROSS_COMPILE}objdump -d test > test.dis

