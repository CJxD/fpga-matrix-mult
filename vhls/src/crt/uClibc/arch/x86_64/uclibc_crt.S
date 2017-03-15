	.space 256
	.global _start
	.global _isr_routine
	.global	_init
	.global	_fini

	/* Stick in a dummy reference to main(), so that if an application
	 * is linking when the main() function is in a static library (.a)
	 * we can be sure that main() actually gets linked in */
	.type   main,%function

_start:
	nop
	jmp _real_start

	.space 0x500 - 256 - 6

_int_vector: # This should be at address 0x500
	# save registers
	call _isr_routine
	nop
	# restore registers
	ret # return from exception

_init:
	ret

_fini:	
	ret

_real_start:
	movq 	$(_stack_top), %rsp
	movq    $0xffffffffffffff10, %rax
	movq    (%rax), %rax	 # Make gaps in stack for each new processor core.
	andq    $255, %rax
	shlq    $14, %rax
	subq    %rax, %rsp
	
	/* Align the stack to a 16 byte boundary to follow the ABI. */
	andq $~15, %rsp

	movq 	%rsp, %rbp
	
	call	uClibc_crt0

	jmp _end

_stdio_init:
	ret


	.global _core_pause
_core_pause:
	pause
	ret
	

	.global _mystack
	.global _simple_mutex_lock
	.global _simple_mutex_unlock

_mystack:
	mov	%rsp,%rax
	ret

unused:	
	mfence
	sfence
	lfence	

mlock1:
	pause
_simple_mutex_lock:
	movq    $0,%rax
	movq    $1,%rcx	
	lock
	cmpxchg %rcx,(%rdi) # if dest==ax then dest:=src and zf set.
	jnz mlock1
	ret

_simple_mutex_unlock:	
	mov     $-1,%rax
	lock
	xadd    %rax,(%rdi)
	ret	
	
_end:
	nop 0
	jmp 0                 # ISS treats a pc of 0 as a request to exit the simulation.
	
	.section .stack       # This overlays at 0 and goes up to the heap base, so stack(s) will be just below the heap start.
	.space 0x100000
_stack_top:
