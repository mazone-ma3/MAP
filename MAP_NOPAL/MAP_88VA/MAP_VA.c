/* PC-88VA GCC-ia16 キャラMAP移動実験  2x2圧縮展開 By m@3 */
/* キャラを出す */
#define _BORLANDC_SOURCE

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <i86.h>
#include <fcntl.h>
#include <dos.h>
#include <unistd.h>

/************************************************************************/
/*		BIT操作マクロ定義												*/
/************************************************************************/

/* BITデータ算出 */
#define BITDATA(n) (1 << (n))

/* BITセット */
#define BITSET(BITNUM, NUMERIC) {	\
	NUMERIC |= BITDATA(BITNUM);		\
}

/* BITクリア */
#define BITCLR(BITNUM, NUMERIC) {	\
	NUMERIC &= ~BITDATA(BITNUM);	\
}

/* BITチェック */
#define BITTST(BITNUM, NUMERIC) (NUMERIC & BITDATA(BITNUM))

/* BIT反転 */
#define BITNOT(BITNUM, NUMERIC) {	\
	NUMERIC ^= BITDATA(BITNUM);		\
}

#define PARTS_HEAD 0x3c00 /*組み合わせキャラデータの先頭番地*/
#define BUFFSIZE 16384

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

unsigned char mapdata[BUFFSIZE];

/*疑似BG処理と基本的な画面操作各種*/

#define MAXCOLOR 16
#define ON 1
#define OFF 0

void get_8(unsigned short ,unsigned short ,unsigned short);
void put_8(unsigned short ,unsigned short ,unsigned short);
void g_init(void);
void end(void);
void clear(unsigned short);
void pal_set(unsigned short,unsigned char,unsigned char,unsigned char);
void pal_all(unsigned char[16][3]);

typedef struct pattern_8{
	unsigned short b_vram[8];
	unsigned short r_vram[8];
	unsigned short g_vram[8];
	unsigned short i_vram[8];
} chr8;

chr8 chr_8[256];

chr8 chr_8_2[256];

unsigned short chr_8_mask[8][256];

#define X_SIZE 18
#define Y_SIZE 18

#define MAP_SIZE_X 128
#define MAP_SIZE_Y 128

#define OFS_X 2
#define OFS_Y 2

#define CHR_X 8
#define CHR_Y 8

unsigned char map_data[(X_SIZE+2) * 32];
unsigned short x = 165, y = 30,xx, yy, old_x = 255, old_y = 255;
unsigned char map_no, map_no2;
unsigned char dir = 2, dir2 = 0, i, j, k;
unsigned short vx, vy;

unsigned char __far *bvram, // = (unsigned char __far *)MK_FP(0xa000, 0),
	 *rvram, // = (unsigned char __far *)MK_FP(0xb000, 0),
	 *gvram, // = (unsigned char __far *)MK_FP(0xc000, 0),
	 *ivram; // = (unsigned char __far *)MK_FP(0xd000, 0);


#define WIDTH 32
#define LINE 128 //200
#define RAM_ADR 0x2000


unsigned char __far *flame[4]
	 = {MK_FP(0xa000,0)	,MK_FP(0xb000,0),MK_FP(0xc000,0),MK_FP(0xd000,0)};

FILE *stream[2];

unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 , 15};


unsigned short conv(char *loadfil)
{
	long i, j,count, count2;
	unsigned short k=0, l=0;
	unsigned char pattern[100];
	unsigned char pc88vacolor[4];
	unsigned char msxcolor[8];
	unsigned char color;
	unsigned short header;

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}
	fread(pattern, 1, 1, stream[0]);	/* MSX先頭を読み捨てる */
	fread(pattern, 1, 4, stream[0]);	/* MSXヘッダも読み捨てる */

	fread(pattern, 1, 2, stream[0]);	/* MSXヘッダを読み捨てる */

	for(count = 0; count < LINE; ++count){
		for(count2 = 0; count2 < WIDTH; ++count2){
	
			i = fread(pattern, 1, 2, stream[0]);	/* 4dot分 */
			if(i < 1)
				break;

			/* 色分解と拡大 */
			msxcolor[0] = (pattern[0] >>4) & 0x0f;
			msxcolor[1] = (pattern[0] >>4) & 0x0f;
			msxcolor[2] = pattern[0] & 0x0f;
			msxcolor[3] = pattern[0] & 0x0f;
			msxcolor[4] = (pattern[1] >>4) & 0x0f;
			msxcolor[5] = (pattern[1] >>4) & 0x0f;
			msxcolor[6] = pattern[1] & 0x0f;
			msxcolor[7] = pattern[1] & 0x0f;
			for(i = 0; i < 4; ++i){
				pc88vacolor[i] = 0;
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 4; ++i){
					color = conv_tbl[msxcolor[j]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, pc88vacolor[i]);
					}else{
						BITCLR(7-j, pc88vacolor[i]);
					}
				}
			}

			i = fread(pattern, 1, 2, stream[0]);	/* 4dot分 */
			if(i < 1)
				break;

			/* 色分解と拡大 */
			msxcolor[0] = (pattern[0] >>4) & 0x0f;
			msxcolor[1] = (pattern[0] >>4) & 0x0f;
			msxcolor[2] = pattern[0] & 0x0f;
			msxcolor[3] = pattern[0] & 0x0f;
			msxcolor[4] = (pattern[1] >>4) & 0x0f;
			msxcolor[5] = (pattern[1] >>4) & 0x0f;
			msxcolor[6] = pattern[1] & 0x0f;
			msxcolor[7] = pattern[1] & 0x0f;

			for(i = 0; i < 4; ++i){
				pattern[i] = pc88vacolor[i];
			}

			for(i = 0; i < 4; ++i){
				pc88vacolor[i] = 0;
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 4; ++i){
					color = conv_tbl[msxcolor[j]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, pc88vacolor[i]);
					}else{
						BITCLR(7-j, pc88vacolor[i]);
					}
				}
			}

			for(i = 0; i < 4; ++i){
				pattern[4 + i] = pc88vacolor[i];
			}

			*(flame[0] + 0 + k + l) = pattern[0];
			*(flame[1] + 0 + k + l) = pattern[1];
			*(flame[2] + 0 + k + l) = pattern[2];
			*(flame[3] + 0 + k + l) = pattern[3];
			*(flame[0] + 1 + k + l) = pattern[4];
			*(flame[1] + 1 + k + l) = pattern[5];
			*(flame[2] + 1 + k + l) = pattern[6];
			*(flame[3] + 1 + k + l) = pattern[7];

			k += 2;
			if(k >= 64){
				k = 0;
				l += 80;
			}
		}
	}
	fclose(stream[0]);

	return 0;
}


/*グラフィック画面から構造体配列pattern_8に読み込む*/
void get_8(unsigned short x, unsigned short y ,unsigned short no)
{
	unsigned short j, c;

	for (j = 0; j < 8; j++){
		c = x / 8 + y * 80 + j * 80;
		chr_8[no].b_vram[j] = *((unsigned short __far *)(unsigned char __far *)(bvram + c));
		chr_8[no].r_vram[j] = *((unsigned short __far *)(unsigned char __far *)(rvram + c));
		chr_8[no].g_vram[j] = *((unsigned short __far *)(unsigned char __far *)(gvram + c));
		chr_8[no].i_vram[j] = *((unsigned short __far *)(unsigned char __far *)(ivram + c));
	}
}

void get_8_2(unsigned short x, unsigned short y ,unsigned short no)
{
	unsigned short j, c;

	for (j = 0; j < 8; j++){
		c = x / 8 + y * 80 + j * 80;
		chr_8_2[no].b_vram[j] = *((unsigned short __far *)(unsigned char __far *)(bvram + c));
		chr_8_2[no].r_vram[j] = *((unsigned short __far *)(unsigned char __far *)(rvram + c));
		chr_8_2[no].g_vram[j] = *((unsigned short __far *)(unsigned char __far *)(gvram + c));
		chr_8_2[no].i_vram[j] = *((unsigned short __far *)(unsigned char __far *)(ivram + c));
	}
}


void get_8_mask(unsigned short x, unsigned short y ,unsigned short no)
{
	unsigned short j, c;

	for (j = 0; j < 8; j++){
		c = x / 8 + y * 80 + j * 80;
		chr_8_mask[j][no] = ~(
			*((unsigned short __far *)(unsigned char __far *)(bvram + c)) |
			*((unsigned short __far *)(unsigned char __far *)(rvram + c)) |
			*((unsigned short __far *)(unsigned char __far *)(gvram + c)) |
			*((unsigned short __far *)(unsigned char __far *)(ivram + c)));
	}
}

/*構造体配列pattern_8の内容を画面に書き出す*/
void put_8(unsigned short x, unsigned short y, unsigned short no)
{
	unsigned short j, c;

	for (j = 0; j < 8; j++){
		c = x / 8 + y * 80 + j * 80;
		*((unsigned short __far *)(unsigned char __far *)(bvram + c))
			= chr_8[no].b_vram[j];
		*((unsigned short __far *)(unsigned char __far *)(rvram + c))
			= chr_8[no].r_vram[j];
		*((unsigned short __far *)(unsigned char __far *)(gvram + c))
			= chr_8[no].g_vram[j];
		*((unsigned short __far *)(unsigned char __far *)(ivram + c))
			= chr_8[no].i_vram[j];
	}
}

void put_8_chr(unsigned short x, unsigned short y, unsigned short no, unsigned short no2)
{
	unsigned short j, c;

	for (j = 0; j < 8; j++){
		c = x / 8 + y * 80 + j * 80;
		*((unsigned short __far *)(unsigned char __far *)(bvram + c)) = 
			(chr_8[no].b_vram[j] & chr_8_mask[j][no2] | chr_8_2[no2].b_vram[j]);
		*((unsigned short __far *)(unsigned char __far *)(rvram + c)) =
			(chr_8[no].r_vram[j] & chr_8_mask[j][no2] | chr_8_2[no2].r_vram[j]);
		*((unsigned short __far *)(unsigned char __far *)(gvram + c)) =
			(chr_8[no].g_vram[j] & chr_8_mask[j][no2] | chr_8_2[no2].g_vram[j]);
		*((unsigned short __far *)(unsigned char __far *)(ivram + c)) =
			(chr_8[no].i_vram[j] & chr_8_mask[j][no2] | chr_8_2[no2].i_vram[j]);
	}
}


void g_init(void)
{
//	outportw(0x100, 0xb000);	/* none-interless Graphic-on 400dot(400line) */
//	outportw(0x100, 0xb020);	/* none-interless Graphic-on 200dot(200line) */
	outportw(0x100, 0xb062);	/* none-interless Graphic-on 400dot(200line) */
								/* 画面ON notsingle-plane 1画面 */

	outportw(0x102, 0x0101);	/* graphic0 Width640 4dot/pixel */
								/* graphic1 Width640 4dot/pixel */

	outportb(0x153, 0x44);		/* G-VRAM選択 */

	outportw(0x106, 0xab90);	/* パレット指定画面割当て指定 */
	outportw(0x108, 0x0000);	/* 直接色指定画面割当て設定 */
	outportw(0x110, 0x008f);	/* 4ビットピクセル */
//	outportb(0x500, 0);	/* 独立アクセス */
//	outportb(0x512, 0);	/* ブロック0 */
//	outportb(0x516, 0);	/* 書き込みプレーン選択 */
}

/*終了処理*/
void end()
{
	outportw(0x100, 0xb000);	/* none-interless Graphic-on 400dot(400line) */
	outportb(0x153, 0x41);		/* T-VRAM選択 */
	outportw(0x106, 0xab98);	/* パレット指定画面割当て指定 */
}


void paint(unsigned char pattern)
{
	unsigned short i, j;

	for (i = 0; i < (80 * 200L); ++i){
		for(j = 0; j < 4; j++){
			*(flame[j] + i) = pattern;
		}
	}
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(unsigned short type)
{
	if(type & 1)
		paint(0x0);

//	if(type & 2)
//		printf("\x1b*");
}

/*パレット・セット*/
void pal_set(unsigned short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	outportw(0x300 + color * 2, (unsigned short)green * 4096 + red * 64 + blue * 2);
}

void pal_all(unsigned char color[16][3])
{
	unsigned short i;
	for(i = 0; i < MAXCOLOR; i++)
		pal_set(i, color[i][0], color[i][1], color[i][2]);
}


/*マップデータをファイルから配列に読み込む*/
unsigned short mapload(unsigned char *fil)
{
	unsigned short inhandle;

	if ((inhandle = open( fil, O_RDONLY )) == -1) { //| O_BINARY
		printf("\x01b[31mCan\'t open file %s.\x01b[37m", fil);
		return ERROR;
	}
	read( inhandle, mapdata, 7);
	read( inhandle, mapdata, BUFFSIZE);
	close(inhandle);
	return NOERROR;
}


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

void chr_sub(void)
{
	unsigned short no2 = chr_tbl[dir * 2 + dir2][(i - CHR_X) + (j - CHR_Y) * 2];
	put_8_chr(vx , vy,	map_no, no2);
}

void pat_sub(void)
{
	put_8(vx, vy, map_no);
}

/*メインルーチン
　初期設定とメインループ*/
void main(void)
{
	unsigned short mode = 1;
	unsigned char pal[16][3] = {
		{  0,  0,  0},
		{  0,  0,  0},
		{  3, 13,  3},
		{  7, 15,  7},
		{  3,  3, 15},
		{  5,  7, 15},
		{ 11,  3,  3},
		{  5, 13, 15},
		{ 15,  3,  3},
		{ 15,  7,  7},
		{ 13, 13,  3},
		{ 13, 13,  7},
		{  3,  9,  3},
		{ 13,  5, 11},
		{ 11, 11, 11},
		{ 15, 15, 15},
	};

	unsigned char old_map_data[(X_SIZE + 2) * 32];
	unsigned char sub_flag;
	unsigned char *map_adr;
	unsigned char *old_map_adr;

	unsigned char k0, k1, k8, ka, st, pd, k5, k9;

	unsigned char *data, *data_tmp;
	unsigned char pat_no, *pat_adr;

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}
	bvram = (unsigned char __far *)MK_FP(0xa000, 0),
	rvram = (unsigned char __far *)MK_FP(0xb000, 0),
	gvram = (unsigned char __far *)MK_FP(0xc000, 0),
	ivram = (unsigned char __far *)MK_FP(0xd000, 0);

	g_init();
	pal_all(pal);

	if(conv("ELMAPCHR.SC5")){
		end();
		exit(1);
	}

/*裏画面からパーツデータを取り出す。*/

	for ( j = 0; j < 16; j++){
		for ( i = 0; i < 16; i++){
			get_8(i * 16, j * 8, i + j * 16);
		}
	}

	for ( j = 0; j < 16; j++){
		for ( i = 0; i < 16; i++){
			get_8_2(i * 16 + 256, j * 8, i + j * 16);
		}
	}

	for ( j = 0; j < 16; j++){
		for ( i = 0; i < 16; i++){
			get_8_mask(i * 16 + 256, j * 8, i + j * 16);
		}
	}

	clear(3);

/*マップのロード to (unsigned char mapdata[])*/

	if (mapload("ELMSTMAP.mdt")){
		end();
		exit(1);
	}

	_disable();
	outportb(0x44, 0x07);
	outportb(0x45, 0x00);
	_enable();


/* ESCで抜けるまでループ*/
	while(((k9 = inportb(0x09)) & 0x80)){ /* ESC */
		k0 = inportb(0x00);
		k1 = inportb(0x01);
		k8 = inportb(0x08);
		ka = inportb(0x0a);
		_disable();
		outportb(0x44, 0x0e);
		st = inportb(0x45);
		outportb(0x44, 0x0f);
		pd = inportb(0x45);
		_enable();
		k5 = inportb(0x05);
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

			data = &mapdata[x / 2 + (y / 2) * MAP_SIZE_X];
			data_tmp = data;
			map_adr = &map_data[xx + yy * 32];

			for(j = 0; j < Y_SIZE / 2; ++j){
				for(i = 0; i < X_SIZE / 2; ++i){
//					data = mapdata[x / 2 + i + (j + y / 2) * 128];
//					pat_no = ((data >> 4) & 0x0f) | ((data << 4) & 0xf0);
					pat_no = ((*data >> 4) & 0x0f) | ((*data << 4) & 0xf0);
					pat_adr = &mapdata[PARTS_HEAD + pat_no * 4];

					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr++);
					map_adr += (32 - 1);
					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr);
					map_adr -= (32 - 1);

//					for(k = 0; k < 4; ++k){
//						map_data[i * 2 + (k % 2) + xx][j * 2 + (k / 2) + yy] = mapdata[PARTS_HEAD + pat_no * 4 + k];
//					}
					++data;
				}
				data_tmp += MAP_SIZE_X;
				data = data_tmp;
				map_adr += (32 * 2 - X_SIZE);
			}
			dir2 = 1 - dir2;

			vx = (OFS_X) * 16;
			vy = (OFS_Y) * 8;
			map_adr = &map_data[1 + 1 * 32];
			old_map_adr = &old_map_data[1 + 1 * 32];

			if(old_x != x){
				for(i = 1; i < (X_SIZE - 1); ++i, vx += 16){
					for(j = 1, vy = (OFS_Y) * 8; j < (Y_SIZE - 1); ++j, vy += 8){
						map_no = *map_adr;
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
						}else if(map_no != *old_map_adr){
							pat_sub();
							*old_map_adr = map_no;
						}
						map_adr += 32;
						old_map_adr += 32;
					}
					map_adr += (1 - 32 * (X_SIZE - 2));
					old_map_adr += (1 - 32 * (X_SIZE - 2));
				}
			}
			else
			{
				for(j = 1; j < (Y_SIZE - 1); ++j, vy += 8){
					for(i = 1, vx = (OFS_X) * 16; i < (X_SIZE - 1); ++i, vx += 16){
						map_no = *map_adr;
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
						}else if(map_no != *old_map_adr){
							pat_sub();
							*old_map_adr = map_no;
						}
						++map_adr;
						++old_map_adr;
					}
					map_adr += (32 - (X_SIZE - 2));
					old_map_adr += (32 - (X_SIZE - 2));
				}
			}
		}
		old_x = x;
		old_y = y;

		while((inportb(0x0040) & 0x20));
		while(!(inportb(0x0040) & 0x20)); /* WAIT VSYNC */
	}

	end();
}
