	.syntax unified


	.text
	.global PendSV_Handler
	.extern get_next_context
	.thumb_func
PendSV_Handler:
	push {r4-r11,lr}
   	mrs r0,msp
   	bl get_next_context
   	msr msp, r0
	pop {r4-r11,lr}
	bx lr
