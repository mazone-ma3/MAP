/* MSX-SC5->PC-98 CONV.(Analog 16 colors) for GCC-ia16 By m@3 */
/* 全体版 */

#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#define EOIDATA 0x20
#define EOI 0

#define ON 1
#define OFF 0
#define PAT 1

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

#define WIDTH 32
#define LINE 200
#define RAM_ADR 0x2000


unsigned char __far *flame[4]
	 = {MK_FP(0xa800,0)	,MK_FP(0xb000,0),MK_FP(0xb800,0),MK_FP(0xe000,0)};

FILE *stream[2];

unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 , 15};

void cursor_switch(short);
void screen_switch(short);


int conv(char *loadfil)
{
	long i, j,count, count2;
	int k=0, l=0;
	unsigned char pattern[100];
	unsigned char pc98color[4];
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
				pc98color[i] = 0;
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 4; ++i){
					color = conv_tbl[msxcolor[j]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, pc98color[i]);
					}else{
						BITCLR(7-j, pc98color[i]);
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
				pattern[i] = pc98color[i];
			}

			for(i = 0; i < 4; ++i){
				pc98color[i] = 0;
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 4; ++i){
					color = conv_tbl[msxcolor[j]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, pc98color[i]);
					}else{
						BITCLR(7-j, pc98color[i]);
					}
				}
			}

			for(i = 0; i < 4; ++i){
				pattern[4 + i] = pc98color[i];
			}

			*(flame[0] + 0 + k + l) = pattern[0];
			*(flame[1] + 0 + k + l) = pattern[1];
			*(flame[2] + 0 + k + l) = pattern[2];
			*(flame[3] + 0 + k + l) = pattern[3];
			*(flame[0] + 1 + k + l) = pattern[4];
			*(flame[1] + 1 + k + l) = pattern[5];
			*(flame[2] + 1 + k + l) = pattern[6];
			*(flame[3] + 1 + k + l) = pattern[7];
/*			*(flame[0] + 80 + k + l) = pattern[0];
			*(flame[1] + 80 + k + l) = pattern[1];
			*(flame[2] + 80 + k + l) = pattern[2];
			*(flame[3] + 80 + k + l) = pattern[3];
			*(flame[0] + 81 + k + l) = pattern[4];
			*(flame[1] + 81 + k + l) = pattern[5];
			*(flame[2] + 81 + k + l) = pattern[6];
			*(flame[3] + 81 + k + l) = pattern[7];
*/
			k += 2;
			if(k >= 64){
				k = 0;
				l += 80; //*2;
			}
		}
	}
	fclose(stream[0]);

	return 0;
}

void g_init(void)
{
	_disable();
	outportb(0x6a, 1   ); /* 16色モード (0x6a=mode f/fp2)*/
	outportb(0xa2, 0x4b);
	outportb(0xa0, 0); /* L/R = 1 (縦方向の拡大係数)*/
	outportb(0x68, 8   ); /* モードF/F1 (8で高解像度)*/
	outportb(0x4a0,0xfff0);
	outportb(0x7c, 0);
	_enable();
}

/*終了処理*/
void end()
{
}

/*カーソル及びファンクションキー表示の制御*/
void cursor_switch(short mode)
{
	if(mode)
		printf("\x1b[>1l\x1b[>5l");
	else
		printf("\x1b*\x1b[>1h\x1b[>5h");
}

void screen_switch(short mode)
{
	if(mode)
		outportb(0xa2, 0x0d); /* 表示開始 */
	else
		outportb(0xa2, 0x0c);
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(short type)
{
	unsigned short i,j;

	if(type & 1){
		for (i = 0; i < 80 * 400; i++)
			for(j = 0; j < 4; j++)
				*(flame[j] + i) = 0;

	}

	if(type & 2)
		printf("\x1b*");
}

/*ページ切り替え*/
void setpage(short visual, short active)
{
	outportb(0xa4, visual);
	outportb(0xa6, active);
}

/*パレット・セット*/
void pal_set(short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
		outportb(0xa8, color);
		outportb(0xaa, green);
		outportb(0xac, red);
		outportb(0xae, blue);
}

void pal_all(unsigned char color[16][3])
{
	short i;
	for(i = 0; i < 16; i++)
		pal_set(i, color[i][0], color[i][1], color[i][2]);
}


int	main(int argc,char **argv){

	short i,j;
	unsigned short a, b;

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

	if (argv[1] == NULL){
		printf("MSX .SC5 file Converter for PC-98.\n");
		return 1;
	}

	screen_switch(OFF);
	g_init();
	pal_all(pal);
	setpage(0,0);
	screen_switch(ON);

	clear(3);
	conv(argv[1]);

	end();

	return 0;
}
