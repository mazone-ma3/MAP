/* z88dk X1turbo/Z  PCGキャラMAP移動実験 2x2圧縮展開 By m@3 */
/* キャラを出す */

#include <stdio.h>
#include <stdlib.h>

#define MAP_ADR 0x8000
#define PARTS_DATA (MAP_ADR+0x3c00)
#define CHRPAT_ADR 0xC000

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

unsigned short y_table[200];

unsigned short vram_ofs;
unsigned char map_data[(X_SIZE+2) * 32];
//unsigned char *WK1FD0 = (unsigned char *)0xf8d6;


void DI(void){
#asm
	DI
#endasm
}

void EI(void){
#asm
	EI
#endasm
}

/* 試験に出るX1より引用 */
void get_key(unsigned char *data, unsigned short num)
{
#asm
	ld	hl, 2
	add	hl, sp
	ld	c,(hl)
	inc	hl
	ld	b,(hl)	;bc = num
	inc	hl
	ld	e, (hl)
	inc	hl
	ld	d, (hl)	; de = data


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
	ld	bc,$1900
	out	(c),d
	ret

GET1:	call	CANR
	ld	bc,$1900
	in	d,(c)
	ret

CANW:	ld	bc,$1a01
CANWLP:	in	a,(c)
	and	$40
	jr	nz,CANWLP
	ret

CANR:	ld	bc,$1a01
CANRLP:	in	a,(c)
	and	$20
	jr	nz,CANRLP
	ret
#endasm
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

#define VRAM_MACRO(X,Y) (X + (Y / 8) * 80 + (Y & 7) * 0x800)
unsigned char data2[3*2*8]; //[3][2][8];
//unsigned char data[3][2][8];

/* 3プレーン転送 */
/* 0x4000,0x8000,0xc000 */
unsigned char adr_tmp2_x, adr_tmp2_y;
void put_chrx1_pat(unsigned short patadr)
{
/*	unsigned short adr_tmp;
	unsigned short ii,jj;
	adr_tmp = CHRPAT_ADR + patadr * 2 + patadr;
	for(jj = 0 ; jj < PARTS_Y; ++jj){
		DI();*/
//		outp(0x1fd0, *WK1FD0 | 0x10); /* BANK1 */
//		outp(0x1fd0, 0x10); /* BANK1 */
/*		for(ii = 0 ; ii < PARTS_X; ++ii){
			data[0][ii][jj] = inp(adr_tmp++);
			data[1][ii][jj] = inp(adr_tmp++);
			data[2][ii][jj] = inp(adr_tmp++);
		}
		adr_tmp += (32 * 3 - PARTS_X * 3);*/
//		outp(0x1fd0, *WK1FD0); /* 元に戻す */
//		outp(0x1fd0, 0); /* 元に戻す */
//		EI();
//	}
#asm
	ld	hl, 2
	add	hl, sp
	ld	c,(hl)
	inc	hl
	ld	b,(hl)	;bc = patadr
	ld	l,c
	ld	h,b
	add	hl,bc
	add	hl,bc
	ld	bc,CHRPAT_ADR;
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	hl,_data2
	ld	d,PARTS_Y
patloop1:
	push	bc
	di
	ld	bc,$1fd0
	ld	a,0x10
	out (c), a
	pop	bc
	ld	e,PARTS_X
patloop2:
	in a,(c)
	ld	(hl),a
	inc	bc
	inc	hl
	in a,(c)
	ld	(hl),a
	inc	bc
	inc	hl
	in a,(c)
	ld	(hl),a
	inc	bc
	inc	hl
	dec	e
	jr	nz,patloop2
	push	hl
	ld	l,c
	ld	h,b
	ld	bc,32*3-PARTS_X*3
	add	hl,bc
	ld	c,l
	ld	b,h
	pop	hl
	push	bc
	ld	bc,$1fd0
	xor	a
	out (c), a
	pop	bc
	ei
	dec	d
	jr	nz,patloop1
#endasm

	adr_tmp2_x = ((OFS_X - 1 + i) * PARTS_X);
	adr_tmp2_y = ((OFS_Y - 1 + j) * PARTS_Y);
/*	for(jj = 0 ; jj < PARTS_Y; ++jj){
		adr_tmp = VRAM_MACRO(((OFS_X - 1 + i) * PARTS_X), ((OFS_Y - 1 + j) * PARTS_Y + jj));
		for(ii = 0 ; ii < PARTS_X; ++ii){
			outp(0x4000 + adr_tmp, data[0][ii][jj]);
			outp(0x8000 + adr_tmp, data[1][ii][jj]);
			outp(0xc000 + adr_tmp, data[2][ii][jj]);
			++adr_tmp;
		}
	}*/
#asm
	ld	de,_data2
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
	ld	hl,$4000
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	a,(de)
	inc	de
	out (c),a
	ld	hl,$4000
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	a,(de)
	inc	de
	out (c),a
	ld	hl,$4000
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
	inc	c
	ld	a,c
	cp	PARTS_Y
	jr	nz,patloop3
#endasm
}

void pat_sub(void)
{
//	outp(0x3000 + vram_ofs, no * 2);
//	outp(0x3000 + vram_ofs+1, no * 2 +1);
#asm
	ld	bc,(_vram_ofs)
	ld	a,(_no)
	sla	a
	out (c),a
	inc	bc
	inc	a
	out	(c),a
#endasm
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


unsigned char data_buf[5];

void main(void)
{
	unsigned char k0, k1, k2, k3, st, data_no;
	unsigned short data, data_tmp;

	unsigned char pat_no;
	unsigned short pat_adr;
	unsigned char x = 165, y = 30,xx, yy, old_x = 255, old_y = 255,, k;
//	unsigned char ii, jj;
	unsigned short vram_ofs_tmp;

	unsigned char old_map_data[(X_SIZE + 2) * 32];
	unsigned char sub_flag;
	unsigned char *map_adr;
	unsigned char *old_map_adr;

	for(i = 0; i < 200; ++i){
		y_table[i] = ((i / 8) * 80 + (i & 7) * 0x800);
	}

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}
	for(i = 0; i < X_SIZE - 2; ++i){
		for(j = 0; j < Y_SIZE - 2; ++j){
			vram_ofs = (OFS_X + i) * PARTS_X + (OFS_Y + j) * SIZE;
			pat_sub2();
		}
	}

	/* Change Pallet */
	outp(0x1000, 0xa2);
	outp(0x1100, 0xc4);
	outp(0x1200, 0xf0);

	/* Priority */
	outp(0x1300, 0xfe);

	do{
		data_buf[0]=0xe3;
		get_key(data_buf, 4);

		k0 = data_buf[1];
		k1 = data_buf[2];
		k2 = data_buf[3];

		data_buf[0]=0xe6;
		get_key(data_buf, 3);
		k3 = data_buf[2];

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
			data = (MAP_ADR + (x / 2) + (y / 2) * MAP_SIZE_X);
			data_tmp = data;
			vram_ofs = 0x3000 + (OFS_X) * PARTS_X + (OFS_Y) * SIZE;
			vram_ofs_tmp = vram_ofs;
			map_adr = &map_data[xx + yy * 32];

			for(j = 0; j < Y_SIZE / 2; ++j){
				DI();
//				outp(0x1fd0, *WK1FD0 | 0x10); /* BANK1 */
				outp(0x1fd0, 0x10); /* BANK1 */

				for(i = 0; i < X_SIZE / 2; ++i){
					data_no = inp(data);

					pat_no = ((data_no >> 4) & 0x0f) | ((data_no << 4) & 0xf0);
					pat_adr = ((PARTS_DATA + pat_no * 4));
//					ii = i * 2 + xx;
//					jj = j * 2 + yy;
					*(map_adr++) = inp(pat_adr++);
					*map_adr = inp(pat_adr++);
					map_adr += (32 - 1);
					*(map_adr++) = inp(pat_adr++);
					*map_adr = inp(pat_adr);
					map_adr -= (32 - 1);

/*					map_data[ii][jj] = inp(pat_adr++);
					map_data[ii+1][jj] = inp(pat_adr++);
					map_data[ii][jj+1] = inp(pat_adr++);
					map_data[ii+1][jj+1] = inp(pat_adr);
*/					++data;
				}
//				outp(0x1fd0, *WK1FD0); /* 元に戻す */
				outp(0x1fd0, 0); /* 元に戻す */
				EI();
				data_tmp += MAP_SIZE_X;
				data = data_tmp;
				map_adr += (32 * 2 - X_SIZE);
			}

			dir2 = 1 - dir2;
			map_adr = &map_data[1 + 1 * 32];
			old_map_adr = &old_map_data[1 + 1 * 32];

			if((old_x != x)){
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
						vram_ofs += SIZE;
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
						vram_ofs += PARTS_X;
						++map_adr;
						++old_map_adr;
					}
					vram_ofs += (SIZE - (PARTS_X) * (X_SIZE - 2));
					map_adr += (32 - (X_SIZE - 2));
					old_map_adr += (32 - (X_SIZE - 2));
				}
			}
		}
		old_x = x;
		old_y = y;
	}while(!(k2 & 0x80) && (k3 != 27));
	printf("End.\n");

	/* Pallet */
	outp(0x1000, 0xaa);
	outp(0x1100, 0xcc);
	outp(0x1200, 0xf0);

	/* Priority */
	outp(0x1300, 0x00);
}

