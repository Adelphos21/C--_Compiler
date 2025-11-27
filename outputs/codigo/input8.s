.data
print_fmt: .string "%ld \n"
.text
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $48, %rsp
 movq $2, %rax
 imulq $8, %rax
 leaq -8(%rbp), %rcx
 subq %rax, %rcx
 pushq %rcx
 movq $10, %rax
 popq %rcx
 movq %rax, (%rcx)
 movq $2, %rax
 imulq $8, %rax
 leaq -8(%rbp), %rcx
 subq %rax, %rcx
 movq (%rcx), %rax
 movq %rax, -48(%rbp)
 movq -48(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq $0, %rax
 jmp .end_main
.end_main:
leave
ret
.section .note.GNU-stack,"",@progbits
