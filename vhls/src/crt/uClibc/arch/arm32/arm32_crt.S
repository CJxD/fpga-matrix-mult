	 
	# $Id: mycrt.S,v 1.2 2011-02-08 11:28:03 djg11 Exp $ 
	# CBG C run-time system
	
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
	.global _start
	.global _core_pause	
	.extern _isr_routine
	.extern _argv_main


_start:
	b real_start

# Table of ARM hardwired PC values
	
#        ResetVector         = 0000000,      // Reset Supervisor nominal - we use the value from COREISA_IF instead.
#        UndefinedVector     = 0x00000004,   // Undefined instruction Undefined
#        SwiVector           = 0x00000008,   // Software interrupt Supervisor
#        PrefetchVector      = 0x0000000C,   // Abort (prefetch) Abort
#        DataVector          = 0x00000010,   // Abort (data) Abort
#        ReservedVector      = 0x00000014,   // Reserved Reserved
#        IrqVector           = 0x00000018,   // IRQ IRQ
#        FiqVector           = 0x0000001C    // FIQ FIQ

	b UndefinedVector
	b SwiVector
	b PrefetchVector
	b DataVector
	b ReservedVector
	b IrqVector
	b FiqVector	
	nop


UndefinedVector:	
SwiVector:	
PrefetchVector:	
DataVector:	
ReservedVector:	
FiqVector:	
@ All of these other interrupts are unimplemented so just hang (or call a simulator exit backdoor).
	b hardware_hang
	

IrqVector:	
	mov	r13,#0x40000  @ IRQ stack base
	push  {r0-r12, r14} @ save registers : r13-r15 are banked so do not need to be saved, but r14 is corrupted by calling _isr_routine!
	bl _isr_routine
	pop   {r0-r12, r14} @ restore registers
	subs pc, lr, #4 @ return from exception - pre cortex.

	
real_start:
 	mov    r3,#0xffffff14  @ See which core I am by reading processor ID from (backdoor) register.
 	ldr    r3,[r3]
	lsls   r3,r3,#16    @Allow this amount of stack space per core, eg 2**16 bytes

        mov  r1,#0x3c0000 @ (_stack) @ First stack (non irq)

	add   r13,r3,r1  	@ Bump stack base according to core number


	@ enable interrupts
	MRS     r1, CPSR       @ Read the status register.
	BIC     r1, r1, #0x80  @ Clear the I bit
	MSR     CPSR_c, r1     @ Write it back to re-enable
	                            
	
	bl   _argv_main      @ Call the argv/argc shim around main that supplies these.

	
hardware_hang:	b hardware_hang


# This should execute a low power pipeline stall instruction for the body of tight spinlocks.	
_core_pause:
	yield
@	mov r0,#10 		  @ Pause for 10 microseconds
@	nop @ 15
	bx lr
	nop

	.global _kill_sim

# A nop number 1
_kill_sim:
	swi 100001
	bl _core_pause
	b _kill_sim
	
	
# Return my stack pointer.
	.global _mystack
_mystack:
	mov    r0,r15
	bx   lr

	.global _simple_mutex_lock
	.global _simple_mutex_unlock

@ Mutex lock - poll waiting for the lock to be clear, then do a strex of a 1, which if successful (r2 is zero) has gained the lock.
@	cmpxchg %rcx,(%rdi) # if dest==ax then dest:=src and zf set.
mlock1:
	yield
_simple_mutex_lock:
	mov r1,#1
	ldrex r2,[r0]
	cmp r2,#0
	bne mlock1
	strex r2,r1,[r0] @ The first operand is set to zero on success
	cmp r2,#0
	bne mlock1
	bx lr
	
_simple_mutex_unlock:	
	mov r1,#0
	str r1, [r0]  @ A simple clear of the mutex should be sufficient - do not need strex I think.
	bx lr


@ _archglue_test_and_set(prlibc_mutex_t *addr, int wdata)
@ It is actually a test and store, with wdata being in second arg, r1.
@ test and set is used by an external loop that generally iterates until success.
@ here we only iterate on the inner loop that tries until strex suceeds and then returns the old value of the mutex/lock.	
	.global _archglue_test_and_set

_archglue_test_and_set1:
	yield
_archglue_test_and_set:	
	ldrex r3,[r0]
	strex r2,r1,[r0] @ The first operand is set to zero on success
	cmp r2,#0
	bne _archglue_test_and_set1
	mov r0, r3 @ Return old value of the mutex/lock.
	bx lr

#endif
	

# The stack for thread0/cpu0 works down from this point, but other stacks will be higher up: must not hit start of heap.
        .space 0x40000
_stack:
