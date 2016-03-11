This code is about the simplest possible to demonstrate AArch64 code running
on the Raspberry Pi 3.

Most of the code is stolen from U-Boot, including some from my not-yet-
upstream AArch32 port of U-Boot to the Raspberry Pi:
* git://git.denx.de/u-boot.git
* git://github.com/swarren/u-boot.git

To build, simply run `./build.sh`. This was tested on Ubuntu 14.04.

To make a bootable SD card, start with a recent Raspbian image, then:
* Copy config.txt to the boot partition.
* Delete `kernel.img` from the boot partition.
* Copy `test.bin` to the boot partition as `kernel7.img`.

You'll need a serial console running at 115200 baud to observe the code
running. 
