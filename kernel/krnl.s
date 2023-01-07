
	.text
	.code 32

	.global vectors_start
	.global vectors_end
	.global gTasks
	.global gTaskSize
	.global TaskSwitch, Scheduler, gpRunning
	.global EnableInterrupt, DisableInterrupt, LockIRQ, UnlockIRQ
	.global GetCPSR, GetSPSR, GetSP
	.global gSwitchFlag, gIntNest
	.set vectorAddr, 0x10140030

reset_handler:
	// set SVC stack in order to call copy_vector()
	ldr r0, =gTasks
	ldr r1, =gTaskSize
	ldr r2, [r1, #0]
	add r0, r0, r2
	mov sp, r0
	// copy vector table to address 0
	bl copy_vectors

	// go in IRQ mode, set IRQ stack
	msr cpsr, #0x12
	ldr sp, =irq_stack_top	

	// call main() in SVC mode with IRQ on: all tasks run in SVC mode
	msr cpsr, #0x13
	bl main
	b .

	.align 4

irq_handler:	
	sub	lr, lr, #4
	stmfd sp!, {r0-r12, lr}
	mrs r0, spsr
	stmfd sp!, {r0} // push SPSR

	ldr r0, =gIntNest // r0->intnest
	ldr r1, [r0]
	add r1, #1
	str r1, [r0] // intnest++

	mov r1, sp // get irq sp into r1
	ldr sp, =irq_stack_top // reset IRQ stack poiner to IRQ stack top

	// switch to SVC mode // to allow nested IRQs: must clear interrupt source
	msr cpsr, #0x93 // to SVC mode with interrupts OFF

	sub sp, #60 // dec SVC mode sp by 14 entries
	mov r0, sp // r0=SVC stack top
	// copy IRQ stack to SVC stack
	mov r3, #15 // 15 times

isan:	
	ldr r2, [r1], #4 // get an entry from IRQ stack
	str r2, [r0], #4 // write to proc's kstack
	sub r3, #1 // copy 14 entries from IRQ stack to PROC's kstack
	cmp r3, #0
	bne isan

	// read vectoraddress register: MUST!!! else no interrupts
	ldr  r1, =vectorAddr
	ldr  r0, [r1] // read vectorAddr register to ACK interrupt

	stmfd sp!, {r0-r3, lr}

	msr cpsr, #0x13 // still in SVC mode but enable IRQ
	//  msr cpsr, #0x93 // still in SVC mode but enable IRQ

	bl irq_chandler // handle interrupt in SVC mode, IRQ off

	msr cpsr, #0x93
	ldmfd sp!, {r0-r3, lr}

	ldr r0, =gIntNest
	ldr r1, [r0]
	sub r1, #1
	str r1, [r0] // intnest--
	cmp r1, #0 // if intnest != 0 => NoSwitch
	bne NoSwitch

	// intnest==0: END OF IRQs: if swflag=1: switch task 	
	ldr r0, =gSwitchFlag
	ldr r0, [r0]
	cmp r0, #0
	bne DoSwitch // if swflag=0: no task switch

NoSwitch:	
	ldmfd sp!, {r0}
	msr   spsr, r0 // restore SPSR

	// irq_chandler() already issued EOI

	ldmfd sp!, {r0-r12, pc}^ // return via SVC stack

DoSwitch: // still in IRQ mode
	// bl endIRQ // show "at end of IRQs"

	// will switch task, so must issue EOI
	ldr  r1, =vectorAddr
	str  r0, [r1] // issue EOI  

	bl TaskSwitch // call tswitch(): resume to here

	// will switch task, so must issue EOI
	//  ldr  r1, =vectorAddr
	//  str  r0, [r1] // issue EOI  

	ldmfd sp!, {r0}
	msr   spsr, r0

	ldmfd sp!, {r0-r12, pc}^ // return via SVC stack

TaskSwitch:
	//       1  2  3  4  5  6  7  8  9  10  11  12  13 14
	//       ---------------------------------------------
	// stack=r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr
	//       ---------------------------------------------
	// Disable IRQ interrupts.
	mrs r0, cpsr
	orr r0, r0, #0x80 // set bit to mask out IRQ interrupts.
	msr cpsr, r0

	stmfd sp!, {r0-r12, lr}

	ldr r0, =gpRunning // r0 = &gpRunning;
	ldr r1, [r0, #0] // r1->running task.
	str sp, [r1, #4] // running->ksp = sp.

	bl Scheduler

	ldr r0, =gpRunning
	ldr r1, [r0, #0] // r1->running task.
	ldr sp, [r1, #4]

	// Enable IRQ interrupts.
	mrs r0, cpsr
	bic r0, r0, #0x80 // Clear bit means unmask IRQ interrupt.
	msr cpsr, r0

	ldmfd sp!, {r0-r12, pc}
	
DisableInterrupt: // size_t sr = DisableInterrupt(); Return old cpsr.
	mrs r4, cpsr
	mov r0, r4
	orr r4, r4, #0x80 // Set bit to mask off IRQ interrupts.
	msr cpsr, r4
	mov pc, lr	

EnableInterrupt: // EnableInterrupt(size_t sr)
	msr cpsr, r0
	mov pc, lr	

LockIRQ:
	mrs r0, cpsr
	orr r0, r0, #0x80 // Set bit means mask off IRQ interrupt.
	msr cpsr, r0
	mov pc, lr	

UnlockIRQ:	
	mrs r0, cpsr
	bic r0, r0, #0x80 // Clear bit means unmask IRQ interrupt.
	msr cpsr, r0
	mov pc, lr	

GetCPSR:
	mrs r0, cpsr
	mov pc, lr

GetSPSR:
	mrs r0, spsr
	mov pc, lr

GetSP:
	mov r0, sp
	mov pc, lr
	
vectors_start:
	ldr pc, reset_handler_addr
	ldr pc, undef_handler_addr
	ldr pc, swi_handler_addr
	ldr pc, prefetch_abort_handler_addr
	ldr pc, data_abort_handler_addr
	b .
	ldr pc, irq_handler_addr
	ldr pc, fiq_handler_addr

	reset_handler_addr:          .word reset_handler
	undef_handler_addr:          .word undef_handler
	swi_handler_addr:            .word swi_handler
	prefetch_abort_handler_addr: .word prefetch_abort_handler
	data_abort_handler_addr:     .word data_abort_handler
	irq_handler_addr:            .word irq_handler
	fiq_handler_addr:            .word fiq_handler

vectors_end:
	
	.end
