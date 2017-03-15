# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

#include "arm-features.h"

.text
.align	0
.global	__aeabi_read_tp
.type	__aeabi_read_tp, %function
__aeabi_read_tp:

#if __ARM_ARCH__ < 6
        mvn     r0, #0xf000
        sub     pc, r0, #31
#else
        mrc     15, 0, r0, cr13, cr0, 3
        bx      lr
#endif

.size	__aeabi_read_tp, . - __aeabi_read_tp
