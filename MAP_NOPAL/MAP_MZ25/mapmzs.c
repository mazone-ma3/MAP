/* z88dk MZ2500  PCGキャラMAP移動実験 2x2圧縮展開 ZSDCC版 By m@3 */
/* キャラを出す */

#include <stdio.h>
#include <stdlib.h>

#define VRAM_ADR 0xa000
#define MAP_ADR 0xa000
#define PARTS_DATA (MAP_ADR+0x3c00)
#define CHRPAT_ADR 0xe000

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

unsigned char *vram_ofs;
unsigned char map_data[(X_SIZE+2) * 32];


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
unsigned short i, j;


void vram_off(void){
	outp(0xb4,5);
	outp(0xb5,a);
	outp(0xb4,6);
	outp(0xb5,b);
}

void vram_i_on(void){
	outp(0xb4,5);
	outp(0xb5,0x26);
	outp(0xb4,6);
	outp(0xb5,0x27);
}

void vram_b_on(void){
	outp(0xb4,5);
	outp(0xb5,0x20);
	outp(0xb4,6);
	outp(0xb5,0x21);
}

void vram_r_on(void){
	outp(0xb4,5);
	outp(0xb5,0x22);
	outp(0xb4,6);
	outp(0xb5,0x23);
}

void vram_g_on(void){
	outp(0xb4,5);
	outp(0xb5,0x24);
	outp(0xb4,6);
	outp(0xb5,0x25);
}


/* 3プレーン転送 */
void put_chrmz_pat(unsigned short patadr)
{
	unsigned char *adr_tmp, *adr_tmp2;
	unsigned short ii,jj;

	adr_tmp = (unsigned char *)(CHRPAT_ADR + patadr * 2 + patadr);
	adr_tmp2 = (unsigned char *)(VRAM_ADR + ((OFS_X - 1 + i) * PARTS_X) + ((OFS_Y - 1 + j) * PARTS_Y) * SIZE);
	for(jj = 0 ; jj < PARTS_Y; ++jj){
		for(ii = 0 ; ii < PARTS_X; ++ii){
			DI();
			vram_b_on();
			*adr_tmp2 = *(adr_tmp++);
			vram_r_on();
			*adr_tmp2 = *(adr_tmp++);
			vram_g_on();
			*adr_tmp2 = *(adr_tmp++);
			vram_off();
			++adr_tmp2;
			EI();
		}
		adr_tmp += (32 * 3 - PARTS_X * 3);
		adr_tmp2 += (SIZE - PARTS_X);
	}
}

void pat_sub(void)
{
	DI();
	/* TextRam ON */
	outp(0xb4,5);
	outp(0xb5,0x38);

	vram_ofs[0] = no * 2;
	vram_ofs[1] = no * 2 + 1;

	vram_off();
	EI();
}

void pat_sub2(void)
{
	DI();
	/* TextRam ON */
	outp(0xb4,5);
	outp(0xb5,0x38);

	vram_ofs[0] = 18;
	vram_ofs[1] = 18;

	vram_ofs[0x800] = 0x0f;
	vram_ofs[0x801] = 0x0f;
	vram_ofs[0x1000] = 0x00;
	vram_ofs[0x1001] = 0x00;

	vram_off();
	EI();
}

void chr_sub(void)
{
	unsigned char no2 = chr_tbl[dir * 2 + dir2][(i - CHR_X) + (j - CHR_Y) * 2];

	put_chrmz_pat((no2 & 0x0f) * 2 + (no2 & 0xf0) * 16);

	pat_sub();
}

unsigned char colortbl[16] = {
	0x00, 0x19, 0x1a, 0x10, 0x1c, 0x1d, 0x1e, 0x1f,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
};

unsigned char k1, k2, k3, k7, ka;
unsigned char  st, data_no;
unsigned char *data, *data_tmp;

unsigned short pat_no;
unsigned char *pat_adr;
unsigned short x = 165, y = 30,xx, yy, old_x = 255, old_y = 255, k;
//unsigned char ii, jj;
unsigned char *vram_ofs_tmp;

unsigned char old_map_data[(X_SIZE + 2) * 32];
unsigned char sub_flag;
unsigned char *map_adr;
unsigned char *old_map_adr;

void main(void)
{
	outp(0xcf, 0);

	outp(0xb4,5);
	a = inp(0xb5);
	outp(0xb4,6);
	b = inp(0xb5);

	/* Change Pallet & Priority */
	for(i = 0; i < 16; ++i){
		outp(0xf4, 0x80 + i);
		outp(0xf5, colortbl[i]);
	}

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}
	for(i = 0; i < X_SIZE - 2; ++i){
		for(j = 0; j < Y_SIZE - 2; ++j){
			vram_ofs = (unsigned char *)(VRAM_ADR +(OFS_X + i) * PARTS_X + (OFS_Y + j) * SIZE);
			pat_sub2();
		}
	}

	do{
		outp(0xe8, 0x31);
		k1 =inp(0xea);

		outp(0xe8, 0x32);
		k2 =inp(0xea);

		outp(0xe8, 0x33);
		k3 =inp(0xea);

		outp(0xe8, 0x37);
		k7 =inp(0xea);

		outp(0xe8, 0x3a);
		ka =inp(0xea);

		st = inp(0xef);

		if(!(k1 & 0x04) || !(k3 & 0x08) || !(st & 0x01)){ /* 8 */
			if(y > 0){
				--y;
				dir = 0;
			}
		}
		if(!(k2 & 0x40) || !(k3 & 0x40) || !(st & 0x08)){ /* 6 */
			if(x < (MAP_SIZE_X*2 - X_SIZE)){
				++x;
				dir = 1;
			}
		}
		if(!(k2 & 0x04) || !(k3 & 0x10) || !(st & 0x02)){ /* 2 */
			if(y < (MAP_SIZE_Y*2 - Y_SIZE)){
				++y;
				dir = 2;
			}
		}
		if(!(k2 & 0x10) || !(k3 & 0x20) || !(st & 0x04)){ /* 4 */
			if(x > 0){
				--x;
				dir = 3;
			}
		}
		if(!(k3 & 0x02) || !(k7 & 0x04) || !(st & 0x10)) /* Z,SPACE */
			if(!(k7 & 0x01) || !(st & 0x20)) /* X */
				break;

		if((old_x != x) || (old_y != y)){
			xx = 1 - x % 2;
			yy = 1 - y % 2;
			data = (unsigned char *)(MAP_ADR + (x / 2) + (y / 2) * MAP_SIZE_X);
			data_tmp = data;
			vram_ofs = (unsigned char *)(VRAM_ADR +(OFS_X) * PARTS_X + (OFS_Y) * SIZE);
			vram_ofs_tmp = vram_ofs;
			map_adr = &map_data[xx + yy * 32];

			for(j = 0; j < Y_SIZE / 2; ++j){

				for(i = 0; i < X_SIZE / 2; ++i){
					data_no = *data;

					pat_no = ((data_no >> 4) & 0x0f) | ((data_no << 4) & 0xf0);
					pat_adr = (unsigned char *)((PARTS_DATA + pat_no * 4));
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
	}while((ka & 0x20));

	/* Change Pallet & Priority */
	for(i = 0; i < 16; ++i){
		outp(0xf4, 0x80 + i);
		outp(0xf5,i);
	}
}

