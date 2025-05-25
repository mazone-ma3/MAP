/* FM TOWNS キャラMAP移動実験  2x2圧縮展開 By m@3 */
/* キャラを出す */
/* 16色版 */

#include <stdio.h>
#include <stdlib.h>

#include <egb.h>
#include <snd.h>
#include <spr.h>
#include <dos.h>
#include <conio.h>
#include <FMCFRB.H>
#include <fcntl.h>
#include <io.h>

char egb_work[1536];

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

unsigned short chr_8[4][16][256];

unsigned short chr_8_2[4][16][256];

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

_Far unsigned short *vram;


#define WIDTH 32
#define LINE 212

_Far short *vram_adr;

FILE *stream[2];

unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 , 15};


int conv(char *loadfil)
{
	long i, count, count2;
	int k=0, l=0, m=0;
	unsigned char read_pattern[WIDTH * LINE * 2+ 2];
	unsigned char pattern[10];
	unsigned short fmtcolor[4];
	unsigned char msxcolor[8];

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
		for(count2 = 0; count2 < WIDTH * LINE / 2; ++count2){
	

			/* 色分解 */
			msxcolor[0] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[1] = read_pattern[m++] & 0x0f;
			fmtcolor[0] = msxcolor[0] + msxcolor[0] * 16;
			fmtcolor[1] = msxcolor[1] + msxcolor[1] * 16;

			msxcolor[0] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[1] = read_pattern[m++] & 0x0f;
			fmtcolor[2] = msxcolor[0] + msxcolor[0] * 16;
			fmtcolor[3] = msxcolor[1] + msxcolor[1] * 16;

			_FP_OFF(vram_adr) = k * 2 + l * 2;
			*(vram_adr++) = fmtcolor[0] + fmtcolor[1] * 256;
			*vram_adr = fmtcolor[2] + fmtcolor[3] * 256;
			_FP_OFF(vram_adr) = k * 2 + l * 2 + 256 * 2;
			*(vram_adr++) = fmtcolor[0] + fmtcolor[1] * 256;
			*vram_adr = fmtcolor[2] + fmtcolor[3] * 256;

			k += 2;
			if(k >= (128)){
				k = 0;
				l += ((256) * 2);
			}
		}
	}
	fclose(stream[0]);

	return 0;
}

/*グラフィック画面から構造体配列pattern_8に読み込む*/
void get_8(unsigned short x, unsigned short y ,unsigned short no)
{
	unsigned short i, j;

	_FP_OFF(vram) = (x / 2) + (y) * 512;
	for (j = 0; j < 16; j++){
		for(i = 0; i < 4; i++){
			chr_8[i][j][no] = *(vram++);
		}
		vram += (256 - 4);
	}
}

void get_8_2(unsigned short x, unsigned short y ,unsigned short no)
{
	unsigned short i, j;

	_FP_OFF(vram) = (x / 2) + (y) * 512;
	for (j = 0; j < 16; j++){
		for(i = 0; i < 4; i++){
			chr_8_2[i][j][no] = *(vram++);
		}
		vram += (256 - 4);
	}
}

/*構造体配列pattern_8の内容を画面に書き出す*/
void put_8(unsigned short x, unsigned short y, unsigned short no)
{
	unsigned short i, j;

	_FP_OFF(vram) = (x / 2) + (y) * 512;
	for (j = 0; j < 16; j++){
		for(i = 0; i < 4; i++){
			*(vram++) = chr_8[i][j][no];
		}
		vram += (256 - 4);
	}
}

void put_8_chr(unsigned short x, unsigned short y, unsigned short no, unsigned short no2)
{
	unsigned short i, j;

	_FP_OFF(vram) = (x / 2) + (y) * 512;
	vram += 0x20000;
	for (j = 0; j < 16; j++){
		for(i = 0; i < 4; i++){
			*(vram++) = chr_8_2[i][j][no2];
		}
		vram += (256 - 4);
	}
}

void g_init(void)
{
//	char para[64];

	EGB_init(egb_work, 1536);

/* 31kHz出力用 */
//	EGB_resolution(egb_work, 0, 10);		/* ペ－ジ0は512x256/32768 */
	EGB_resolution(egb_work, 0, 3);			/* ペ－ジ0は640x480/16 */
	EGB_resolution(egb_work, 1, 3);			/* ペ－ジ1は640x480/16 */
	EGB_displayPage(egb_work, 1, 3);		/* 上にくるペ－ジは1で両方とも表示 */
	EGB_writePage(egb_work, 0);				/* ペ－ジ0の設定 */
	EGB_displayStart(egb_work, 2, 1, 1);		/* 表示拡大率(縦横1倍) */
	EGB_displayStart(egb_work, 3, 640, 480);	/* EGB画面の大きさ(640x480) */
	EGB_displayStart(egb_work, 0, 0, 0);		/* 表示開始位置 */
	EGB_displayStart(egb_work, 1, 0, 0);		/* 仮想画面中の移動 */

	EGB_writePage(egb_work, 1);				/* ペ－ジ1の設定 */
	EGB_displayStart(egb_work, 2, 1, 1);		/* */
	EGB_displayStart(egb_work, 3, 640, 480);	/* */
	EGB_displayStart(egb_work, 0, 0, 0);		/* */
	EGB_displayStart(egb_work, 1, 0, 0);		/* */

	EGB_color(egb_work, 0, 0x0000);				/* ペ－ジ1をクリアスクリ－ン */
	EGB_color(egb_work, 2, 0x0000);				/* 透明色で埋める */
	EGB_writePage(egb_work, 1);
	EGB_clearScreen(egb_work);

}

/*終了処理*/
void end()
{
	EGB_resolution(egb_work, 0, 4);		/* ペ－ジ0は640x400/16 */
	EGB_resolution(egb_work, 1, 4);		/* ペ－ジ1は640x400/16 */
	EGB_displayPage(egb_work, 0, 3);

	EGB_writePage(egb_work, 0);			/* ペ－ジ0をクリアスクリ－ン */
	EGB_clearScreen(egb_work);
	EGB_displayStart(egb_work,0,0,0);
	EGB_displayStart(egb_work, 1, 0, 0);
	EGB_displayStart(egb_work,2,1,1);
	EGB_displayStart(egb_work, 3, 640, 400);
	EGB_writePage(egb_work, 1);			/* ペ－ジ1をクリアスクリ－ン */
	EGB_clearScreen(egb_work);
	EGB_displayStart(egb_work,0,0,0);
	EGB_displayStart(egb_work, 1, 0, 0);
	EGB_displayStart(egb_work,2,1,1);
	EGB_displayStart(egb_work, 3, 640, 400);
}

void paint(unsigned short color)
{
	unsigned short i, j;

	for (i = 0; i < (480); ++i){
		for (j = 0; j < 640; ++j){
			_FP_OFF(vram_adr) = (j + i * 512) * 1;
			*vram_adr = color;
		}
	}
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(short type)
{
	if(type & 1){
		paint(0x0);
	}
	if(type & 2)
		printf("\x1b*");
}

/*パレット・セット*/
void pal_set(unsigned char color, unsigned char red, unsigned char blue,
	unsigned char green)
{
	_outp(0x448,0x01);
	_outp(0x44a,0x01);	/* priority register */

	_outp(0xfd90, color);
	_outp(0xfd92, blue * 16);
	_outp(0xfd94, red * 16);
	_outp(0xfd96, green * 16);

	_outp(0x448,0x01);
	_outp(0x44a,0x21);	/* priority register */

	_outp(0xfd90, color);
	_outp(0xfd92, blue * 16);
	_outp(0xfd94, red * 16);
	_outp(0xfd96, green * 16);
}

void pal_all(void)
{
	unsigned char i;
	for(i = 0; i < 16; i++)
		pal_set(i, ((pal[i][0] + 1)*1-1) * (pal[i][0] != 0), ((pal[i][2]+1)*1-1) * (pal[i][2] != 0), ((pal[i][1]+1)*1-1) * (pal[i][1] != 0));
}

/*マップデータをファイルから配列に読み込む*/
short mapload(char *fil)
{
	short inhandle;

	if ((inhandle = open( fil, O_RDONLY )) == -1) { //| O_BINARY
		printf("\x01b[31mCan\'t open file %s.\x01b[37m", fil);
		return ERROR;
	}
	read( inhandle, (char *)mapdata, 7);
	read( inhandle, (char *)mapdata, BUFFSIZE);
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
	put_8(vx, vy, map_no);
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

	unsigned char st, pd;
	unsigned char k5, k6, k7, k8, k9, ka;

	unsigned char *data, *data_tmp;
	unsigned char pat_no, *pat_adr;
	unsigned short paddata;
	unsigned int encode;

	static char matrix[16];

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}

	g_init();
	pal_all();

	_FP_SEG(vram_adr)=0x120;
	clear(3);

	if(conv("ELMAPCHR.SC5")){
		end();
		exit(1);
	}

/*画面からパーツデータを取り出す。*/

	_FP_SEG(vram)=0x120;
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

/*	for ( j = 0; j < 16; j++){
		for ( i = 0; i < 16; i++){
			get_8_mask(i * 16 + 256, j * 16, i + j * 16);
		}
	}
*/
	clear(3);

/*マップのロード to (unsigned char mapdata[])*/

	if (mapload("ELMSTMAP.MDT")){
		end();
		exit(1);
	}


/* ESCで抜けるまでループ*/
	while((KYB_read( 1, &encode ) != 0x1b)){ //((k9 = inportb(0x09)) & 0x80)){ /* ESC */
		KYB_matrix(matrix);

		k5 = ~matrix[5];
		k6 = ~matrix[6];
		k7 = ~matrix[7];
		k8 = ~matrix[8];
		k9 = ~matrix[9];
		ka = ~matrix[0xa];

		paddata = _inb(0x4d0 + 0 * 2); 
		st = (paddata & 0x0f);
		pd = (paddata >> 4) & 0x03;

		if(!(k7 & 0x08) || !(k9 & 0x20) || !(st & 0x01)){ /* 8 */
			if(y > 0){
				--y;
				dir = 0;
			}
		}
		if(!(k8 & 0x01) || !(ka & 0x02) || !(st & 0x08)){ /* 6 */
			if(x < (MAP_SIZE_X*2 - X_SIZE)){
				++x;
				dir = 1;
			}
		}
		if(!(k8 & 0x08) || !(ka & 0x01) || !(st & 0x02)){ /* 2 */
			if(y < (MAP_SIZE_Y*2 - Y_SIZE)){
				++y;
				dir = 2;
			}
		}
		if(!(k7 & 0x40) || !(k9 & 0x80) || !(st & 0x04)){ /* 4 */
			if(x > 0){
				--x;
				dir = 3;
			}
		}
		if(!(k5 & 0x04) || !(k6 & 0x20) || !(pd & 0x01)) /* Z,SPACE */
			if(!(k5 & 0x08) || !(pd & 0x02)) /* X */
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
		do{
			_outb(0x440, 30);
		}while((_inb(0x0443) & 0x04) == 0); /* 動作中 */
		do{
			_outb(0x440, 30);
		}while((_inb(0x0443) & 0x04) != 0); /* 動作中 */
	}

//	getch();
	end();

	return 0;
}
