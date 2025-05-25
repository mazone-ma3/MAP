/* X1—p‰æ–ÊƒNƒŠƒA “¯‘‚«‚İ */

#include<stdio.h>
#include<stdlib.h>

#define WIDTH 80
#define LINE 200
#define SIZE 80

#define VRAM_MACRO(X,Y) (X + (Y / 8) * WIDTH + (Y & 7) * 0x800)

void setvramio(unsigned char color)
{
__asm
	ld	hl, 2
	add	hl, sp

	ld	bc,#0x1a03
	ld	a,#0x0b
	out	(c),a
	ld	a,#0x0a
	out	(c),a

	ld	bc,0
;	ld	de,#0x3fff
loop:
	ld	a, (hl)
	out	(c),a
	inc	bc
;	dec	de
;	ld	a,d
;	or	e
	ld	a,b
	cp	#0x40
	jr	nz,loop

__endasm;
}

/*void setvramio(void)
{
__asm
	ld	bc,#0x1a02
	in	a,(c)
	or	#0x20
	out	(c),a
	nop
	and	#0xdf
	out	(c),a
__endasm;
}*/

void term(void)
{
/*__asm
	xor	a
	ld	c,0
	call	0005h
__endasm;*/
}

unsigned short adr;
unsigned short i,j;
unsigned char chr, color = 0;
unsigned short vram_ofs;

void main(int argc,char **argv)
{

	if (argc >= 2){
		color = atoi(argv[1]);
	}

__asm
	DI
__endasm;

	setvramio(color);

//	chr = inp(0x1a02);
//	chr = 0;
//	outp(0x1a02, chr | 0x20);
//	outp(0x1a02, chr & 0xdf);
//	outp(0x1a02, chr | 0x00);

//	for(j = 0 ; j < 0x3fff; ++j){
//		outp(0x4000 + adr, color);
//		outp(0x8000 + adr, color);
//		outp(0xc000 + adr, color);
//		++adr;
//	}

//	adr = 0;
//	for(j = 0 ; j < LINE; ++j){
//		adr = VRAM_MACRO(0, j);
//		for(i = 0 ; i < WIDTH; ++i){
//			outp(0x0000 + adr, color);
//			outp(0x4000 + adr, color);
//			outp(0x8000 + adr, color);
//			outp(0xc000 + adr, color);
//			++adr;
//		}
//	}
	chr = inp(0);

	for(j = 0; j < 25; ++j){
		for(i = 0; i < 80; ++i){
			vram_ofs = i + j * SIZE;
			outp(0x3000 + vram_ofs, 0);
			outp(0x2000 + vram_ofs, 0);
		}
	}

__asm
	EI
__endasm;

	term();
}
