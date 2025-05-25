/* PC-98 Screen to BSAVE CONV.(Analog 16 colors) for GCC-ia16 By m@3 */
/* 全体版 */

#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#define ON 1
#define OFF 0

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

unsigned char __far *flame[4]
	 = {MK_FP(0xa800,0)	,MK_FP(0xb000,0),MK_FP(0xb800,0),MK_FP(0xe000,0)};

FILE *stream[2];

void cursor_switch(short);
void screen_switch(short);


int conv(char *savefil, unsigned char __far *flame)
{
	long i;
	int k=0, l=0;
	unsigned char pattern[100];

	if ((stream[0] = fopen( savefil, "wb")) == NULL) {
		printf("Can\'t open file %s.\n", savefil);

		fclose(stream[0]);
		return 1;
	}
	for(i = 0; i < 0x4000; ++i){
		pattern[0] = flame[i];
		fwrite(pattern, 1, 1, stream[0]);
	}
//	fwrite(flame, 1, 0x4000, stream[0]);
	fwrite("\x1a", 1, 1, stream[0]);
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

	char dst[100];

	if ((argv[1] == NULL) || (argc < 2)){
		printf("Screen to BSAVE Converter for PC-98.\n");
		return 1;
	}

	screen_switch(OFF);
	g_init();
	setpage(0,0);
	screen_switch(ON);

	snprintf(dst, sizeof dst, "%s.grb", argv[1]);
	conv(dst, flame[0]);
	snprintf(dst, sizeof dst, "%s.grr", argv[1]);
	conv(dst, flame[1]);
	snprintf(dst, sizeof dst, "%s.grg", argv[1]);
	conv(dst, flame[2]);
	snprintf(dst, sizeof dst, "%s.gri", argv[1]);
	conv(dst, flame[3]);

	end();

	return 0;
}
