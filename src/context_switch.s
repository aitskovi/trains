	.file	"switch.c"
	.text
	.align	2
	.global	kernel_exit
	.type	kernel_exit, %function
kernel_exit:
	stmfd sp!, {r0, r4, r5, r6, r7, r8, r9, sl, fp, lr} @ Store kernel registers

    ldr r1, [r0, #4] @ Load SP from Task

	msr cpsr_c, #159 @ Change to system state
	mov sp, r1 @ Install stack pointer of regular process

	ldmfd sp!, {r0, r1, r2, r4, r5, r6, r7, r8, r9, sl, fp, lr} @ Reload user registers

	msr cpsr_c, #147 @ Change to svc state
                 @ Return Value is already installed in R0

    msr spsr, r1 @ SPSR is in R1
	movs pc, r2 @ GO!(Old PC was in R2)

    .size	kernel_exit, .-kernel_exit
	.align	2
	.global	kernel_enter
	.type	kernel_enter, %function
kernel_enter:

    @ Grab the Kernel data about this task
    mrs r1, spsr
    mov r2, lr

	@ Change to system state
	msr cpsr_c, #159

	@ Save user state
	stmfd sp!, {r0, r1, r2, r4, r5, r6, r7, r8, r9, sl, fp, lr}

    @ Grab the user's sp.
    mov r1, sp

	@ Change to svc state
	msr cpsr_c, #147

    @ Backup Request
    mov ip, r0

    @ Load kernel state
    ldmia sp!, {r0, r4, r5, r6, r7, r8, r9, sl, fp, lr}

    @ Store user's sp
    str r1, [r0, #4]

    @ Install Request Param
    mov r0, ip

    @ Into the Kernel.
    mov pc, lr

	.size	kernel_enter, .-kernel_enter
    .align	2
	.global	irq_enter
	.type	irq_enter, %function
irq_enter:
    @ Change to system state
	msr cpsr_c, #159
    
	@ Save scratch registers.
	stmfd sp!, {r0, r1, r2, r3, ip}

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

    @ Make our spsr the correct one.
    msr spsr_c, #146

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
    ldmfd sp!, {r0, r1, r2, r3, ip}

    @Switch to Interrupt Mode
    msr cpsr_c, #146

    @Back to Work!
    subs pc, lr, #4

	.size	irq_enter, .-irq_enter
	.ident	"GCC: (GNU) 4.0.2"
