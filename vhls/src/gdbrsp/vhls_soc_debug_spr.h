// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

#ifndef VHLS_SOC_DEBUG_SPR_H
#define VHLS_SOC_DEBUG_SPR_H

//! For ARM and other architectures we use the same offsets as the OR1K to map registers
// into a debug address space.
#define VHLS_SPR_SR   17  
#define VHLS_SPR_PC   16 /* of course also GPR 15 */
#define VHLS_SPR_GPR0 0x400
#define VHLS_MAX_SPRS 65536

#endif
