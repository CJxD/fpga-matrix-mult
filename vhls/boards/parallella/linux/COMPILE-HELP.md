Copyright (C) 2015 XPARCH, Ltd.

DJG is using arm-linux-gnu-gcc (GCC) 4.9.1 20140717 (Red Hat Cross 4.9.1-1)

The kernel is compiled with the following flags but I am not sure whether I am targeting ARM7 or ARM9.

ARCHFLAGS= -mabi=aapcs-linux -mno-thumb-interwork -mfpu=vfp  -mlittle-endian



We will also need the SD card filesystem device and the Ethernet devices working.



------------------------------
How to add a printk backdoor for early output:

Without the following edit, the kernel writes its console output to a RAM buffer and then only emits it on the uart once the
uart is properly set up by the device tree.  The only exceptions are the very lowlevel calls to printascii that output on the 
UART if the machine identity register is wrong in the head.S startup code.



Put the following two lines after the call to vscnprint in the function vprintk_emit of the file kernel/printk/printk.c

	text_len = vscnprintf(text, sizeof(textbuf), fmt, args);

        char *pp = text;
        while (*pp) craft_wrch(*pp++);

And somewhere else provide an implementation of a craft_wrch that makes a polled write to the uart.

You could call the arm implementation in head.S created by the macro arch/arm/include/debug/zynq.S
or else just use

void craft_wrch(char c*) {  ((volatile char *)0xEEEE1030)[0] = c; }   

Also note there is a printascii function in the arch/arm/debug.S that makes low-level uart output and it should work
if you call that as well but since our UART never goes unready on write the above, non-polling direct store is fine.

Once the UART is working with its proper (interrupt driven) output we can disable the backdoor.

------------------------------


How to compile the ADI based linux kernel? (uImage)

git clone https://github.com/parallella/parallella-linux
cd parallella-linux bash
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnu-
export PATH=:$PATH
make ARCH=arm parallella_defconfig
make ARCH=arm LOADADDR=0x8000 uImage



#####################################################################

How to compile the device tree? (devicetree.dtb)

scripts/dtc/dtc -I dts -O dtb -o arch/arm/boot/zynq-zed.dtb arch/arm/boot/dts/zynq-zed.dts

###################################################################

How to create the FPGA bitstream? (parallella.bit.bin)
Use the Xilinx tool chain (ISE 14.4)

####################################################################

How to create the Zynq first stage loader? (fsbl.elf)?
Use the Xilinx tool chain (ISE 14.4)

#########################################################

How to compile u-boot? (u-boot.elf)

sudo apt-get install u-boot-tools

git clone https://github.com/parallella/parallella-uboot.git

cd parallella-uboot

bash

export ARCH=arm
export CROSS_COMPILE= #eg arm-xilinx-linux-gnuabi-
export PATH=:$PATH
make parallella_config
make -j 2

#############################################################

How to create the qspi flash image? (parallella.bin)

Create a ".bif" configuration file with the following content:
the_ROM_image:
{
[bootloader]/path/to/fsbl.elf
/path/to/u-boot.elf
}

Run bootgen:

bootgen -image /path/to/image.bif -o i parallella.bin

#############################################################

How to create an SD card?

Method #1: Download

Method #2: 1.Use gparted to crete two partitions: -The first partition will hold the boot loader, devicetree and kernel images. It should be about 50MiB in size, and formatted with FAT file system. Label this partition as "BOOT" -The second partition will occupy the rest of the available space. It will store the system’s root file sytem, and should be formatted with ext4 file system. This partition should be labelled as "rootfs"

Get a Linux/FPGA achive from http://github.com/parallella/parallella-bin.

Unzip the archive and copy files to BOOT partition on SD card

Download root file system from: http://releases.linaro.org/13.12/ubuntu/saucy-images/developer/linaro-saucy-server-20131216-586.tar.gz
sudo tar -zxvf linaro-saucy-developer-20131216-586.tar.gz
cd binary
sudo rsync -a --progress ./ /media/"username"/rootfs

wget http://releases.linaro.org/14.01/ubuntu/saucy-images/nano/linaro-saucy-nano-20140126-627.tar.gz https://raw.github.com/parallella/parallella-bin/master/rel.14.02.06.tgz sudo tar --strip-components=1 -C /media/aolofsson/rootfs -xzpf linaro-saucy-nano-20140126-627.tar.gz sudo emacs sudo emacs /media/aolofsson/rootfs/etc/network/interfaces ;;insert following auto lo iface lo inet loopback allow-hotplug eth0 auto eth0 iface eth0 inet dhcp

#############################################################
Status API Training Shop Blog About
© 2014 GitHub, Inc. Terms Privacy Security Contact

#### 

Use this

make ARCH=arm CROSS_COMPILE=arm-linux-gnu- V=1

-mabi=aapcs-linux -mno-thumb-interwork -mfpu=vfp  -mlittle-endian

  arm-linux-gnu-gcc -Wp,-MD,kernel/.exit.o.d  -nostdinc -isystem /usr/lib/gcc/arm-linux-gnueabi/4.9.1/include -I/home/djg11/parallella-sw/complete-from-git/parallella-linux/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/home/djg11/parallella-sw/complete-from-git/parallella-linux/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/home/djg11/parallella-sw/complete-from-git/parallella-linux/include/uapi -Iinclude/generated/uapi -include /home/djg11/parallella-sw/complete-from-git/parallella-linux/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -Wno-maybe-uninitialized -fno-dwarf2-cfi-asm -mabi=aapcs-linux -mno-thumb-interwork -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -DCC_HAVE_ASM_GOTO   -fno-stack-protector -D"KBUILD_STR(s)=#s" -D"KBUILD_BASENAME=KBUILD_STR(exit)"  -D"KBUILD_MODNAME=KBUILD_STR(exit)" -c -o kernel/.tmp_exit.o kernel/exit.c
