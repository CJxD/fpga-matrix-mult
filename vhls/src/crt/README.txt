Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>



A quick guide to the C run-time systems.
----------------------------------------

Every image should contain a definition of _isr_routine in case that image uses interrupts.



---

We support two: uClibc and prlibc.  Makefiles should use the latter if env var USE_PRLIBC is 1.

Plan was: Architecture-specific code is either in the archstart.S and archglue.c
file. Typically you need both of these for an arch. They are in the
prlibc folder and used even if using uClibc.

Currently uClibc has a per-arch uclibc_crt.S file however.


The bare metal start procedure is as follows:

 1. archstart.S implements the reset vector and initialised the stack pointer for each core.

 2. archstart calls _argv_main which makes a call to initiliase the C runtime and then finds values of argc and argv to call the application's main().  

When not using djgthreads, _argv_main is implemented in prblic.c when prlibc is being used and in solosbits? when using uClibc.  

When using djgthreads, _argv_main is provided in djgthreads and a flag is needed to turn off _argv_main in prlibc if prlibc is being used.  _argv_main will send core0 on to the application's main function and keep the other cores idle until needed.
   


 3. Some applications are linked with a measuring shim that has a main which paritions off setup arguments at a -- delimiter and then calls the real application's main.  Note this is the second such paritioniing.  The vhls simulator splits off its own args at the first -- delimiter and passes the remainder to the application shim.



We need to provide various low-level resources in all cases.

The _test_and_set function is coded in assembler in the crt.S or in inline C code in the archglue file.


We also have
  _simple_mutex_unlock(X)



The _isr_service function is a separate file so that individual image Makefiles can replace with their own as they need.

===================================================

To compile uClibc:

 $ cd dist
 $ make menuconfig
 $ make CROSS_COMPILE=arm-linux-gnu- V=2

settings are in hidden file .config


You might need to link the linux headers in the dist folder if -iquote seems to be failing to redirect system headers for <linux/limits.h> and so on.
  ln -s ~/parallella-sw/complete-from-git/parallella-linux/include/uapi/linux
  ln -s ~/parallella-sw/complete-from-git/parallella-linux/arch/arm/include/generated/asm 
  ln -s ~/parallella-sw/complete-from-git/parallella-linux/include/uapi/asm-generic/

in the dist/included folder
 ln -s ~/parallella-sw/complete-from-git/parallella-linux/include/linux/

kenel headers: /home/djg11/parallella-sw/linux-xlnx/arch/arm/include

arm-linux-gnu-gcc

Select 'arm' or whatever under the target architecture.
For parallella you need to set little endian option.
Disable the 'shared libries' under the general settings menu - then you will get a nice (large) static libc.a file for simple use.

# END

