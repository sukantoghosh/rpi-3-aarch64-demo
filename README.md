This repository contains a proposed "ARM boot stub" for the Raspberry Pi 3
running in AArch64 mode. It is anticipated the boot stub will be integrated
into the VC FW. A very simple test application is included to demonstrate its
use.

The ARM boot stub was inspired by the AArch32 boot stub code that the
Raspberry Pi Foundation published at my request in
https://github.com/raspberrypi/firmware/issues/579
The license/copyright match their code release.

Some of the application code is stolen from U-Boot, including some from my
recently upstreamed AArch32 port of U-Boot to the Raspberry Pi:
* git://git.denx.de/u-boot.git
* git://github.com/swarren/u-boot.git

To build, simply run `./build.sh`. This was tested on Ubuntu 16.04. You will
need an aarch64-linux-gnu-gcc binary (i.e. gcc-aarch64-linux-gnu package) or
equivalent.

To make a bootable SD card, start with a recent Raspbian image, update all the
firmware files to the latest from firmware.git (I tested with  046effa
"firmware: arm_loader: emmc clock depends on core clock See: #572", then:
* Copy config.txt to the boot partition.
* Delete `kernel.img` from the boot partition.
* Copy `test.bin` to the boot partition as `kernel7.img`.

You'll need a serial console running at 115200 baud to observe the code
running. 
