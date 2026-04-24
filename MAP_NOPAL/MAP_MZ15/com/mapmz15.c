/* z88dk MZ1500  PCGキャラMAP移動実験 2x2圧縮展開 ZSDCC版 By m@3 */
/* キャラを出す */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "inkey.h"

#define vram_data ((volatile unsigned char *)0xd000)
#define atr ((volatile unsigned char *)0xd800)

#define keystrobe ((volatile unsigned char *)0xe000)
#define keydata ((volatile unsigned char *)0xe001)

#define joyport ((volatile unsigned char *)0xe008)

#define ioport  ((volatile unsigned char *)0xe002)

#define pcgvram1 ((volatile unsigned char *)0xd400)
#define pcgvram2 ((volatile unsigned char *)0xdc00)

#define pcgdata ((volatile unsigned char *)0xd000)


#define VRAM_ADR 0

#define ERROR 1
#define NOERROR 0


#define PARTS_HEAD 0x3c00 /*組み合わせキャラデータの先頭番地*/
#define BUFFSIZE 16384
unsigned char mapdata[BUFFSIZE];

FILE *stream[2];

#define PARTS_NUM 128

#define PARTS_SIZE 8*3*PARTS_NUM
//unsigned char pcg_data[PARTS_SIZE];

#define PARTS_NUM2 8*4

#define PARTS_SIZE2 8*3*PARTS_NUM2
unsigned char pcg_data2[PARTS_SIZE2];

#define MASK_SIZE 8*PARTS_NUM2

unsigned char mask_data[MASK_SIZE];

#define SIZE 40

#define X_SIZE 18
#define Y_SIZE 18

#define PARTS_X 1
#define PARTS_Y 8

#define MAP_SIZE_X 128
#define MAP_SIZE_Y 128

#define OFS_X 2
#define OFS_Y 2

#define CHR_X 8
#define CHR_Y 8

unsigned short vram_ofs;
unsigned char map_data[(X_SIZE+2) * 32];

#define MAXCOLOR 8

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

/*マップデータをファイルから配列に読み込む*/
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
//	fread( buffer, 1, 7, stream[0]);
	fread( buffer, 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}


unsigned char no;
unsigned char *vram_adr;
unsigned char a,b;

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
unsigned char dir = 2, dir2 = 0;
unsigned short i, j, k;

void clearBuffer(void) {
	while (kbhit()) {
		getch(); // 押されているキーをすべて読み飛ばす
	}
}

void setpcg(unsigned char *data, unsigned char bank, int num)
{
	int i, j, k = bank * 256 * 8;
//	DI();
	for(j = 0; j < num; ++j){
		for(i = 0; i < 8; ++i){
			outp(0xe5, 0x01);	/* blue */
			pcgdata[k] = *(data++);
			outp(0xe5, 0x02);	/* red */
			pcgdata[k] = *(data++);
			outp(0xe5, 0x03);	/* green */
			pcgdata[k] = *(data++);
			++k;
		}
	}
	outp(0xe6, 0x00);	/* PCG close */
//	EI();
}

void pat_sub(void)
{
//	DI();

//	outp(0xe3,0);		/* VRAM */
//	pcgvram1[vram_ofs] = no;
__asm
	ld	bc,0xd400 ; pcgvram1
	ld	hl,(_vram_ofs)
	add	hl,bc
	ld	a,(_no)
	ld	(hl),a
__endasm;

//	pcgvram2[vram_ofs] = 0x08;

//	outp(0xe1,0);		/* DRAM */
//	EI();
}

void pat_sub2(void)
{
//	DI();

//	outp(0xe3,0);		/* VRAM */
//	pcgvram1[vram_ofs] = 0;
__asm
	ld	bc,0xd400 ; pcgvram1
	ld	hl,(_vram_ofs)
	add	hl,bc
;	ld	a,(_no)
	xor	a
	ld	(hl),a
__endasm;
//	pcgvram2[vram_ofs] = 0x08;

//	outp(0xe1,0);		/* DRAM */
//	EI();
}

unsigned char no2,num;
unsigned short ii,jj, kk, ll;//, m;
unsigned char mask;

void put_chrmz_pat(void) //unsigned char no2, unsigned char num) __sdcccall(1)
{

	ii = no * 8;
	jj = no2 * 8 * 3;
	kk = (256 + num) * 8;
	ll = no2 * 8;
__asm
	push	bc
	push	de
	push	hl

	ld	b,8
chrmzloop:
	ld	de,_mask_data
	ld	hl,(_ll)
	add	hl,de
	ld	a,(hl)
	ld	(_mask),a


	ld	a,1
	out	(0xe5),a

	ld	a,(_mask)
	ld	c,a

	ld	de,0xd000 ;pcgdata
	ld	hl,(_ii)
	add	hl,de
	ld	a,(hl)

	and	a,c
	ld	c,a

	ld	de,_pcg_data2
	ld	hl,(_jj)
	add	hl,de
	ld	a,(hl)
	or	a,c

	ld	hl,(_jj)
	inc	hl
	ld	(_jj),hl

	ld	de,0xd000 ;pcgdata
	ld	hl,(_kk)
	add	hl,de
	ld	(hl),a
	push	hl


	ld	a,2
	out	(0xe5),a

	ld	a,(_mask)
	ld	c,a

	ld	de,0xd000 ;pcgdata
	ld	hl,(_ii)
	add	hl,de
	ld	a,(hl)

	and	a,c
	ld	c,a

	ld	de,_pcg_data2
	ld	hl,(_jj)
	add	hl,de
	ld	a,(hl)
	or	a,c

	ld	hl,(_jj)
	inc	hl
	ld	(_jj),hl

	ld	de,0xd000 ;pcgdata
	ld	hl,(_kk)
	add	hl,de
	pop	hl
	ld	(hl),a
	push	hl


	ld	a,3
	out	(0xe5),a

	ld	a,(_mask)
	ld	c,a

	ld	de,0xd000 ;pcgdata
	ld	hl,(_ii)
	add	hl,de
	ld	a,(hl)

	and	a,c
	ld	c,a

	ld	de,_pcg_data2
	ld	hl,(_jj)
	add	hl,de
	ld	a,(hl)
	or	a,c

	ld	hl,(_jj)
	inc	hl
	ld	(_jj),hl

	ld	de,0xd000 ;pcgdata
	ld	hl,(_kk)
	add	hl,de
	pop	hl
	ld	(hl),a

	ld	hl,(_ii)
	inc	hl
	ld	(_ii),hl

	ld	hl,(_kk)
	inc	hl
	ld	(_kk),hl

	ld	hl,(_ll)
	inc	hl
	ld	(_ll),hl

	dec	b
	jp	nz,chrmzloop

	xor	a
	out	(0xe6),a

	pop	hl
	pop	de
	pop	bc
__endasm;
	return;
/*
	for(m = 0; m < 8; ++m){
		mask = mask_data[ll];
		outp(0xe5, 0x01);	/* blue */
/*		pcgdata[kk] = pcgdata[ii] & mask | pcg_data2[jj++];

		outp(0xe5, 0x02);	/* red */
/*		pcgdata[kk] = pcgdata[ii] & mask | pcg_data2[jj++];

		outp(0xe5, 0x03);	/* green */
/*		pcgdata[kk++] = pcgdata[ii] & mask | pcg_data2[jj++];

		++ii;
		++ll;
	}
	outp(0xe6, 0x00);*/	/* PCG close */
//	outp(0xe3,0);		/* VRAM */
}

void chr_sub(void)
{
//	unsigned char 
	num = (i - CHR_X) + (j - CHR_Y) * 2;
//	unsigned char 
	no2 = chr_tbl[dir * 2 + dir2][num];

//	DI();

	put_chrmz_pat(); //no2, num);

//	outp(0xe3,0);		/* VRAM */
//	pcgvram1[vram_ofs] = num;
//	pcgvram2[vram_ofs] = 0x08 | 0x40;
__asm
	ld	hl,0xd400 ; pcgvram1
	ld	de,(_vram_ofs)
	add	hl,de
	ld	a,(_num)
	ld	(hl),a

	ld	hl,0xdc00 ; pcgvram2
;	ld	de,(_vram_ofs)
	add	hl,de
	ld	a,0x08 | 0x40
	ld	(hl),a
__endasm;

//	outp(0xe1,0);		/* DRAM */
//	EI();
//	pat_sub();
}

void wait_vsync(void)
{
	while(!((*ioport) & 0x80)); /* WAIT VSYNC */
	while((*ioport) & 0x80);
}

void sys_wait(unsigned char wait)
{
	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync();
}

// vsync
void vsync(void) {
	wait_vsync();
}

void wait(int j) {
	int i;
	for (i = 0; i < j; ++i)
		vsync();
}

enum {
	COLOR_BLACK,
	COLOR_BLUE,
	COLOR_RED,
	COLOR_MAGENTA,
	COLOR_GREEN,
	COLOR_CYAN,
	COLOR_YELLOW,
	COLOR_WHITE
};

void cls(void) {
//	DI();
//	outp(0xe3,0);		/* VRAM */
	k = 0;
	for(j = 0; j < 25; j++){
		for(i = 0; i < 40; ++i){
//			put_chr8(i, j, ' ', 0);
//			k = i + j * 40;
			pcgvram1[k] = 0x20;
			pcgvram2[k] = 0x08;

			vram_data[k] = 0;
			atr[k] = COLOR_BLACK | (COLOR_WHITE * 16);
			++k;
		}
	}
//	outp(0xe1,0);		/* DRAM */
//	EI();
}

void cls2(void) {
//	int i,j,k=0;
	k = 0;
//	DI();
//	outp(0xe3,0);		/* VRAM */
	for(j = 0; j < 25; j++){
		for(i = 0; i < 40; ++i){
//			put_chr8(i, j, ' ', 0);
//			k = i + j * 40;
			pcgvram1[k] = 0x00;
			pcgvram2[k] = 0x00;

			vram_data[k] = 0;
			atr[k] = COLOR_BLACK | (COLOR_WHITE * 16);
			++k;
		}
	}
//	outp(0xe1,0);		/* DRAM */
//	EI();
}

unsigned char keycode = 0;
unsigned char k2, k1, k3, k6, k7;
unsigned char st1,st2;
//unsigned char
//volatile int x_count_org = 0, y_count_org = 0;
//unsigned char
//volatile int x_count = 0, y_count = 0;

unsigned char keyscan(void)
{
//	DI();
//	outp(0xe3,0);		/* VRAM,KEY */
	keycode = 0;
//	DI();
//	outp(0x1c00,14);
//	st = *joydata;
//	EI();
//	st1 = ~GetJoy1(&x_count);
//	st2 = ~GetJoy2(&y_count);

	*keystrobe = 0x01 | 0x80;
	k1 = ~(*keydata);

	*keystrobe = 0x02 | 0x80;
	k2 = ~(*keydata);

	*keystrobe = 0x03 | 0x80;
	k3 = ~(*keydata);

	*keystrobe = 0x06 | 0x80;
	k6 = ~(*keydata);

	*keystrobe = 0x07 | 0x80;
	k7 = ~(*keydata);

	if((k7 & 0x20)){// || ((y_count > (y_count_org + 10)) && ((y_count > 100)))){ /* ↑ */
		keycode |= KEY_UP1;
	}
	if((k7 & 0x08)){// || ((x_count < (x_count_org - 10)) && ((x_count > 100)) ) ){ /* → */
		keycode |= KEY_RIGHT1;
	}
	if((k7 & 0x10)){// || ((y_count < (y_count_org - 10)) && ((y_count > 100)))){ /* ↓ */
		keycode |= KEY_DOWN1;
	}
	if((k7 & 0x04)){// || ((x_count > (x_count_org + 10)) && ((x_count > 100)))){ /* ← */
		keycode |= KEY_LEFT1;
	}
	if((k6 & 0x10) || (k1 & 0x40)){// || (x_count < 100)){ /* Z,SPACE */
		keycode |= KEY_A;
	}
	if((k2 & 0x01)){// || (y_count < 100)){ /* X */
		keycode |= KEY_B;
	}

//	outp(0xe1,0);		/* DRAM */
//	EI();
	return keycode;
}

unsigned char  st, data_no;
unsigned char *data, *data_tmp;

unsigned short pat_no;
unsigned char *pat_adr;
unsigned short x = 165, y = 30,xx, yy, old_x = 255, old_y = 255;
unsigned short vram_ofs_tmp;

unsigned char old_map_data[(X_SIZE + 2) * 32];
unsigned char sub_flag;
unsigned char *map_adr;
unsigned char *old_map_adr;

void map_sub(void)
{
/*			for(j = 0; j < Y_SIZE / 2; ++j){

				for(i = 0; i < X_SIZE / 2; ++i){
					data_no = *data;

					pat_no = ((data_no >> 4) & 0x0f) | ((data_no << 4) & 0xf0);

					pat_adr = &mapdata[7+PARTS_HEAD + pat_no * 4];

					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr++);
					map_adr += (32 - 1);
					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr);
					map_adr -= (32 - 1);

					++data;
				}
				data_tmp += MAP_SIZE_X;
				data = data_tmp;
				map_adr += (32 * 2 - X_SIZE);
			}

			return;
*/
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
	outp(0xf0,0x01);	/* PCG ON */
/*	for(i = 0; i < 256; ++i){
		pcgvram1[i] = i;
		pcgvram2[i] = 0x08 | 0x40;
	}*/

	if(bload("elmst15.pcg", mapdata, PARTS_SIZE))
		return ERROR;

	outp(0xe3,0);		/* VRAM */
	setpcg(mapdata, 0, PARTS_NUM);

	if(bload("elmch15.pcg", pcg_data2, PARTS_SIZE2))
		return ERROR;
//	setpcg(pcg_data2, 1, PARTS_NUM2);

	if(bload("elmsk15.pcg", mask_data, MASK_SIZE))
		return ERROR;

/*マップのロード to (unsigned char mapdata[])*/

	if(mapload("ELMSTMAP.MDT")){
		return ERROR;
	}

	outp(0xe3,0);		/* VRAM */
	cls();

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}
	for(i = 0; i < X_SIZE - 2; ++i){
		for(j = 0; j < Y_SIZE - 2; ++j){
			vram_ofs = (VRAM_ADR +(OFS_X + i) * PARTS_X + (OFS_Y + j) * SIZE);
			pat_sub2();
		}
	}

	for(;;){
		keycode = keyscan();

		if(keycode & KEY_UP1){ /* 8 */
			if(y > 0){
				--y;
				dir = 0;
			}
		}
		if(keycode & KEY_RIGHT1){ /* 6 */
			if(x < (MAP_SIZE_X*2 - X_SIZE)){
				++x;
				dir = 1;
			}
		}
		if(keycode & KEY_DOWN1){ /* 2 */
			if(y < (MAP_SIZE_Y*2 - Y_SIZE)){
				++y;
				dir = 2;
			}
		}
		if(keycode & KEY_LEFT1){ /* 4 */
			if(x > 0){
				--x;
				dir = 3;
			}
		}
		if(keycode & KEY_A) /* Z,SPACE */
			if(keycode & KEY_B) /* X */
				break;

		if((old_x != x) || (old_y != y)){
			xx = 1 - x % 2;
			yy = 1 - y % 2;

			data = &mapdata[x / 2 + (y / 2) * MAP_SIZE_X];

			data_tmp = data;
			vram_ofs = (VRAM_ADR +(OFS_X) * PARTS_X + (OFS_Y) * SIZE);
			vram_ofs_tmp = vram_ofs;
			map_adr = &map_data[xx + yy * 32];

			map_sub();

			dir2 = 1 - dir2;
			map_adr = &map_data[1 + 1 * 32];
			old_map_adr = &old_map_data[1 + 1 * 32];

/*			if((old_x != x)){
				for(i = 1; i < (X_SIZE - 1); ++i){
					for(j = 1; j < (Y_SIZE - 1); ++j){
						no = *map_adr;
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
	push	hl
	push	bc
	push	de
	call	_pat_sub
	pop	de
	pop	bc
	pop	hl
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
					for(i = 1; i < (X_SIZE - 1); ++i){
						no = *map_adr;
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
/*
						vram_ofs += PARTS_X;
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
	}

	cls2();

	return NOERROR;
}

