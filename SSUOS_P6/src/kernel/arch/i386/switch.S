
.set ARG_CUR, 8
.set ARG_NEXT, 12

.global switch_process

.func switch_process
switch_process:

	pushl %ebp
	movl %esp, %ebp

	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi

.global process_stack_ofs
	mov process_stack_ofs, %edx
	mov process_stack_ofs, %ebx
	addl $4, %ebx

	movl ARG_CUR(%ebp), %eax
	movl %esp, (%eax, %edx, 1)
	
	movl ARG_NEXT(%ebp), %eax
	
	movl (%eax, %edx, 1), %esp
	movl (%eax, %ebx, 1), %ecx
	movl %ecx, %cr3

	popl %edi		
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax

	popl %ebp

	ret

