# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
#
#
#  Work in progress to emulate the parallella linux boot.
#

ifeq ($(strip $(VHLS)),)
$(error Please set VHLS variable to the directory that contains the PRAZOR simulator source code)
endif

ifeq ($(strip $(LKERNEL)),)
$(error Please set LKERNEL variable to the directory that holds Parallela linux kernel source code)
endif

ifeq ($(strip $(ARMCROSS)),)
$(error Please set ARMCROSS variable to ARM cross toolchain prefix, e.g. arm-linux-gnu-)
endif


KIMAGE =$(LKERNEL)/vmlinux
# KIMAGE=/home/djg11/parallella-sw/linux-xlnx/vmlinux
# KIMAGE =vmlinux-from-parcard1

DRAM_FLAGS ?= -dram-system-ini $(VHLS)/src/dramsim2/dist/system.ini.example  -dram-device $(VHLS)/boards/parallella/DDR3_micron_32M_8B_x32_parallella.ini


SIM=$(VHLS)/src/arm/vhls-arm7smp


all:dis run

run:
#	ulimit -t100 ;	$(SIM) -imagehi $(KIMAGE)  -image loimage $(DRAM_FLAGS) -tracelevel 8 -no-caches   -tracelevel 0
# -watch c00ac440 +16  -watch C01BD258 +16 -watch c0054380 +16 -watch  c065a058 +16  -watch 0 16 



#	ulimit -t2000 ;	$(SIM) -kernel $(KIMAGE) -devicetree prazor-linux.dtb -boot loimage -vdd disk4.img $(DRAM_FLAGS) -tracelevel 9 -cores 2 
	ulimit -t2000 ;	$(SIM) -kernel $(KIMAGE) -devicetree prazor-linux.dtb -boot loimage -vdd disk6.img $(DRAM_FLAGS) -core-frequency 667 -cores 2

#  -ptracer $(HOME)/ptracer-log.bin
#   -tracelevel 10 -watch C04E053C +64 -tracelevel 0 



#-tracelevel 9 -watch ffff10a0 +116 -watch c0011aa0 +5c -watch c001b200 +20 -watch c0008398 +98 -watch b6e00000 +200000 -watch 16d20000 +10000 -watch be000000 +1000000 -watch c002c96c +20 -watch 8000 +8000 -watch a0000 +20000

#-tracelevel 9 -watch ffff1000 +120 -watch C0011A40 +128 -watch C063ACA0 +32 -watch c00084c8 +100 -watch C000E7B8 +256 -watch C00574F0 +64 -watch c005a7dc +310 -watch c062ff78 +60

#-tracelevel 9 -watch ffff0000 +128 -watch FFFF1000 +128 -watch C0011A40 +128 -watch C063ACA0 +32 -watch c00084c8 +100 -watch C000E7B8 +256 -watch C00574F0 +64 -watch c005a7dc +310 -watch c0057a78 +368 -watch c02f8a50 +40

#-tracelevel 9 -watch ffff0000 +128 -watch FFFF1000 +128 -watch C0011A40 +128 -watch C063ACA0 +32 -watch C0023400 +256 -watch C000E7B8 +256 -watch C00574F0 +64 -watch C005A980 +256 -watch c00138a8 +64 -watch c0067754 +256 -watch c00675cc +168 -watch c0062cc4 +2084


#  -wait-debugger -no-caches
#  -watch c000fddc  +1000
#c01c6a28

# -watch c00081bc +1024 -tracelevel 0  -watch c01c4e60 +16 


# ARM9 - use gcc as the assembler since I am not sure what is the equiv for these args for as.
ARCHFLAGS=-march=armv7-a -mabi=aapcs-linux -mno-thumb-interwork -mfpu=vfp  -mlittle-endian

loimage:loboot.o
	$(ARMCROSS)ld -o loimage  -Ttext 0x0 loboot.o 

prazor-linux-dtb.o: prazor-linux-dtb.s
	$(ARMCROSS)gcc -c -o prazor-linux-dtb.o $(ARCHFLAGS) prazor-linux-dtb.s

loboot.o:loboot.S
	$(ARMCROSS)gcc -c -o loboot.o $(ARCHFLAGS) loboot.S

dis:loimage prazor-linux.dtb
	$(ARMCROSS)objdump -d loimage > loimage.dis


#DTS=$(LKERNEL)/arch/arm/boot/dts/zynq-parallella1-headless.dts
DTS=simple.dts

DTC=$(LKERNEL)/scripts/dtc/dtc

# Device tree blob:
prazor-linux.dtb: $(DTS)
	$(DTC) -I dts -O dtb -o prazor-linux.dtb $(DTS)

simple-plus-ethernet.dtb: simple-plus-ethernet.dts
	$(DTC) -I dts -O dtb -o simple-plus-ethernet.dtb simple-plus-ethernet.dts


prazor-linux-dtb.s:$(DTS)
	$(DTC) -O asm -o prazor-linux-dtb.s  $(DTS)
	$(DTC) -o prazor-linux-dtb.txt  $(DTS)

clean:
	rm -f loboot.o loimage loimage.dis prazor-linux-dtb* prazor-linux.dtb


# The device tree starts with as 54410000 (TA..) as checked by __vet_atags
# edfe0dde which is what I have got CONFIG_CPU

#;the_top.coreunit_0.core_0:COREISA_IF: Fetch insn a=0xC00081CC (__vet_atags+0x10) ins=e1550006  ..Ua  e1550006  cmp     r5, r6
#core_0:  r4=00000000  r5=EDFE0DD0  r6=EDFE0DD0  r7=006166D0

# useful GDB Commands
# file ~/parallella-sw/parallella-linux/vmlinux
# x/32 0   examine memory
# x/32i    disassemble
# s
# c
# i r   show registers
# x/32b 0xc054c232  - the Booting Linux message - R2 of svnprintf
# break <Addr>  set a break point
# clear <addr>
