/* PC-8801—p‰æ–ÊƒNƒŠƒA */

#include<stdio.h>
#include<stdlib.h>

unsigned char color = 0;

void  cls88(unsigned char color)
{
__asm
	ld	hl, 2
	add	hl, sp
	ld	a, (hl)

	DI

;	XOR A
;	LD BC,#0x035C
	DI
;loop1:
;	OUT (C),A
	LD HL,#0x0C000
loop2:
	LD C,#0x5C
	OUT (C),A
	LD (HL),A
	LD C,#0x5D
	OUT (C),A
	LD (HL),A
	LD C,#0x5E
	OUT (C),A
	LD (HL),A

	INC HL
	CP H
	JR NZ,loop2
;	INC C
;	DJNZ loop1
	OUT (#0x5F),A


	LD C,#0x32
	IN A,(C)
	AND #0xef
	OUT (C),A
	XOR A
	LD HL,#0x0f3c8
loop3:
	LD (HL),A
	INC HL
	CP H
	JR NZ,loop3
	IN A,(C)
	OR #0x10
	OUT (C),A
	EI
__endasm;
}

void term(void)
{
/*__asm
	xor	a
	ld	c,0
	call	0005h
__endasm;*/
}

void main(int argc,char **argv)
{
	if (argc >= 2){
		color = atoi(argv[1]);
	}
	cls88(color);
	term();
}
