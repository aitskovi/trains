	.file	"switch.c"
	.section	.rodata
	.align	2
.LC0:
	.ascii	"In kernel_exit\012\015\000"
	.align	2
.LC1:
	.ascii	"kernel_exit activating\012\015\000"
	.align	2
.LC2:
	.ascii	"exiting kernel_exit\012\015\000"
	.text
	.align	2
	.global	kernel_exit
	.type	kernel_exit, %function
kernel_exit:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {r4, r5, sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	ldr	sl, .L4
.L3:
	add	sl, pc, sl
	str	r0, [fp, #-20]
	str	r1, [fp, #-24]
	mov	r0, #1
	ldr	r3, .L4+4
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	mov	r0, #1
	ldr	r3, .L4+8
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)

	stmfd sp!, {r6, r7, r8, r9, sl, fp, ip, lr} @ Store kernel registers

	ldr r0, [fp, #-20] @ Load address of TD
	bl task_get_sp(PLT) @ Get stack pointer from TD
	mov r5, r0 @ Save sp in r5

	ldr r0, [fp, #-20] @ Load address of TD
	bl task_get_spsr(PLT) @ Get spsr from TD
	msr spsr, r0 @ Save spsr

	ldr r0, [fp, #-20] @ Load address of TD
	bl task_get_pc(PLT) @ Get pc from TD
	mov r14, r0

	msr cpsr_c, #31 @ Change to system state

	mov sp, r5 @ Install stack pointer of regular process
	ldmfd sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr} @ Reload user registers

	msr cpsr_c, #19 @ Change to svc state
	movs pc, r14 @ GO!

/*
	mov r1, r14
	mov r0, #1
	bl bwputr(PLT)
*/
.L5:
	.align	2
.L4:
	.word	_GLOBAL_OFFSET_TABLE_-(.L3+8)
	.word	.LC0(GOTOFF)
	.word	.LC1(GOTOFF)
	.word	.LC2(GOTOFF)
	.size	kernel_exit, .-kernel_exit
	.align	2
	.global	kernel_enter
	.type	kernel_enter, %function
kernel_enter:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0

	@ Change to system state
	msr cpsr_c, #31
	@ Save user state
	stmfd sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}

	@ Change to svc state
	msr cpsr_c, #19
	@ Backup scratch registers
	mov r4, r0
	mov r5, r1
	@ Backup PC
	mov r1, lr

    @ Load kernel state
    ldmia sp!, {r6, r7, r8, r9, sl, fp, ip, lr}

    ldr r0, [fp, #-20] @ Load address of TD
					   @ LR already in r1
	bl task_save_pc

	@ Change to system state
	msr cpsr_c, #31
	mov r1, sp
	@ Change to svc state
	msr cpsr_c, #19

	@ Save SP to TD
	ldr r0, [fp, #-20] @ Load address of TD
					   @ SP already in r1
	bl task_save_sp

    @ Save SPSR to TD
    ldr r0, [fp, #-20] @ Load address of TD
	mrs r1, spsr	   @ SPSR already in r1
    bl task_save_spsr

	mov	r0, #1
	ldr	r3, .L4+12
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)

    mov r0, r4
    mov r1, r5

	ldmfd	sp, {r3, r4, r5, sl, fp, sp, pc}

	.size	kernel_enter, .-kernel_enter
	.ident	"GCC: (GNU) 4.0.2"
