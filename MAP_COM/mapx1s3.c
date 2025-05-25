/* z88dk X1/turbo/turboZ  PCG�L����MAP�ړ����� 2x2���k�W�J ZSDCC�� By m@3 */
/* .com�� �X�^���_�[�h���[�h�ŋN�����ĉ����� 
/* �L�������o��  */

#include <stdio.h>
#include <stdlib.h>

//#define MAP_ADR 0x8000
//#define PARTS_DATA (MAP_ADR+0x3c00)
//#define CHRPAT_ADR 0xC000

#define PARTS_SIZE 0x1800
#define CHR_SIZE 0xf00

unsigned short y_table[200];

unsigned char vram_data[CHR_SIZE];
//#define VRAM_DATA_ADR vram_data

#define PARTS_HEAD 0x3c00 /*�g�ݍ��킹�L�����f�[�^�̐擪�Ԓn*/
#define BUFFSIZE 16384
unsigned char mapdata[BUFFSIZE];

FILE *stream[2];

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0


#define SIZE 80

#define X_SIZE 18
#define Y_SIZE 18

#define PARTS_X 2
#define PARTS_Y 8

#define MAP_SIZE_X 128
#define MAP_SIZE_Y 128

#define OFS_X 2
#define OFS_Y 2

#define CHR_X 8
#define CHR_Y 8

unsigned short vram_ofs;
unsigned char map_data[(X_SIZE+2) * 32];
//unsigned char *WK1FD0 = (unsigned char *)0xf8d6;

#define MAXCOLOR 8

/* BRG */

unsigned char org_pal[MAXCOLOR][3] =
	{{ 0, 0, 0 },
	{ 0, 0, 15 },
	{ 15, 0, 0 },
	{ 0, 0, 0 },//	{ 15, 0, 15 },
	{ 0, 15, 0 },
	{ 0, 15, 15 },
	{ 15, 15, 0 },
	{ 15, 15, 15 },};

unsigned char org_pal2[MAXCOLOR][3] =
	{{ 0, 0, 0 },
	{ 0, 0, 15 },
	{ 15, 0, 0 },
	{ 15, 0, 15 },
	{ 0, 15, 0 },
	{ 0, 15, 15 },
	{ 15, 15, 0 },
	{ 15, 15, 15 },};

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

/* (24�{��)PCG�ݒ� �����ɏo��X1�قڂ܂�� */
/* �f�[�^��8�o�C�gx256�p�^�[��x3�v���[��=6144�o�C�g�ݒ� */
void set_pcg(unsigned char *mainram)
{
__asm
BLUE	EQU	#0x15+1
RED		EQU	#0x16+1
GREEN	EQU	#0x17+1

MULTI	EQU	#8 ;1����3�{��

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

/*�}�b�v�f�[�^���t�@�C������z��ɓǂݍ���*/
short mapload(char *loadfil)
{
//	short inhandle;

//	if ((inhandle = open( loadfil, O_RDONLY )) == -1) { //| O_BINARY
	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
//	printf("Load file %s. Address %x Size %x End %x\n", loadfil, mapdata, BUFFSIZE, mapdata+BUFFSIZE);
//	read( inhandle, mapdata, 7);
	fread( mapdata, 1, 7, stream[0]);
//	read( inhandle, mapdata, BUFFSIZE);
	fread( mapdata, 1, BUFFSIZE, stream[0]);
//	close(inhandle);
	fclose(stream[0]);
	return NOERROR;
}

short bload(char *loadfil, unsigned char *buffer, unsigned short size)
{
	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
//	printf("Load file %s. Address %x Size %x End %x\n", loadfil, buffer, size, buffer+size);
//	Set_RAM_MODE();
//	fread( buffer, 1, 4, stream[0]);
	fread( buffer, 1, size, stream[0]);
//	Set_ROM_MODE();
	fclose(stream[0]);
	return NOERROR;
}

void set_key(void)
{
__asm
	ld	d,0xe4
	call	SEND1
	call	CANW
	ld	d,0
	call	SEND1
	call	CANW
	ld	d,0
	call	SEND1
	call	CANW

__endasm;
}

/* �����ɏo��X1�����p */
void get_key(unsigned char *data, unsigned short num)
{
__asm
	ld	hl, #2
	add	hl, sp
	ld	e,(hl)
	inc	hl
	ld	d,(hl)	; de = data

	inc	hl
	ld	c, (hl)
	inc	hl
	ld	b, (hl)	; bc = num


FM49:	ei
	ex	de,hl
	ld	d,(hl)
	inc	hl
	ld	e,c
	call	SEND1
	call	CANW
	di
	dec	e

FM49LP:	call	GET1
	ld	(hl),d
	inc	hl
	dec	e
	jr	nz,FM49LP
	ei
	ret

SEND1:	call	CANW
	ld	bc,#0x1900
	out	(c),d
	ret

GET1:	call	CANR
	ld	bc,#0x1900
	in	d,(c)
	ret

CANW:	ld	bc,#0x1a01
CANWLP:	in	a,(c)
	and	#0x40
	jr	nz,CANWLP
	ret

CANR:	ld	bc,#0x1a01
CANRLP:	in	a,(c)
	and	#0x20
	jr	nz,CANRLP
	ret
__endasm;
}

short pat_tmp;
unsigned char no;

unsigned char chr_tbl[8][4] = {
		{0, 1, 0 + 16, 1 + 16},
		{2, 3, 2 + 16, 3 + 16},
		{4, 5, 4 + 16, 5 + 16},
		{6, 7, 6 + 16, 7 + 16},
		{8, 9, 8 + 16, 9 + 16},
		{10, 11, 10 + 16, 11 + 16},
		{12, 13, 12 + 16, 13 + 16},
		{14, 15, 14 + 16, 15 + 16},
};
unsigned char dir = 2, dir2 = 0, i, j;

//#define VRAM_MACRO(X,Y) (X + (Y / 8) * 80 + (Y & 7) * 0x800)
//#define VRAM_MACRO(X,Y) (X + y_table[Y])
//unsigned char data2[3*2*8]; //[3][2][8];

/* 3�v���[���]�� */
/* 0x4000,0x8000,0xc000 */
unsigned char adr_tmp2_x, adr_tmp2_y;

void put_chrx1_pat(unsigned short patadr) __sdcccall(1)
{
//	unsigned short adr_tmp;
//	unsigned char *adr_tmp, *adr_tmp3 = data2;
//	unsigned short ii,jj;
/*
//	adr_tmp = CHRPAT_ADR + patadr * 2 + patadr;
	adr_tmp = &vram_data[patadr * 2 + patadr];
	for(jj = 0 ; jj < PARTS_Y; ++jj){*/
//		DI();
//		outp(0x1fd0, *WK1FD0 | 0x10); /* BANK1 */
//		outp(0x1fd0, 0x10); /* BANK1 */
/*		for(ii = 0 ; ii < PARTS_X; ++ii){
//			data2[0][ii][jj] = inp(adr_tmp++);
//			data2[1][ii][jj] = inp(adr_tmp++);
//			data2[2][ii][jj] = inp(adr_tmp++);
//			data2[0][ii][jj] = (*adr_tmp++);
//			data2[1][ii][jj] = (*adr_tmp++);
//			data2[2][ii][jj] = (*adr_tmp++);
			(*adr_tmp3++) = (*adr_tmp++);
			(*adr_tmp3++) = (*adr_tmp++);
			(*adr_tmp3++) = (*adr_tmp++);
		}
		adr_tmp += (32 * 3 - PARTS_X * 3);*/
//		outp(0x1fd0, *WK1FD0); /* ���ɖ߂� */
//		outp(0x1fd0, 0); /* ���ɖ߂� */
//		EI();
//	}
__asm
	ld	c,l
	ld	b,h
	add	hl,bc
	add	hl,bc
	ld	bc,_vram_data;
	add	hl,bc
	push	hl
__endasm;
/*
	ld	de,_data2
	ld	c,PARTS_Y
patloop1:
	ld	b,PARTS_X
patloop2:
	push	bc
	ldi
	ldi
	ldi
	pop	bc
	djnz patloop2
	push	de
	ld	de,32*3-PARTS_X*3
	add	hl,de
	pop	de
	dec	c
	jr	nz,patloop1
__endasm;
*/
//	adr_tmp2 = VRAM_MACRO(((OFS_X - 1 + i) * PARTS_X), ((OFS_Y - 1 + j) * PARTS_Y));
	adr_tmp2_x = ((OFS_X - 1 + i) * PARTS_X);
	adr_tmp2_y = ((OFS_Y - 1 + j) * PARTS_Y);

/*	adr_tmp3 = data2;
	for(jj = 0 ; jj < PARTS_Y; ++jj){
		adr_tmp2 = VRAM_MACRO(((OFS_X - 1 + i) * PARTS_X), ((OFS_Y - 1 + j) * PARTS_Y + jj));
		for(ii = 0 ; ii < PARTS_X; ++ii){
//			outp(0x4000 + adr_tmp2, data2[0][ii][jj]);
//			outp(0x8000 + adr_tmp2, data2[1][ii][jj]);
//			outp(0xc000 + adr_tmp2, data2[2][ii][jj]);
			outp(0x4000 + adr_tmp2, (*adr_tmp3++));
			outp(0x8000 + adr_tmp2, (*adr_tmp3++));
			outp(0xc000 + adr_tmp2, (*adr_tmp3++));
			++adr_tmp2;
		}
	}*/
__asm
;	ld	de,_data2
	pop	de
	ld	c,0 ;PARTS_Y
patloop3:
	push	bc
	ld	a,(_adr_tmp2_y)
	add	a,c
	ld	c,a
	ld	b,0
	ld	hl,_y_table
	add	hl,bc
	add	hl,bc
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	ld	a,(_adr_tmp2_x)
	ld	l,a
	ld	h,0
	add	hl,bc
	pop	bc
	ld	b,PARTS_X
patloop4:
	push	bc
	push	hl
	ld	c,l
	ld	b,h
	ld	hl,0x4000
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	a,(de)
	inc	de
	out (c),a
	ld	hl,0x4000
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	a,(de)
	inc	de
	out (c),a
	ld	hl,0x4000
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	a,(de)
	inc	de
	out (c),a
	pop	hl
	inc	hl
	pop	bc
	djnz patloop4

	ld	hl,32*3-PARTS_X*3
	add	hl,de
	ex	de,hl

	inc	c
	ld	a,c
	cp	PARTS_Y
	jr	nz,patloop3
__endasm;
}

void pat_sub(void)
{
//	outp(0x3000 + vram_ofs, no * 2);
//	outp(0x3000 + vram_ofs+1, no * 2 +1);
__asm
	ld	bc,(_vram_ofs)
	ld	a,(_no)
	sla	a
	out (c),a
	inc	bc
	inc	a
	out	(c),a
__endasm;
}

void pat_sub2(void)
{
	outp(0x2000 + vram_ofs, 0x27);
	outp(0x2000 + vram_ofs+1, 0x27);
}

void chr_sub(void)
{
	unsigned char no2 = chr_tbl[dir * 2 + dir2][(i - CHR_X) + (j - CHR_Y) * 2];

	put_chrx1_pat((no2 & 0x0f) * 2 + (no2 & 0xf0) * 16);

	pat_sub();
}

/*�p���b�g�E�Z�b�g*/
void pal_set_text(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	outp(0x1fb9 - 1 + color, green / 4 * 16 + red / 4 * 4 + blue / 4);
}

void pal_set(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	unsigned short adr1, adr2;
	adr1 = (color & 0x04) / 4 * 16 * 8 + (color & 0x02) / 2 * 8;
	adr2 = (color & 0x01) * 16 * 8;
	outp(0x1000 + adr1, adr2 + blue);
	outp(0x1100 + adr1, adr2 + red);
	outp(0x1200 + adr1, adr2 + green);

	if(color)
		pal_set_text(pal_no, color, red, green, blue);
}

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned short j;
	unsigned short adr1, adr2;
	for(j = 0; j < 4096; j++){
		adr1 = (j & 0xf00) / 256 * 16 + (j & 0xf0) / 16;
		adr2 = (j & 0x0f) * 16;
		outp(0x1000 + adr1, adr2 + j & 0x0f);
		outp(0x1100 + adr1, adr2 + (j & 0xf0) / 16);
		outp(0x1200 + adr1, adr2 + (j & 0xf00) / 256);
	}
}

void pal_all_text(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	for(j = 1; j < MAXCOLOR; j++)
		pal_set_text(pal_no, j, color[j][0], color[j][1], color[j][2]);
}

void wait_vsync(void)
{
	while(!(inp(0x1a01) & 0x80)); /* WAIT VSYNC */
	while((inp(0x1a01) & 0x80));
}

void sys_wait(unsigned char wait)
{
	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync();
}

//value < 0 ���ɋ߂Â���B
//value = 0 �ݒ肵���F
//value > 0 ���ɋ߂Â���B
void set_constrast(int value, unsigned char org_pal[MAXCOLOR][3], int pal_no)
{
	int j, k;
	int pal[3];

	for(j = 0; j < MAXCOLOR; j++){

		for(k = 0; k < 3; k++){
			if(value > 0)
				pal[k] = org_pal[j][k] + value;
			else if(value < 0)
				pal[k] = org_pal[j][k] * (15 + value) / 15;
			else
				pal[k] = org_pal[j][k];
			if(pal[k] < 0)
				pal[k] = 0;
			else if(pal[k] > 15)
				pal[k] = 15;
		}

//		DI();
//		wait_vsync();
//		outp(0x1fb0, 0x90);	/* ���F���[�h */
//		outp(0x1fc5, 0x80);	/* �O���t�B�b�N�p���b�g�A�N�Z�XON */

		pal_set(pal_no, j, pal[0], pal[1], pal[2]);

//		outp(0x1fc5, 0x0);
//		outp(0x1fb0, 0x0);
//		EI();
	}

}

//wait�l�̑��x�ō�����t�F�[�h�C������B
void fadeinblack(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = -15; j <= 0; j++){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//wait�l�̑��x�ō��Ƀt�F�[�h�A�E�g����B
void fadeoutblack(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = 0; j != -16; j--){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//wait�l�̑��x�Ŕ��Ƀt�F�[�h�A�E�g����B
void fadeoutwhite(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = 0; j < 16; j++){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//�p���b�g���Ó]����B
void pal_allblack(int pal_no)
{
	short j;
	unsigned short adr1, adr2;

	for(j = 0; j < 4096; j++){
		adr1 = (j & 0xf00) / 256 * 16 + (j & 0xf0) / 16;
		adr2 = (j & 0x0f) * 16;
		outp(0x1000 + adr1, adr2 | 0x00);
		outp(0x1100 + adr1, adr2 | 0x00);
		outp(0x1200 + adr1, adr2 | 0x00);
	}

	for(j = 1; j < 8; j++)
		outp(0x1fb9 - 1 + j, 0);
}

unsigned char data_buf[5];

unsigned char k0, k1, k2, k3, st, data_no;
unsigned char k0_old, k1_old, k2_old;
//unsigned short data, data_tmp;
unsigned char *data, *data_tmp;

unsigned char pat_no;
//unsigned short pat_adr;
unsigned char *pat_adr;
unsigned char x = 165, y = 30,xx, yy, old_x = 255, old_y = 255, k;

unsigned short vram_ofs_tmp;

unsigned char old_map_data[(X_SIZE + 2) * 32];
unsigned char sub_flag;
unsigned char *map_adr;
unsigned char *old_map_adr;

unsigned char fadeflag = 0;


unsigned char check_turbo(void)
{
__asm
	ld	h,#0
	ld	l,#0

	ld	b, #0x1f
	ld	c, #0xa0			; �{�̑���CTC($1fa0)�������turbo
	ld	a,7 ;00000111B
	out	(c), a
	out	(c), c
	in	a, (c)
	xor	c
	jr	nz,CHKEND
	ld	l,1
CHKEND:
__endasm;
}

/* BNN X1-Techknow��� */

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

void map_sub(void)
{
/*			for(j = 0; j < Y_SIZE / 2; ++j){
//				DI();
//				outp(0x1fd0, *WK1FD0 | 0x10); /* BANK1 */
//				outp(0x1fd0, 0x10); /* BANK1 */

				for(i = 0; i < X_SIZE / 2; ++i){
//					data_no = inp(data);
//					pat_no = ((data_no >> 4) & 0x0f) | ((data_no << 4) & 0xf0);
					pat_no = ((*data >> 4) & 0x0f) | ((*data << 4) & 0xf0);

//					pat_adr = ((PARTS_DATA + pat_no * 4));
					pat_adr = &mapdata[PARTS_HEAD + pat_no * 4];
//					ii = i * 2 + xx;
//					jj = j * 2 + yy;*/
/*					*(map_adr++) = inp(pat_adr++);
					*map_adr = inp(pat_adr++);
					map_adr += (32 - 1);
					*(map_adr++) = inp(pat_adr++);
					*map_adr = inp(pat_adr);
					map_adr -= (32 - 1);
*//*					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr++);
					map_adr += (32 - 1);
					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr);
					map_adr -= (32 - 1);
*/
/*					map_data[ii][jj] = inp(pat_adr++);
					map_data[ii+1][jj] = inp(pat_adr++);
					map_data[ii][jj+1] = inp(pat_adr++);
					map_data[ii+1][jj+1] = inp(pat_adr);
*///					++data;
				}
//				outp(0x1fd0, *WK1FD0); /* ���ɖ߂� */
//				outp(0x1fd0, 0); /* ���ɖ߂� */
//				EI();
/*				data_tmp += MAP_SIZE_X;
				data = data_tmp;
				map_adr += (32 * 2 - X_SIZE);
			}*/
/*__asm
	ld	a,(_yy)
	ld	l,a
	ld	h,a
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	ld	a,(_xx)
	ld	c,a
	ld	b,0
	add	hl,bc
	ld	bc,_map_data
	add	hl,bc
	ld	(_map_adr),hl*/
__asm
	ld	de,(_map_adr)
	ld	b,Y_SIZE / 2
loop0:
	push	bc
	ld	b,X_SIZE / 2
loop1:
	push	bc
	ld	hl,(_data)
	ld	a,(hl)
	rlc	a
	rlc	a
	rlc	a
	rlc	a

	ld	l,a
	ld	h,0
	add	hl,hl
	add	hl,hl
	ld	bc,PARTS_HEAD
	add	hl,bc
	ld	bc,_mapdata
	add	hl,bc
	ex	de,hl

;	ld	hl,(_map_adr)

	ld	a,(de)
	ld	(hl),a
	inc	de
	inc	hl

	ld	a,(de)
	ld	(hl),a
	inc	de
	ld	bc,32-1
	add	hl,bc

	ld	a,(de)
	ld	(hl),a
	inc	de
	inc	hl

	ld	a,(de)
	ld	(hl),a
	ld	bc,-(32-1)
	add	hl,bc

;	ld	(_map_adr),hl

	ex	de,hl
	ld	hl,(_data)
	inc	hl
	ld	(_data),hl

	pop	bc
	djnz loop1

;	push	bc
	ex	de,hl
;	ld	hl,(_map_adr)
	ld	bc,32 * 2 - X_SIZE
	add	hl,bc
;	ld	(_map_adr),hl
	ex	de,hl

	ld	hl,(_data_tmp)
	ld	bc,MAP_SIZE_X
	add	hl,bc
	ld	(_data_tmp),hl
	ld	(_data),hl

	pop	bc
	djnz loop0

;	ld	hl,_map_data+1+1*32
;	ld	(_map_adr),hl
__endasm;
}

int main(void)
{
//	unsigned char ii, jj;
	unsigned char turbo = 0;
	unsigned char zmode = 0;

	for(i = 0; i < 200; ++i){
		y_table[i] = ((i / 8) * 80 + (i & 7) * 0x800);
	}

	if(bload("elmst3.pcg", map_data, PARTS_SIZE))
		return ERROR;
	outp(0x1a03, 0x0c);	/* 80�s */
	set_lowreso();
	set_pcg(map_data);
	if(!(inp(0x1ff0) & 0x01)){
		set_hireso();
		outp(0x1fd0, 0x03);
	}

/*�}�b�v�̃��[�h to (unsigned char mapdata[])*/

	if(mapload("ELMSTMAP.MDT")){
		return ERROR;
	}
	if(bload("elmchrx.grp", vram_data, CHR_SIZE))
		return ERROR;

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}

	/* Change Pallet */
	outp(0x1000, 0xa2);
	outp(0x1100, 0xc4);
	outp(0x1200, 0xf0);

	/* Priority */
	outp(0x1300, 0xfe);

	outp(0x1fb0, 0x90);	/* ���F���[�h */
	if(inp(0x1fb0) == 0x90){
		zmode = 1;
		outp(0x1fc5, 0x80);	/* �O���t�B�b�N�p���b�g�A�N�Z�XON */
		pal_allblack(0);
	}

	for(i = 0; i < X_SIZE - 2; ++i){
		for(j = 0; j < Y_SIZE - 2; ++j){
			vram_ofs = (OFS_X + i) * PARTS_X + (OFS_Y + j) * SIZE;
			pat_sub2();
		}
	}

	if(zmode){
		outp(0x1fc0, 0x01);	/* ���F���[�h�v���C�I���e�B */
//		outp(0x1fe0, 0);
	}

	k0 = 0;
	k1 = 0;
	k2 = 0;

/*	outp(#0x1fa0, 0x07);
	outp(#0x1fa0, 0xa0);
	if((inp(0x1fa0) ^ 0xa0)){
		turbo = 1;
	}*/
	turbo = check_turbo();
//	if(!turbo)
	set_key();
	do{
		if(turbo){
			k0_old = k0;
			k1_old = k1;
			k2_old = k2;

			data_buf[0]=0xe3;
			get_key(data_buf, 4);

			k0 = data_buf[1];
			k1 = data_buf[2];
			k2 = data_buf[3];
			k3 = 0;

			data_buf[0]=0xe6;
			get_key(data_buf, 3);
			if((k0 == k0_old) && (k1 == k1_old) && (k2 == k2_old)){
				k3 = data_buf[2];
			}
		}else{
			data_buf[0]=0xe6;
			get_key(data_buf, 3);
			k3 = data_buf[2];
		}

		DI();
		outp(0x1c00,14);
		st = inp(0x1b00);
		EI();

		if((k1 & 0x10) || (k3 == 30) || (k3 == 56) || !(st & 0x01)){ /* 8 */
			if(y > 0){
				--y;
				dir = 0;
			}
		}
		if((k1 & 0x02) || (k3 == 28) || (k3 == 54) || !(st & 0x08)){ /* 6 */
			if(x < (MAP_SIZE_X*2 - X_SIZE)){
				++x;
				dir = 1;
			}
		}
		if((k1 & 0x08) || (k3 == 31) || (k3 == 50) || !(st & 0x02)){ /* 2 */
			if(y < (MAP_SIZE_Y*2 - Y_SIZE)){
				++y;
				dir = 2;
			}
		}
		if((k1 & 0x40) || (k3 == 29) || (k3 == 52) || !(st & 0x04)){ /* 4 */
			if(x > 0){
				--x;
				dir = 3;
			}
		}
		if((k0 & 0x04) || (k2 & 0x02) || (k3 == 32)  || (k3 == 122) || !(st & 0x20)) /* Z,SPACE */
			if((k0 & 0x02) || (k3 == 120) || !(st & 0x40)) /* X */
				break;

		if((old_x != x) || (old_y != y)){
			xx = 1 - x % 2;
			yy = 1 - y % 2;
//			data = (MAP_ADR + (x / 2) + (y / 2) * MAP_SIZE_X);
			data = &mapdata[x / 2 + (y / 2) * MAP_SIZE_X];
			data_tmp = data;
			vram_ofs = 0x3000 + (OFS_X) * PARTS_X + (OFS_Y) * SIZE;
			vram_ofs_tmp = vram_ofs;
			map_adr = &map_data[xx + yy * 32];

			map_sub();

			dir2 = 1 - dir2;
			map_adr = &map_data[1 + 1 * 32];
			old_map_adr = &old_map_data[1 + 1 * 32];

/*			if((old_x != x)){
				for(i = 1; i < (X_SIZE - 1); ++i){
					for(j = 1; j < (Y_SIZE - 1); ++j){*/
/*						no = *map_adr;
						sub_flag = 0;
						if(i >= CHR_X){
							if(i <= (CHR_X+1)){
								if(j >= CHR_Y){
									if(j <= (CHR_Y+1)){
										sub_flag = 1;
									}
								}
							}
						}
						if(sub_flag){
							chr_sub();
						}else if(no != *old_map_adr){
							pat_sub();
							*old_map_adr = no;
						}
*/
__asm
	ld	hl,_old_x
	ld	a,(_x)
	cp	a,(hl)
	jp	z,skip00

	ld	hl,(_map_adr)
	ld	de,(_old_map_adr)

	ld	c,1
skip01:
	ld	a,c
	ld	(_i),a
	ld	b,1
skip02:
	ld	a,b
	ld	(_j),a
	push	bc
;	ld	hl,(_map_adr)
	ld	a,(hl)
	ld	(_no),a
	ld	a,c ;(_i)
	cp	a,CHR_X	; i - CHR_X >= 0
	jr	c,skip1
	cp	a,CHR_X+2	; i - (CHR_X+1) <= 0
;	jr	z,skip0_1
	jr	nc,skip1
skip0_1:
	ld	a,b ;(_j)
	cp	a,CHR_Y	; j - CHR_Y >= 0
	jr	c,skip1
	cp	a,CHR_Y+2	; j - (CHR_Y+1) <= 0
;	jr	z,skip0_2
	jr	nc,skip1
skip0_2:
;	push	bc
	push	hl
	push	de
	call	_chr_sub
	pop	de
	pop	hl
;	pop	bc
	jr	skip2
skip1:
;	ld	a,(hl)
;	ld	de,(_old_map_adr)
	ld	a,(de)
	cp	a,(hl)
	jr	z,skip2
	ld	a,(hl)
	ld	(de),a
	push	bc
	call	_pat_sub
	pop	bc
skip2:
	push	hl
	ld	hl,(_vram_ofs)
	ld	bc,SIZE
	add	hl,bc
	ld	(_vram_ofs),hl
	pop	hl

	ld	bc,32
;	ld	hl,(_map_adr)
	add	hl,bc
;	ld	(_map_adr),hl
	ex	de,hl
;	ld	hl,(_old_map_adr)
	add	hl,bc
;	ld	(_old_map_adr),hl
	ex	de,hl

	pop	bc
;	ld	hl,_j
;	inc	(hl)
;	ld	a,(hl)
	inc	b
	ld	a,b
	cp	a,Y_SIZE-1
	jr	nz,skip02

	push	bc
	push	hl
	ld	hl,(_vram_ofs_tmp)
	ld	bc,PARTS_X
	add	hl,bc
	ld	(_vram_ofs_tmp),hl
	ld	(_vram_ofs),hl
	pop	hl

	ld	bc,1 - 32 * (X_SIZE - 2)
;	ld	hl,(_map_adr)
	add	hl,bc
;	ld	(_map_adr),hl
	ex	de,hl
;	ld	hl,(_old_map_adr)
	add	hl,bc
;	ld	(_old_map_adr),hl
	ex	de,hl
	pop	bc

;	ld	hl,_i
;	inc	(hl)
;	ld	a,(hl)
	inc	c
	ld	a,c
	cp	a,X_SIZE-1
	jp	nz,skip01
	jp	skip05
__endasm;
/*						vram_ofs += SIZE;
						map_adr += 32;
						old_map_adr += 32;
					}
					vram_ofs_tmp += PARTS_X;
					vram_ofs = vram_ofs_tmp;
					map_adr += (1 - 32 * (X_SIZE - 2));
					old_map_adr += (1 - 32 * (X_SIZE - 2));
				}
			}
			else
			{
				for(j = 1; j < (Y_SIZE - 1); ++j){
					for(i = 1; i < (X_SIZE - 1); ++i){*/
/*						no = *map_adr;
						sub_flag = 0;
						if(i >= CHR_X){
							if(i <= (CHR_X+1)){
								if(j >= CHR_Y){
									if(j <= (CHR_Y+1)){
										sub_flag = 1;
									}
								}
							}
						}
						if(sub_flag){
							chr_sub();
						}else if(no != *old_map_adr){
							pat_sub();
							*old_map_adr = no;
						}*/
__asm
skip00:
	ld	c,1
	ld	hl,(_map_adr)
	ld	de,(_old_map_adr)

skip03:
	ld	a,c
	ld	(_j),a
	ld	b,1
skip04:
	ld	a,b
	ld	(_i),a
	push	bc
;	ld	hl,(_map_adr)
	ld	a,(hl)
	ld	(_no),a
	ld	a,b ;(_i)
	cp	a,CHR_X	; i - CHR_X >= 0
	jr	c,skip4
	cp	a,CHR_X+2	; i - (CHR_X+1) <= 0
;	jr	z,skip3_1
	jr	nc,skip4
skip3_1:
	ld	a,c ;(_j)
	cp	a,CHR_Y	; j - CHR_Y >= 0
	jr	c,skip4
	cp	a,CHR_Y+2	; j - (CHR_Y+1) <= 0
;	jr	z,skip3_2
	jr	nc,skip4
skip3_2:
;	push	bc
	push	hl
	push	de
	call	_chr_sub
	pop	de
	pop	hl
;	pop	bc
	jr	skip5
skip4:
;	ld	a,(hl)
;	ld	de,(_old_map_adr)
	ld	a,(de)
	cp	a,(hl)
	jr	z,skip5
	ld	a,(hl)
	ld	(de),a
;	push	bc
	push	hl
	push	de
	call	_pat_sub
	pop	de
	pop	hl
;	pop	bc
skip5:
	push	hl
	ld	hl,(_vram_ofs)
	ld	bc,PARTS_X
	add	hl,bc
	ld	(_vram_ofs),hl
	pop	hl
;	ld	hl,(_map_adr)
	inc	hl
;	ld	(_map_adr),hl
;	ld	de,(_old_map_adr)
	inc	de
;	ld	(_old_map_adr),de

	pop	bc
;	ld	hl,_i
;	inc	(hl)
;	ld	a,(hl)
	inc	b
	ld	a,b
	cp	a,X_SIZE-1
	jr	nz,skip04

	push	bc
	push	hl
	ld	hl,(_vram_ofs)
	ld	bc,SIZE - (PARTS_X) * (X_SIZE - 2)
	add	hl,bc
	ld	(_vram_ofs),hl
	pop	hl
	ld	bc,32 - (X_SIZE - 2)
;	ld	hl,(_map_adr)
	add	hl,bc
;	ld	(_map_adr),hl
	ex	de,hl
;	ld	hl,(_old_map_adr)
	add	hl,bc
;	ld	(_old_map_adr),hl
	ex	de,hl
	pop	bc

;	ld	hl,_j
;	inc	(hl)
;	ld	a,(hl)
	inc	c
	ld	a,c
	cp	a,Y_SIZE-1
	jp	nz,skip03
skip05:
__endasm;

/*						vram_ofs += PARTS_X;
						++map_adr;
						++old_map_adr;
					}
					vram_ofs += (SIZE - (PARTS_X) * (X_SIZE - 2));
					map_adr += (32 - (X_SIZE - 2));
					old_map_adr += (32 - (X_SIZE - 2));
				}
			}*/
		}
		old_x = x;
		old_y = y;

		if(fadeflag == 0){
			fadeflag = 1;
			if(inp(0x1fb0) == 0x90){
				fadeinblack(org_pal, 0, 3);
			}
		}

	}while(!(k2 & 0x80) && (k3 != 27));
//	printf("End.\n");

	if(zmode){
		fadeoutblack(org_pal, 0, 3);
//		outp(0x1fb0, 0x90);	/* ���F���[�h */
//		outp(0x1fc5, 0x80);	/* �O���t�B�b�N�p���b�g�A�N�Z�XON */
//		pal_all(0, org_pal2);
		pal_all_text(0, org_pal2);
//		for(j = 1; j < 8; j++)
//			outp(0x1fb9 - 1 + j, 255);
		set_constrast(0, org_pal2, 0);

		outp(0x1fc5, 0x0);
		outp(0x1fb0, 0x0);
	}

	/* Pallet */
	outp(0x1000, 0xaa);
	outp(0x1100, 0xcc);
	outp(0x1200, 0xf0);

	/* Priority */
	outp(0x1300, 0x00);

	return NOERROR;
}

