/* PC-8801—p‰æ–ÊƒNƒŠƒA ALU */

#include<stdio.h>
#include<stdlib.h>

unsigned char color = 0;

void  cls88(unsigned char color)
{
__asm
	ld	hl, 2
	add	hl, sp
	ld	a, (hl)
	ld	b,a

	DI

;	XOR A
;	LD BC,#0x035C
;	DI

	in	a,(#0x32)
	or	#0x40
	out	(#0x32),a	; USE ALU
	ld	a,#0x80
	out	(#0x35),a	; Access GVRAM

	xor	a
;	ld	a,#0x07
	out	(#0x34),a

	ld	a, b
;	xor	#0xff
	cpl

;loop1:
;	OUT (C),A
	LD HL,#0x0C000
loop2:
;	LD C,#0x5C
;	OUT (C),A
	LD (HL),A
;	LD C,#0x5D
;	OUT (C),A
;	LD (HL),A
;	LD C,#0x5E
;	OUT (C),A
;	LD (HL),A

	INC HL
	CP H
	JR NZ,loop2
;	INC C
;	DJNZ loop1
;	OUT (#0x5F),A

	xor a
	out(#0x35),a	; Access MAINRAM
	in	a,(#0x32);
	and #0xbf;
	out(#0x32),a	; NOUSE ALU


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
