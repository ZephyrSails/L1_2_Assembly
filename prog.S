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
	movq $5, %rdi
	movq $5, %rsi
	call allocate
	movq %rax, %r12
	movq $85, 8(%r12)
	movq 8(%r12), %rdi
	call print
	movq 16(%r12), %rdi
	call print
	movq %r12, %rdi
	movq $1, %rsi
	movq $_printData_ret, -8(%rsp)
	subq $8, %rsp
	jmp _printData

_printData_ret:
	movq %r12, %rdi
	movq $3, %rsi
	movq $_printData2_ret, -8(%rsp)
	subq $8, %rsp
	jmp _printData

_printData2_ret:
	movq %r12, %rdi
	movq $5, %rsi
	movq $_printData3_ret, -8(%rsp)
	subq $8, %rsp
	jmp _printData

_printData3_ret:
	movq 0(%rsp), %r12
	addq $8, %rsp
	retq

_printData:
	subq $32, %rsp
	movq %r12, 0(%rsp)
	movq %r13, 8(%rsp)
	movq %r14, 16(%rsp)
	movq %r15, 24(%rsp)
	movq %rdi, %r12
	movq %rsi, %r13
	movq %r12, %rdi
	call print
	movq %r13, %rdi
	call print
	movq 0(%r12), %r14
	salq $1, %r14
	addq $1, %r14
	movq %r14, %rdi
	call print
	cmpq %r14, %r13
	jl _correct
	jmp _wrong

_wrong:
	movq %r12, %rdi
	movq %r13, %rsi
	call w

_correct:
	movq %r13, %r15
	sarq $1, %r15
	imulq $8, %r15
	addq $8, %r15
	salq $1, %r15
	addq $1, %r15
	movq %r15, %rdi
	call print
	movq %r15, %rdx
	sarq $1, %rdx
	addq %r12, %rdx
	movq 0(%rdx), %rdi
	call print

_exitFunction:
	movq 0(%rsp), %r12
	movq 8(%rsp), %r13
	movq 16(%rsp), %r14
	movq 24(%rsp), %r15
	addq $32, %rsp
	retq
