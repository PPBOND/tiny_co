
/* from glibc-2.1.8 source code ... */

#if defined(__i386__)

/* glibc-2.18\sysdeps\unix\sysv\linux\i386 */

#define oFS 24
#define oEDI 36
#define oESI 40
#define oEBP 44
#define oESP 48
#define oEBX 52
#define oEDX 56
#define oECX 60
#define oEAX 64
#define oEIP 76

#define oFPREGS 96
#define oSIGMASK 108
#define oFPREGSMEM 236

.globl getcontext
.type  getcontext, @function
getcontext:

	/* Load address of the context data structure.  */
	movl	4(%esp), %eax

	/* Return value of getcontext.  EAX is the only register whose
	   value is not preserved.  */
	movl	$0, oEAX(%eax)

	/* Save the 32-bit register values and the return address.  */
	movl	%ecx, oECX(%eax)
	movl	%edx, oEDX(%eax)
	movl	%edi, oEDI(%eax)
	movl	%esi, oESI(%eax)
	movl	%ebp, oEBP(%eax)
	movl	(%esp), %ecx
	movl	%ecx, oEIP(%eax)
	leal	4(%esp), %ecx		/* Exclude the return address.  */
	movl	%ecx, oESP(%eax)
	movl	%ebx, oEBX(%eax)

	/* Save the FS segment register.  We don't touch the GS register
	   since it is used for threads.  */
	xorl	%edx, %edx
	movw	%fs, %dx
	movl	%edx, oFS(%eax)

#ifdef _WITH_FLOAT_SUPPORT
	/* We have separate floating-point register content memory on the
	   stack.  We use the __fpregs_mem block in the context.  Set the
	   links up correctly.  */
	leal	oFPREGSMEM(%eax), %ecx
	movl	%ecx, oFPREGS(%eax)
	/* Save the floating-point context.  */
	fnstenv	(%ecx)
	/* And load it right back since the processor changes the mask.
	   Intel thought this opcode to be used in interrupt handlers which
	   would block all exceptions.  */
	fldenv	(%ecx)
#endif

	/* All done, return 0 for success.  */
	xorl	%eax, %eax
	ret

#elif defined(__x86_64__)

/* glibc-2.18\sysdeps\unix\sysv\linux\x86_64 */

#define oR8 40
#define oR9 48
#define oR12 72
#define oR13 80
#define oR14 88
#define oR15 96
#define oRDI 104
#define oRSI 112
#define oRBP 120
#define oRBX 128
#define oRDX 136
#define oRCX 152
#define oRSP 160
#define oRIP 168

#define oFPREGS 224
#define oSIGMASK 296
#define oFPREGSMEM 424
#define oMXCSR 448

.globl getcontext
.type  getcontext, @function
getcontext:

/* Save the preserved registers, the registers used for passing
	   args, and the return address.  */
	movq	%rbx, oRBX(%rdi)
	movq	%rbp, oRBP(%rdi)
	movq	%r12, oR12(%rdi)
	movq	%r13, oR13(%rdi)
	movq	%r14, oR14(%rdi)
	movq	%r15, oR15(%rdi)

	movq	%rdi, oRDI(%rdi)
	movq	%rsi, oRSI(%rdi)
	movq	%rdx, oRDX(%rdi)
	movq	%rcx, oRCX(%rdi)
	movq	%r8, oR8(%rdi)
	movq	%r9, oR9(%rdi)

	movq	(%rsp), %rcx
	movq	%rcx, oRIP(%rdi)
	leaq	8(%rsp), %rcx		/* Exclude the return address.  */
	movq	%rcx, oRSP(%rdi)

#ifdef _WITH_FLOAT_SUPPORT
	/* We have separate floating-point register content memory on the
	   stack.  We use the __fpregs_mem block in the context.  Set the
	   links up correctly.  */
	leaq	oFPREGSMEM(%rdi), %rcx
	movq	%rcx, oFPREGS(%rdi)
	/* Save the floating-point environment.  */
	fnstenv	(%rcx)
	fldenv	(%rcx)
	stmxcsr oMXCSR(%rdi)
#endif

	/* All done, return 0 for success.  */
	xorl	%eax, %eax
	ret
	
#endif
/* a faster impl is http://www.rethinkdb.com/blog/making-coroutines-fast */

