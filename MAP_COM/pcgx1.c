/* z88dk X1/turbo/turboZ  PCGキャラ表示実験 ZSDCC版 By m@3 */
/* .com版 スタンダードモードで起動して下さい 
/* キャラを出す  */

#include <stdio.h>
#include <stdlib.h>

#define PARTS_SIZE 0x1800
unsigned char pcg_data[PARTS_SIZE];

#define SIZE 80

FILE *stream[2];

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

void DI(void){
__asm
	DI
__endasm;
}

void EI(void){
__asm
	EI
__endasm;
}

/* (24倍速)PCG設定 試験に出るX1ほぼまんま */
/* データを8バイトx256パターンx3プレーン=6144バイト設定 */
void set_pcg(unsigned char *mainram)
{
__asm
BLUE	EQU	#0x15+1
RED		EQU	#0x16+1
GREEN	EQU	#0x17+1

MULTI	EQU	#8;	1なら3倍速

	ld	hl, 2
	add	hl, sp
	ld	c, (hl)
	inc	hl
	ld	b, (hl)	; bc=mainram
;	push	bc
	ld	l,c
	ld	h,b	; hl = mainram
	ld	(HLWORK),hl

	ld	bc,#0x1800
	ld	a,6
	out	(c),a
	inc	bc
	ld	a,18
	out(c),a

START:
	ld	bc,#0x1FD0
	xor	a
	out	(c),a

	ld	bc,#0x3800+#0x5a0
	ld	hl,#0x260
	ld	d,0
	call	SETIO

	ld	bc,#0x2000+#0x5a0
	ld	hl,#0x260
	ld	d,#0x20
	call	SETIO

;	ld	hl,#0xe200
;	pop	hl
;	ld	(HLWORK),hl
	xor	A

LOOP:
	push	af
	call	SET8
	ld	hl,(HLWORK)
	call	SETPCG
	ld	hl,(HLWORK)
	ld	bc,24*MULTI ;8
	add	hl,bc
	ld	(HLWORK),hl
	pop	af
	add	a,MULTI;8
	jp	nz,LOOP

	ld	bc,#0x1800
	ld	a,6
	out	(c),a
	inc	bc
	ld	a,25
	out	(c),a

;	ld	bc,#0x1FD0
;	ld	a,0x03
;	out	(c),a
	jmp	END

SET8:
	ld	bc,#0x3000+#0x5a0
	ld	d,a
	ld	e,MULTI ;8
SET80:
	push	bc
	ld	hl,48
	call	SETIO
	pop	bc
	ld	hl,80
	add	hl,bc
	ld	b,h
	ld	c,l
	inc	d
	dec	e
	jp	nz,SET80

	ret

SETIO:
	out	(c),d
	inc	bc
	dec	hl
	ld	a,h
	OR	l
	jp	nz,SETIO
	ret

SETPCG:
	ld	b,#0x15+1 ;BLUE
	ld	c,0
	ld	d,#0x16+1 ;RED
	ld	e,#0x17+1 ;GREEN
	ld	a,#0x08*MULTI ;8
	ex	af,af
	exx

	di
	ld	bc,#0x1A01
VDSP0:
	in	a,(c)
	jp	p,VDSP0
VDSP1:
	in	a,(c)
	jp	m,VDSP1

	exx
	ex	af,af

SETP:
	outi
	ld	b,d
	outi
	ld	b,e
	outi

	ld	b,#0x15+1	;BLUE

	ex	af,af
	ld	a,#0x0b
DLY:
	dec	a
	jp	nz,DLY
	ex	af,af

	inc	c
	dec	a
	jp	nz,SETP

	ei
	ret
HLWORK:
	ds	2

;	END
END:
__endasm;
}

short bload(char *loadfil, unsigned char *buffer, unsigned short size)
{
	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
//	printf("Load file %s. Address %x Size %x End %x\n", loadfil, buffer, size, buffer+size);
//	fread( buffer, 1, 4, stream[0]);
	fread( buffer, 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}

unsigned char no;
unsigned short vram_ofs;
unsigned char i, j;

/* BNN X1-Techknowより */

unsigned char hireso[12] = {
	0x6B,0x50,0x59,0x88,0x1B,0x00,0x19,0x1A,0x00,0x0F,0x00,0x00	//80line
//	0x35,0x28,0x2d,0x84,0x1B,0x00,0x19,0x1A,0x00,0x0F,0x00,0x00	//40line
};
unsigned char lowreso[12] = {
	0x6F,0x50,0x59,0x38,0x1F,0x02,0x19,0x1C,0x00,0x07,0x00,0x00	//80line
//	0x37,0x28,0x2d,0x34,0x1F,0x02,0x19,0x1C,0x00,0x07,0x00,0x00	//40line
};

void set_hireso(void)
{
	unsigned char i;
	for(i = 0; i < 12; ++i){
		outp(0x1800,i);
		outp(0x1801,hireso[i]);
	}
}

void set_lowreso(void)
{
	unsigned char i;
	for(i = 0; i < 12; ++i){
		outp(0x1800,i);
		outp(0x1801,lowreso[i]);
	}
}

int	main(int argc,char **argv)
{
	for(j = 0; j < 8; ++j){
		for(i = 0; i < 32; ++i){
			no = i + j * 32;
			vram_ofs = i + j * SIZE;
			outp(0x3000 + vram_ofs, no);
			outp(0x2000 + vram_ofs, 0x27);
		}
	}

	if (argc >= 2){ //argv[1] == NULL){
		if(bload(argv[1], pcg_data, PARTS_SIZE))
			return ERROR;
		outp(0x1a03, 0x0c);	/* 80行 */
		set_lowreso();
		set_pcg(pcg_data);
		if(!(inp(0x1ff0) & 0x01)){
			set_hireso();
			outp(0x1fd0, 0x03);
		}
	}

	return NOERROR;
}

