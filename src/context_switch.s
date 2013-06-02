	.file	"switch.c"
	.text
	.align	2
	.global	kernel_exit
	.type	kernel_exit, %function
kernel_exit:
	mov	ip, sp
	stmfd	sp!, {r4, r5, r6, sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
    mov	r6, r0

	stmfd sp!, {r6, r7, r8, r9, sl, fp, ip, lr} @ Store kernel registers

	mov r0, r6 @ Load address of TD
	bl task_get_sp(PLT) @ Get stack pointer from TD
	mov r5, r0 @ Save sp in r5

	mov r0, r6 @ Load address of TD
	bl task_get_spsr(PLT) @ Get spsr from TD
	msr spsr, r0 @ Save spsr

	mov r0, r6 @ Load address of TD
	bl task_get_return_value(PLT) @ Get return value from TD
	mov r4, r0

	mov r0, r6 @ Load address of TD
	bl task_get_pc(PLT) @ Get pc from TD
	mov r14, r0

	msr cpsr_c, #159 @ Change to system state
	mov sp, r5 @ Install stack pointer of regular process
	mov r0, r4 @ Install return value
	ldmfd sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr} @ Reload user registers

	msr cpsr_c, #147 @ Change to svc state
	movs pc, r14 @ GO!

    .size	kernel_exit, .-kernel_exit
	.align	2
	.global	kernel_enter
	.type	kernel_enter, %function
kernel_enter:
	@ Change to system state
	msr cpsr_c, #159
	@ Save user state
	stmfd sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}

	@ Change to svc state
	msr cpsr_c, #147
	@ Backup scratch registers
	mov r4, r0
	mov r5, r1
	@ Backup PC
	mov r1, lr

    @ Load kernel state
    ldmia sp!, {r6, r7, r8, r9, sl, fp, ip, lr}

    mov r0, r6 @ Load address of TD
					   @ LR already in r1
	bl task_save_pc

	@ Change to system state
	msr cpsr_c, #159
	mov r1, sp
	@ Change to svc state
	msr cpsr_c, #147

	@ Save SP to TD
	mov r0, r6         @ Load address of TD
					   @ SP already in r1
	bl task_save_sp

    @ Save SPSR to TD
    mov r0, r6         @ Load address of TD
	mrs r1, spsr	   @ SPSR already in r1
    bl task_save_spsr

    mov r0, r4
    mov r1, r5

	ldmfd	sp, {r3, r4, r5, r6, sl, fp, sp, pc}

	.size	kernel_enter, .-kernel_enter
    .align	2
	.global	irq_enter
	.type	irq_enter, %function
irq_enter:
    @ Change to system state
	msr cpsr_c, #159
    
	@ Save scratch registers.
	stmfd sp!, {r0, r1, r2, r3}

    @ Change to IRQ Mode
    msr cpsr_c, #146

    @ Grab PC/SPSR
    mov r0, lr
    mrs r1, spsr 

    @ Switch to System Mode
    msr cpsr_c, #159

    @ Save LR/SPSR
    stmfd sp!, {r0, r1}

    @ Into Supervisor Mode!
    msr cpsr_c, #147

    @ Insert special value into r0
    mov r0, #0

    @ Enter the Kernel
    bl kernel_enter

    @ !!! BACK FROM THE KERNEL !!!

    @ Switch to System Mode.
    msr cpsr_c, #159

    @ Unload LR/SPSR
    ldmfd sp!, {r0, r1}

    @ Back to Interrupt Mode
    msr cpsr_c, #146

    @ Install LR/SPSR
    mov lr, r0
    msr spsr, r1

    @ Switch to System Mode
    msr cpsr_c, #159

    @ Reload scratch register state
    ldmfd sp!, {r0, r1, r2, r3}

    @Switch to Interrupt Mode
    msr cpsr_c, #146

    @Back to Work!
    subs pc, lr, #4

	.size	irq_enter, .-irq_enter
	.ident	"GCC: (GNU) 4.0.2"
