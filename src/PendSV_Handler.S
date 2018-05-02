	.syntax unified

	.thumb

	.data

	state: .word 0

	.text
	.global PendSV_Handler
	.extern get_next_context
	.thumb_func
PendSV_Handler:

	//FPU Context
	tst lr,0x10
	it eq
	vpusheq {s16-s31}

	push {r4-r11,lr}
   	mrs r0,msp
   	bl get_next_context
   	msr msp, r0
	pop {r4-r11,lr}

	//FPU Context
	tst lr,0x10
	it eq
	vpopeq {s16-s31}

	bx lr
