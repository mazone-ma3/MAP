	.globl _main

	.bank	prog

	.area	.text
	.globl __start

;	org 0x6000

__start:

;	pshs	d,x,y,u

	jsr	_main

;	puls	d,x,y,u
	rts

	.end
; __start

