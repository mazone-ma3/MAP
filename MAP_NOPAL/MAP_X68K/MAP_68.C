/* X68K キャラMAP移動実験  2x2圧縮展開 By m@3 */
/* キャラを出す */
/* 16色(Text)版 */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <sys\dos.h>
#include <conio.h>
#include <fcntl.h>

#include <doslib.h>
#include <iocslib.h>
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

typedef struct pattern_8{
	unsigned short b_vram[16];
	unsigned short r_vram[16];
	unsigned short g_vram[16];
	unsigned short i_vram[16];
} chr8;

chr8 chr_8[256];

chr8 chr_8_2[256];

unsigned short chr_8_mask[16][256];

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

unsigned char *bvram, *rvram, *gvram, *ivram; 



#define WIDTH 32
#define LINE 212

unsigned char *vram_adr;

FILE *stream[2];

unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 , 15};

int conv(char *loadfil)
{
	long i, j, count, count2;
	int k=0, l=0, m=0;
	unsigned char read_pattern[WIDTH * LINE * 2+ 2];
	unsigned char pattern[10];
	unsigned short x68color[8];
	unsigned char msxcolor[16];
	unsigned char color;

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}
	fread(pattern, 1, 1, stream[0]);	/* MSX先頭を読み捨てる */
	fread(pattern, 1, 4, stream[0]);	/* MSXヘッダも読み捨てる */

	fread(pattern, 1, 2, stream[0]);	/* MSXヘッダを読み捨てる */


	for(count = 0; count < 4; ++count){
		i = fread(read_pattern, 1, WIDTH * LINE, stream[0]);
		m = 0;
//		if(i < 1)
//			break;
		for(count2 = 0; count2 < WIDTH * LINE / 4; ++count2){
	
			/* 色分解 */
			msxcolor[0] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[1] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[2] = read_pattern[m] & 0x0f;
			msxcolor[3] = read_pattern[m++] & 0x0f;
			msxcolor[4] = (read_pattern[m] >> 4) & 0x0f;
			msxcolor[5] = (read_pattern[m] >> 4) & 0x0f;
			msxcolor[6] = read_pattern[m] & 0x0f;
			msxcolor[7] = read_pattern[m++] & 0x0f;
			msxcolor[8] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[9] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[10] = read_pattern[m] & 0x0f;
			msxcolor[11] = read_pattern[m++] & 0x0f;
			msxcolor[12] = (read_pattern[m] >> 4) & 0x0f;
			msxcolor[13] = (read_pattern[m] >> 4) & 0x0f;
			msxcolor[14] = read_pattern[m] & 0x0f;
			msxcolor[15] = read_pattern[m++] & 0x0f;

			for(i = 0; i < 8; ++i){
				x68color[i] = 0;
			}
			for(j = 0; j < 8; ++j){
				for(i = 0; i < 4; ++i){
					color = conv_tbl[msxcolor[j]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, x68color[i]);
					}else{
						BITCLR(7-j, x68color[i]);
					}
				}
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 4; ++i){
					color = conv_tbl[msxcolor[j+8]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, x68color[i+4]);
					}else{
						BITCLR(7-j, x68color[i+4]);
					}
				}
			}

//			for(i = 0; i < 4; ++i){
//				pattern[i] = x68color[i];
//			}
			vram_adr = (unsigned char *)0xe00000 + k*2 + l; // * 2;
			*(vram_adr + 0x20000 * 0) = x68color[0];
			*(vram_adr + 0x20000 * 0 + 0x80) = x68color[0];
			*(vram_adr + 0x20000 * 1) = x68color[1];
			*(vram_adr + 0x20000 * 1 + 0x80) = x68color[1];
			*(vram_adr + 0x20000 * 2) = x68color[2];
			*(vram_adr + 0x20000 * 2 + 0x80) = x68color[2];
			*(vram_adr + 0x20000 * 3) = x68color[3];
			*(vram_adr + 0x20000 * 3 + 0x80) = x68color[3];
			*(vram_adr + 0x20000 * 0 + 1) = x68color[4];
			*(vram_adr + 0x20000 * 0 + 1 + 0x80) = x68color[4];
			*(vram_adr + 0x20000 * 1 + 1) = x68color[5];
			*(vram_adr + 0x20000 * 1 + 1 + 0x80) = x68color[5];
			*(vram_adr + 0x20000 * 2 + 1) = x68color[6];
			*(vram_adr + 0x20000 * 2 + 1 + 0x80) = x68color[6];
			*(vram_adr + 0x20000 * 3 + 1) = x68color[7];
			*(vram_adr + 0x20000 * 3 + 1 + 0x80) = x68color[7];

			k += 1;
			if(k >= (32)){
				k = 0;
				l += (0x80*2); //(256);
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

	for (j = 0; j < 16; j++){
		c = x / 8 + y * 0x80 + j * 0x80;
		chr_8[no].b_vram[j] = *((unsigned short *)(unsigned char *)(bvram + c));
		chr_8[no].r_vram[j] = *((unsigned short *)(unsigned char *)(rvram + c));
		chr_8[no].g_vram[j] = *((unsigned short *)(unsigned char *)(gvram + c));
		chr_8[no].i_vram[j] = *((unsigned short *)(unsigned char *)(ivram + c));
	}
}

void get_8_2(unsigned short x, unsigned short y ,unsigned short no)
{
	unsigned short j, c;

	for (j = 0; j < 16; j++){
		c = x / 8 + y * 0x80 + j * 0x80;
		chr_8_2[no].b_vram[j] = *((unsigned short *)(unsigned char *)(bvram + c));
		chr_8_2[no].r_vram[j] = *((unsigned short *)(unsigned char *)(rvram + c));
		chr_8_2[no].g_vram[j] = *((unsigned short *)(unsigned char *)(gvram + c));
		chr_8_2[no].i_vram[j] = *((unsigned short *)(unsigned char *)(ivram + c));
	}
}


void get_8_mask(unsigned short x, unsigned short y ,unsigned short no)
{
	unsigned short j, c;

	for (j = 0; j < 16; j++){
		c = x / 8 + y * 0x80 + j * 0x80;
		chr_8_mask[j][no] = ~(
			*((unsigned short *)(unsigned char *)(bvram + c)) |
			*((unsigned short *)(unsigned char *)(rvram + c)) |
			*((unsigned short *)(unsigned char *)(gvram + c)) |
			*((unsigned short *)(unsigned char *)(ivram + c)));
	}
}

/*構造体配列pattern_8の内容を画面に書き出す*/
void put_8(unsigned short x, unsigned short y, unsigned short no)
{
	unsigned short j, c;

	for (j = 0; j < 16; j++){
		c = x / 8 + y * 0x80 + j * 0x80;
		*((unsigned short *)(unsigned char *)(bvram + c))
			= chr_8[no].b_vram[j];
		*((unsigned short *)(unsigned char *)(rvram + c))
			= chr_8[no].r_vram[j];
		*((unsigned short *)(unsigned char *)(gvram + c))
			= chr_8[no].g_vram[j];
		*((unsigned short *)(unsigned char *)(ivram + c))
			= chr_8[no].i_vram[j];
	}
}

void put_8_chr(unsigned short x, unsigned short y, unsigned short no, unsigned short no2)
{
	unsigned short j, c;

	for (j = 0; j < 16; j++){
		c = x / 8 + y * 0x80 + j * 0x80;
		*((unsigned short *)(unsigned char *)(bvram + c)) = 
			(chr_8[no].b_vram[j] & chr_8_mask[j][no2] | chr_8_2[no2].b_vram[j]);
		*((unsigned short *)(unsigned char *)(rvram + c)) =
			(chr_8[no].r_vram[j] & chr_8_mask[j][no2] | chr_8_2[no2].r_vram[j]);
		*((unsigned short *)(unsigned char *)(gvram + c)) =
			(chr_8[no].g_vram[j] & chr_8_mask[j][no2] | chr_8_2[no2].g_vram[j]);
		*((unsigned short *)(unsigned char *)(ivram + c)) =
			(chr_8[no].i_vram[j] & chr_8_mask[j][no2] | chr_8_2[no2].i_vram[j]);
	}
}


void g_init(void)
{
/*	CRTMOD(0x0a); */ 	/* 256x256 256colors 2plane 31kHz */
//	CRTMOD(0x06);	/* 256x256 16colors 4plane 31kHz */
//	CRTMOD(0x0e);	/* 256x256 65536colors 1plane 31kHz */
//	CRTMOD(0x04);	/* 512x512 16colors 4plane 31kHz */
	CRTMOD(0x10);	/* 768x512 16colors 1plane 31kHz */
	G_CLR_ON();
	B_CUROFF();
}

/*終了処理*/
void end()
{
	B_CURON();
	CRTMOD(0x10);	/* 768x512 16colors 1plane 31kHz */
}

void paint(unsigned short color)
{
	unsigned short i, j;

	for (i = 0; i < 512; ++i){
		for (j = 0; j < 0x80; ++j){
			*(vram_adr + j + i * 0x80 + 0x20000 * 0) = color; /* bit */;
			*(vram_adr + j + i * 0x80 + 0x20000 * 1) = color; /* bit */;
			*(vram_adr + j + i * 0x80 + 0x20000 * 2) = color; /* bit */;
			*(vram_adr + j + i * 0x80 + 0x20000 * 3) = color; /* bit */;
		}
	}
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(short type)
{
	if(type & 1)
		paint(0x0);

	if(type & 2)
		printf("\x1b*");
}

/*パレット・セット*/
void pal_set(unsigned char color, unsigned char red, unsigned char blue,
	unsigned char green)
{
	unsigned short *pal_port;
	pal_port = (unsigned short *)(0xe82200 + color * 2);
	*pal_port = (green * 32 * 32 + red * 32 + blue) * 2 + 1;
}

void pal_all(void)
{
	unsigned char i;
	for(i = 0; i < 16; i++)
		pal_set(i, ((pal[i][0] + 1)*2-1) * (pal[i][0] != 0), ((pal[i][2]+1)*2-1) * (pal[i][2] != 0), ((pal[i][1]+1)*2-1) * (pal[i][1] != 0));
}

/*マップデータをファイルから配列に読み込む*/
short mapload(char *fil)
{
	short inhandle;

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


int	main(int argc,char **argv){

	unsigned char old_map_data[(X_SIZE + 2) * 32];
	unsigned char sub_flag;
	unsigned char *map_adr;
	unsigned char *old_map_adr;

	unsigned char k0, k5, k6, k7, k8, k9, st, pd;

	unsigned char *data, *data_tmp;
	unsigned char pat_no, *pat_adr;
	unsigned char volatile *vsync = (unsigned char *)0xe88001;	/* MFP */
	unsigned char *reg = (unsigned char *)0xe9a001;
	unsigned short paddata;

	vram_adr = (unsigned char *)0xe00000;
dum:	B_SUPER(0);		/* スーパーバイザモード 最適化防止にラベルを付ける */

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}
	bvram = (unsigned char *)0xe00000;
	rvram = (unsigned char *)0xe20000;
	gvram = (unsigned char *)0xe40000;
	ivram = (unsigned char *)0xe60000;

	g_init();
	pal_all();

	clear(3);

	if(conv("ELMAPCHR.SC5")){
		end();
		exit(1);
	}

/*画面からパーツデータを取り出す。*/

	for ( j = 0; j < 16; j++){
		for ( i = 0; i < 16; i++){
			get_8(i * 16, j * 16, i + j * 16);
		}
	}

	for ( j = 0; j < 16; j++){
		for ( i = 0; i < 16; i++){
			get_8_2(i * 16 + 256, j * 16, i + j * 16);
		}
	}

	for ( j = 0; j < 16; j++){
		for ( i = 0; i < 16; i++){
			get_8_mask(i * 16 + 256, j * 16, i + j * 16);
		}
	}

	vram_adr = (unsigned char *)0xe00000;
	clear(3);

/*マップのロード to (unsigned char mapdata[])*/

	if (mapload("ELMSTMAP.MDT")){
		end();
		exit(1);
	}

/* ESCで抜けるまでループ*/
	while(!((k0 = BITSNS(0)) & 2)){ //((k9 = inportb(0x09)) & 0x80)){ /* ESC */
		k5 = ~BITSNS(5);
		k6 = ~BITSNS(6);
		k7 = ~BITSNS(7);
		k8 = ~BITSNS(8);
		k9 = ~BITSNS(9);

		paddata = reg[0];
		st = (paddata & 0x0f); // ^ 0x0f;
		pd = ((paddata >> 5) & 0x03); // ^ 0x03;

		if(!(k8 & 0x10) || !(k7 & 0x10) || !(st & 0x01)){ /* 8 */
			if(y > 0){
				--y;
				dir = 0;
			}
		}
		if(!(k9 & 0x02) || !(k7 & 0x20) || !(st & 0x08)){ /* 6 */
			if(x < (MAP_SIZE_X*2 - X_SIZE)){
				++x;
				dir = 1;
			}
		}
		if(!(k9 & 0x10) || !(k7 & 0x40) || !(st & 0x02)){ /* 2 */
			if(y < (MAP_SIZE_Y*2 - Y_SIZE)){
				++y;
				dir = 2;
			}
		}
		if(!(k8 & 0x80) || !(k7 & 0x08) || !(st & 0x04)){ /* 4 */
			if(x > 0){
				--x;
				dir = 3;
			}
		}
		if(!(k5 & 0x04) || !(k6 & 0x20) || !(pd & 0x02)) /* Z,SPACE */
			if(!(k5 & 0x08) || !(pd & 0x01)) /* X */
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
			vy = (OFS_Y) * 16;
			map_adr = &map_data[1 + 1 * 32];
			old_map_adr = &old_map_data[1 + 1 * 32];

			if(old_x != x){
				for(i = 1; i < (X_SIZE - 1); ++i, vx += 16){
					for(j = 1, vy = (OFS_Y) * 16; j < (Y_SIZE - 1); ++j, vy += 16){
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
				for(j = 1; j < (Y_SIZE - 1); ++j, vy += 16){
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

	/* VSYNC待ち */
		while(!((*vsync) & 0x10));	/* 調査中 */
		while((*vsync) & 0x10);		/* 調査中 */
	}
//	getch();

	vram_adr = (unsigned char *)0xe00000;
	clear(3);
	end();

	exit(0);

	return 0;
}
