/* PC-88 CHR88.BAS作成キャラMAP移動実験 やっぱALU使わない 2x2圧縮展開 By m@3 */
/* キャラを出す */
/* 転送部インラインアセンブラ化 */

#include <stdio.h>
#include <stdlib.h>

#define VRAM_ADR 0xc000
#define MAINRAM_ADR 0x2000
#define MAP_ADR 0x4000
#define VRAM_DATA_ADR 0xc700
#define PARTS_DATA (MAP_ADR+0x3c00)
#define MASK_DATA_ADR 0x1600
#define VRAM_TMP_ADR 0xfe80

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

unsigned char map_data[(X_SIZE+2) * 32];

#define MAXCOLOR 8

unsigned char org_pal[MAXCOLOR][3] =
	{{ 0, 0, 0 },
	{ 0, 0, 15 },
	{ 15, 0, 0 },
	{ 15, 0, 15 },
	{ 0, 15, 0 },
	{ 0, 15, 15 },
	{ 15, 15, 0 },
	{ 15, 15, 15 },};

unsigned char romport;

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

void Set_RAM_MODE(void){
#asm
	DI
	ld	a,($e6c2)
	or	$02
	out($31),a
#endasm
}

void Set_ROM_MODE(void){
#asm
	ld	a,($e6c2)
	out($31),a
	EI
#endasm
}

void set_mainROM(void)
{
	romport = inp(0x71);
	outp(0x71, 0xff);
}

void reset_mainROM(void)
{
	outp(0x71, romport);
}

void  put_chr88_pat(unsigned char *mainram, unsigned char *vram)
{
#asm
	ld	hl, 2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)	; de=vram
	inc	hl
	ld	c,(hl)
	inc	hl
	ld	b,(hl)	; bc = mainram
	ld	l,c
	ld	h,b	; hl = mainram

	ld	b,8

looppat:
	DI
	ld	a,($e6c2)
	or	$02
	out	($31),a

	out	($5c),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	out	($5d),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	out	($5e),a
	ld	a,(hl)
	ld	(de),a
	inc	hl

	inc	de
	out	($5c),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	out	($5d),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	out	($5e),a
	ld	a,(hl)
	ld	(de),a

	out	($5f),a

	ld	a,($e6c2)
	out($31),a
	EI

	push	de	;	hl = mainram
	ld	de, 91 ;((256 / 8 - 2) * 3 + 1)
	add	hl,de
	pop	de

	push	hl	;de = vram
	ex	de,hl
	ld	de,79 ;(80 - 1)
	add	hl,de
	ex	de,hl
	pop	hl

	djnz looppat

#endasm
}

void  put_chr88_pat2(unsigned char *mainram, unsigned char *vram)
{
#asm
	ld	hl, 2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)	; de=vram
	inc	hl
	ld	c,(hl)
	inc	hl
	ld	b,(hl)	; bc = mainram
	ld	l,c
	ld	h,b	; hl = mainram

	ld	b,8

looppat2:
	DI
	ld	a,($e6c2)
	or	$02
	out	($31),a

	out	($5c),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	out	($5d),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	out	($5e),a
	ld	a,(hl)
	ld	(de),a
	inc	hl

	inc	de
	out	($5c),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	out	($5d),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	out	($5e),a
	ld	a,(hl)
	ld	(de),a

	out	($5f),a

	ld	a,($e6c2)
	out($31),a
	EI

	push	de	;	hl = mainram
	ld	de, 91 ;((256 / 8 - 2) * 3 + 1)
	add	hl,de
	pop	de

	inc	de	;de = vram

	djnz looppat2

#endasm
}

void  put_chr_or(unsigned char *mainram, unsigned char *vram)
{
#asm
	ld	hl, 2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)	; de=vram
	inc	hl
	ld	c,(hl)
	inc	hl
	ld	b,(hl)	; bc = mainram
	ld	l,c
	ld	h,b	; hl = mainram

	ld	b,8
;	ld	c,0

loopor:
	DI
	ld	a,($e6c2)
	or	$02
	out	($31),a

	out	($5c),a
	ld	a,(de)
	out	($5f),a
	or	(hl)
	out	($5c),a
	ld	(de),a
	inc	hl
	out	($5d),a
	ld	a,(de)
	out	($5f),a
	or	(hl)
	out	($5d),a
	ld	(de),a
	inc	hl
	out	($5e),a
	ld	a,(de)
	out	($5f),a
	or	(HL)
	out	($5e),a
	ld	(de),a
	inc	hl

	inc	de
	out	($5c),a
	ld	a,(de)
	out	($5f),a
	or	(hl)
	out	($5c),a
	ld	(de),a
	inc	hl
	out	($5d),a
	ld	a,(de)
	out	($5f),a
	or	(hl)
	out	($5d),a
	ld	(de),a
	inc	hl
	out	($5e),a
	ld	a,(de)
	out	($5f),a
	or	(HL)
	out	($5e),a
	ld	(de),a

	out	($5f),a

	ld	a,($e6c2)
	out($31),a
	EI

	push	de	;	hl = mainram
	ld	de, 91 ;((256 / 8 - 2) * 3 + 1)
	add	hl,de
	pop	de

	inc	de	;de = vram

	djnz loopor

#endasm
}

void mask_ALU(unsigned char *mainram, unsigned char *vram)
{
#asm
	ld	hl, 2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)	; de=vram
	inc	hl
	ld	c,(hl)
	inc	hl
	ld	b,(hl)	; bc = mainram

	ld	l,c
	ld	h,b	; hl = mainram


	ld	b,8
looppat3:
	DI
	ld	a,($e6c2)
	or	$02
	out	($31),a

	in	a,($32)
	or	$40
	out	($32),a	; USE ALU
	ld	a,$80
	out	($35),a	; Access GVRAM

	xor	a
	out	($34),a

	ld	a,(hl)
	ld	(de),a

	inc	hl

	inc	de
	ld	a,(hl)
	ld	(de),a

	xor a
	out($35),a	; Access MAINRAM
	in	a,($32);
	and $bf;
	out($32),a	; NOUSE ALU

	ld	a,($e6c2)
	out($31),a
	EI

	push	de	;	hl = mainram
	ld	de, 31 ;((256 / 8 - 2) * 1 + 1)
	add	hl,de
	pop	de

	inc	de	;de = vram

	djnz looppat3

#endasm
}


void copy_ALU(unsigned char *srcvram, unsigned char *dstvram)
{
#asm
	DI
	ld	hl, 2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)	; de = dstvram
	inc	hl
	ld	c,(hl)
	inc	hl
	ld	b,(hl)	; bc = srcvram

	ld	l,c
	ld	h,b	; hl = srcvram

	ld	a,($e6c2)
	or	$02
	out	($31),a

	in	a,($32)
	or	$40
	out	($32),a	; USE ALU
	ld	a,$90
	out	($35),a	; Access GVRAM

	ldi			;1
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;2
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;3
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;4
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;5
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;6
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;7
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;8
	ld	a,(hl)
	ld	(de),a

	xor a
	out($35),a	; Access MAINRAM
	in	a,($32);
	and $bf;
	out($32),a	; NOUSE ALU

	ld	a,($e6c2)
	out($31),a
	EI
#endasm
}


unsigned char no;
unsigned char *vram_adr;
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

void chr_sub(void)
{
	unsigned short pat_tmp, pat_tmp2;
	unsigned short no2 = chr_tbl[dir * 2 + dir2][(i - CHR_X) + (j - CHR_Y) * 2];

	unsigned char *vram_tmp_adr = (unsigned char *)VRAM_TMP_ADR;
	pat_tmp2 = (no2 & 0x0f) * 2 + (no2 & 0xf0) * 16;
	pat_tmp = (no & 0x0f) * 2 + (no & 0xf0) * 16;
	put_chr88_pat2((unsigned char *)(MAINRAM_ADR + pat_tmp * 2 + pat_tmp), vram_tmp_adr);
/* マスクを書く */
	mask_ALU((unsigned char *)(MASK_DATA_ADR + pat_tmp2), vram_tmp_adr);

	put_chr_or((unsigned char *)(VRAM_DATA_ADR + pat_tmp2 * 2 + pat_tmp2), vram_tmp_adr);

	copy_ALU(vram_tmp_adr,vram_adr);
}

void pat_sub(void)
{
	unsigned short pat_tmp;
	pat_tmp = (no & 0x0f) * 2 + (no & 0xf0) * 16;
	put_chr88_pat((unsigned char *)(MAINRAM_ADR + pat_tmp * 2 + pat_tmp), vram_adr);
}

void wait_vsync(void);

/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	outp(0x54 + color, red * 8 | blue);
	outp(0x54 + color, green | 0x40);
}

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned short i;
	for(i = 0; i < MAXCOLOR; i++)
		pal_set(pal_no, i, color[i][0]/2, color[i][1]/2, color[i][2]/2);
}

void wait_vsync(void)
{
	while((inp(0x40) & 0x20)); /* WAIT VSYNC */
	while(!(inp(0x40) & 0x20));
}

void sys_wait(unsigned char wait)
{
	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync();
}

//value < 0 黒に近づける。
//value = 0 設定した色
//value > 0 白に近づける。
void set_constrast(int value, unsigned char org_pal[MAXCOLOR][3], int pal_no)
{
	unsigned char j, k;
	unsigned char pal[3];

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
		if(j == (MAXCOLOR / 2))
			wait_vsync();
		pal_set(pal_no, j, pal[0]/2, pal[1]/2, pal[2]/2);
	}
}

//wait値の速度で黒からフェードインする。
void fadeinblack(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = -15; j <= 0; j++){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//wait値の速度で黒にフェードアウトする。
void fadeoutblack(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = 0; j != -16; j--){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//wait値の速度で白にフェードアウトする。
void fadeoutwhite(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = 0; j < 16; j++){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//パレットを暗転する。
void pal_allblack(int pal_no)
{
	unsigned char j;
	for(j = 0; j < MAXCOLOR; j++)
		pal_set(pal_no, j, 0, 0, 0);
}

void main(void)
{
	unsigned char k0, k1, k8, ka, st, pd, k5, k9, *data, *data_tmp;
	unsigned char pat_no, *pat_adr;
	unsigned char x = 165, y = 30,xx, yy, old_x = 255, old_y = 255,, k;
//	unsigned char ii, jj;
	unsigned char *vram_adr_tmp;

	unsigned char old_map_data[(X_SIZE + 2) * 32];
	unsigned char sub_flag;
	unsigned char *map_adr;
	unsigned char *old_map_adr;

	unsigned char fadeflag = 0;

//	outp(0x32, inp(0x32) | 0x20);	/* 512 colors */
	pal_allblack(0);

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}

	while(((k9 = inp(0x09)) & 0x80)){ /* ESC */
		k0 = inp(0x00);
		k1 = inp(0x01);
		k8 = inp(0x08);
		ka = inp(0x0a);
		DI();
		outp(0x44, 0x0e);
		st = inp(0x45);
		outp(0x44, 0x0f);
		pd = inp(0x45);
		EI();
		k5 = inp(0x05);
		if(!(k1 & 0x01) || !(k8 & 0x02) || !(st & 0x01)){ /* 8 */
			if(y > 0){
				--y;
				dir = 0;
			}
		}
		if(!(k0 & 0x40) || !(k8 & 0x04) || !(st & 0x08)){ /* 6 */
			if(x < (MAP_SIZE_X*2 - X_SIZE)){
				++x;
				dir = 1;
			}
		}
		if(!(k0 & 0x04) || !(ka & 0x02) || !(st & 0x02)){ /* 2 */
			if(y < (MAP_SIZE_Y*2 - Y_SIZE)){
				++y;
				dir = 2;
			}
		}
		if(!(k0 & 0x10) || !(ka & 0x04) || !(st & 0x04)){ /* 4 */
			if(x > 0){
				--x;
				dir = 3;
			}
		}
		if(!(k5 & 0x04) || !(k9 & 0x40) || !(pd & 0x01)) /* Z,SPACE */
			if(!(k5 & 0x01) || !(pd & 0x02)) /* X */
				break;

		if((old_x != x) || (old_y != y)){
			xx = 1 - x % 2;
			yy = 1 - y % 2;
			data = (unsigned char *)(MAP_ADR + (x / 2) + (y / 2) * MAP_SIZE_X);
			data_tmp = data;
			vram_adr = (unsigned char *)(VRAM_ADR + (OFS_X * PARTS_X + SIZE * PARTS_Y * OFS_Y));
			vram_adr_tmp = vram_adr;
			map_adr = &map_data[xx + yy * 32];

			for(j = 0; j < Y_SIZE / 2; ++j){

				for(i = 0; i < X_SIZE / 2; ++i){
					Set_RAM_MODE();
					pat_no = ((*data >> 4) & 0x0f) | ((*data << 4) & 0xf0);
					pat_adr = (unsigned char *)(PARTS_DATA + pat_no * 4);
//					ii = i * 2 + xx;
//					jj = j * 2 + yy;
					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr++);
					map_adr += (32 - 1);
					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr);
					map_adr -= (32 - 1);

/*					map_data[ii][jj] = *(pat_adr++);
					map_data[ii+1][jj] = *(pat_adr++);
					map_data[ii][jj+1] = *(pat_adr++);
					map_data[ii+1][jj+1] = *(pat_adr);
*/					++data;
					Set_ROM_MODE();
				}
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
						vram_adr += SIZE * PARTS_Y;
						map_adr += 32;
						old_map_adr += 32;
					}
					vram_adr_tmp += PARTS_X;
					vram_adr = vram_adr_tmp;
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
						vram_adr += PARTS_X;
						++map_adr;
						++old_map_adr;
					}
					vram_adr += (SIZE * PARTS_Y - (PARTS_X) * (X_SIZE - 2));
					map_adr += (32 - (X_SIZE - 2));
					old_map_adr += (32 - (X_SIZE - 2));
				}
			}
		}
		old_x = x;
		old_y = y;

		if(fadeflag == 0){
			fadeflag = 1;
			fadeinblack(org_pal, 0, 3);
		}
	}
	fadeoutblack(org_pal, 0, 3);
	pal_all(0, org_pal);
}

