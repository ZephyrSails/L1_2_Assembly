.text
	.globl go
go:
	pushq	%rbx
	pushq	%rbp
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15

	call _go

	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%rbp
	popq	%rbx
	retq

_go:
	subq $8, %rsp
	movq %r12, 0(%rsp)
	movq $0, %r12

_startLoop:
	cmpq $5, %r12
	jle _loopBody
	jmp _exitLoop

_loopBody:
	movq $5, %rdi
	movq $_testFunction_ret, -8(%rsp)
	subq $8, %rsp
	jmp _testFunction

_testFunction_ret:
	movq %rax, %rdi
	call print
	addq $2, %r12
	jmp _startLoop

_exitLoop:
	movq 0(%rsp), %r12
	addq $8, %rsp
	retq

_testFunction:
	movq %rdi, %rax
	retq
