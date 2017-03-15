# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>


		 
	# 
	# CBG verysimple.S simple run-time system
	
# Files with suffix capital S are hand-written assembly files.
#
#	The crt.S file is the C run time system.  Minimally, it must set up a
#	stack pointer and branch to the user's main function.
#
#	It should be placed in memory at the point where the processor will
#	start execution after hardware reset.  To achieve this, it should
#	generally be listed first on the link editor command line.
#
#	When interrrupts are used, like reset, these have a hardwired starting
#	address.  The interrupt handler entry points may be placed in crt.S or
#	in separate files.
#
#	When there are multiple processor cores, they will each need their own
#	stack.  This can be achieved with one of three methods:
#	 1.  separate physical memories mapped into stack space for each core,
#	 2.  address munging logic so that different regions of the same
#	memory are used at the same logical address
#	 3. by making the crt.S file read a processor core number register
#	and bumping its stack base in proportion to the value read.  N.B. most
#	stacks work downwards so the stack base is at the top (highest)
#	logical address in the stack region.


	.text
	.space 256
	
	.global _start
	.global _core_pause	
	.extern _isr_routine
	.extern _argv_main


_start: 	# This should be at address 0x100 - the reset start point.
	l.j real_start
	l.nop

	.space 0x500 - 256 - 4

int_vector:	# This should be at address 0x500
	# save registers

	l.jal _isr_routine
	l.nop
	# restore register
	l.rfe # return from exception

	
real_start:
 	l.addi  r3,r0,0xffffff14  # See which core I am by reading processor ID from (backdoor) register.
 	l.lwz   r3,0x0(r3)
	l.slli  r3,r3,16    # Allow this amount of stack space per core, eg 2**14 bytes

        l.movhi r1,hi(_stack)
        l.ori   r1,r1,lo(_stack)
	l.add   r1,r3,r1  	# Bump stack base according to core number

	l.jal   _argv_main      # Call the argv/argc shim around main that supplies these.
	l.nop
	
hang:	l.j hang
	l.nop

# This should execute a low power pipeline stall instruction for the body of tight spinlocks.	
_core_pause:
	l.addi r3,r0,10 		# Pause for 10 microseconds
	l.nop 15
	l.jr r9
	l.nop

	.global _kill_sim

# A nop number 1
_kill_sim:
	l.nop 1	
	l.jal _core_pause
	l.j _kill_sim
	
	
# Return my stack pointer.
	.global _mystack
_mystack:
	l.jr   r9
	l.addi r11,r1,0
	
        .section .stack
# The stack for thread0/cpu0 works down from this point, but other stacks will be higher up: must not hit start of heap.
        .space 0x40000
_stack:
