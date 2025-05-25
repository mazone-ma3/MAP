/* PC-AT GCC-ia16 キャラMAP移動実験  2x2圧縮展開 By m@3 */
/* キャラを出す */

#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <i86.h>
#include <fcntl.h>
#include <dos.h>
#include <unistd.h>

#define ON 1
#define OFF 0
#define PAT 1

#define EOIDATA 0x20
#define EOI 0x20

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

#define MAXCOLOR 16

/* R G B */
unsigned char org_pal[MAXCOLOR][3] = {
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

/*疑似BG処理と基本的な画面操作各種*/

unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 , 15};

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

unsigned short x = 165, y = 30;
unsigned char dir = 2, dir2 = 0;

#define WIDTH 32
#define LINE 212
#define RAM_ADR 0x2000


unsigned char __far *vram_adr = MK_FP(0xa000,0);

unsigned char __far *vram = MK_FP(0xa800, 0);

int conv(char *loadfil)
{
	long i, j,count, count2;
	int k=0, l=0, m=0;
	unsigned char read_pattern[WIDTH * LINE * 2 + 2];
	unsigned char pattern[10]; 
	unsigned char pcatcolor[4];
	unsigned char msxcolor[8];
	unsigned char color;
	unsigned short header;
	FILE *stream[2];

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}
	fread(pattern, 1, 1, stream[0]);	/* MSX先頭を読み捨てる */
	fread(pattern, 1, 4, stream[0]);	/* MSXヘッダも読み捨てる */

	fread(pattern, 1, 2, stream[0]);	/* MSXヘッダを読み捨てる */


	for(count = 0; count < 8; ++count){
		i = fread(read_pattern, 1, (WIDTH * LINE / 2), stream[0]);	/* 4dot分 */
		m = 0;
//		if(i < 1)
//			break;
		for(count2 = 0; count2 < (WIDTH * LINE / 8); ++count2){
	

			/* 色分解と拡大 */
			msxcolor[0] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[1] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[2] = read_pattern[m] & 0x0f;
			msxcolor[3] = read_pattern[m] & 0x0f;
			msxcolor[4] = (read_pattern[++m] >>4) & 0x0f;
			msxcolor[5] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[6] = read_pattern[m] & 0x0f;
			msxcolor[7] = read_pattern[m++] & 0x0f;
			for(i = 0; i < 4; ++i){
				pcatcolor[i] = 0;
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 4; ++i){
					color = conv_tbl[msxcolor[j]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, pcatcolor[i]);
					}else{
						BITCLR(7-j, pcatcolor[i]);
					}
				}
			}

//			i = fread(pattern, 1, 2, stream[0]);	/* 4dot分 */
//			if(i < 1)
//				break;

			/* 色分解と拡大 */
			msxcolor[0] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[1] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[2] = read_pattern[m] & 0x0f;
			msxcolor[3] = read_pattern[m] & 0x0f;
			msxcolor[4] = (read_pattern[++m] >>4) & 0x0f;
			msxcolor[5] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[6] = read_pattern[m] & 0x0f;
			msxcolor[7] = read_pattern[m++] & 0x0f;

			for(i = 0; i < 4; ++i){
				pattern[i] = pcatcolor[i];
			}

			for(i = 0; i < 4; ++i){
				pcatcolor[i] = 0;
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 4; ++i){
					color = conv_tbl[msxcolor[j]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, pcatcolor[i]);
					}else{
						BITCLR(7-j, pcatcolor[i]);
					}
				}
			}

			for(i = 0; i < 4; ++i){
				pattern[4 + i] = pcatcolor[i];
			}

			outportb(0x3ce, 0x05);	// write mode
			outportb(0x3cf, 0x00);	// normal
			outportb(0x3ce, 0x08);	// bit mask
			outportb(0x3cf, 0xff);	// none-mask

			outportb(0x3c4, 0x02); 	// map mask
			outportb(0x3c5, 0x01);	// plane
			*(vram_adr + 0 + k + l) = pattern[0];
			*(vram_adr + 1 + k + l) = pattern[4];
//			*(vram_adr + 80 + k + l) = pattern[0];
//			*(vram_adr + 81 + k + l) = pattern[4];

			outportb(0x3c4, 0x02);
			outportb(0x3c5, 0x02);
			*(vram_adr + 0 + k + l) = pattern[1];
			*(vram_adr + 1 + k + l) = pattern[5];
//			*(vram_adr + 80 + k + l) = pattern[1];
//			*(vram_adr + 81 + k + l) = pattern[5];

			outportb(0x3c4, 0x02);
			outportb(0x3c5, 0x04);
			*(vram_adr + 0 + k + l) = pattern[2];
			*(vram_adr + 1 + k + l) = pattern[6];
//			*(vram_adr + 80 + k + l) = pattern[2];
//			*(vram_adr + 81 + k + l) = pattern[6];

			outportb(0x3c4, 0x02);
			outportb(0x3c5, 0x08);
			*(vram_adr + 0 + k + l) = pattern[3];
			*(vram_adr + 1 + k + l) = pattern[7];
//			*(vram_adr + 80 + k + l) = pattern[3];
//			*(vram_adr + 81 + k + l) = pattern[7];

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


/*グラフィック画面から構造体配列pattern_8に読み込む*/
void get_8(unsigned short x, unsigned short y ,unsigned short no)
{
	unsigned short j, c;

	outportb(0x3ce, 0x05);	// read mode
	outportb(0x3cf, 0x00);	// normal
	outportb(0x3ce, 0x08);	// bit mask
	outportb(0x3cf, 0xff);	// none-mask

	for (j = 0; j < 8; j++){
		c = x / 8 + y * 80 + j * 80;
		outportb(0x3ce, 0x04); 	// read plane no
		outportb(0x3cf, 0x00);	// plane
		chr_8[no].b_vram[j] = vram[c] + vram[c + 1] * 256;
		outportb(0x3ce, 0x04); 	// read plane no
		outportb(0x3cf, 0x01);	// plane
		chr_8[no].r_vram[j] = vram[c] + vram[c + 1] * 256;
		outportb(0x3ce, 0x04); 	// read plane no
		outportb(0x3cf, 0x02);	// plane
		chr_8[no].g_vram[j] = vram[c] + vram[c + 1] * 256;
		outportb(0x3ce, 0x04); 	// read plane no
		outportb(0x3cf, 0x03);	// plane
		chr_8[no].i_vram[j] = vram[c] + vram[c + 1] * 256; 
	}
}

void get_8_2(unsigned short x, unsigned short y ,unsigned short no)
{
	unsigned short j, c;
	unsigned short temp;

	outportb(0x3ce, 0x05);	// read mode
	outportb(0x3cf, 0x00);	// normal
	outportb(0x3ce, 0x08);	// bit mask
	outportb(0x3cf, 0xff);	// none-mask

	for (j = 0; j < 8; j++){
		c = x / 8 + y * 80 + j * 80;
		outportb(0x3ce, 0x04); 	// read plane no
		outportb(0x3cf, 0x00);	// plane
		temp = chr_8_2[no].b_vram[j] = (vram[c] + vram[c + 1] * 256);
		outportb(0x3ce, 0x04); 	// read plane no
		outportb(0x3cf, 0x01);	// plane
		temp |= chr_8_2[no].r_vram[j] = (vram[c] + vram[c + 1] * 256);
		outportb(0x3ce, 0x04); 	// read plane no
		outportb(0x3cf, 0x02);	// plane
		temp |= chr_8_2[no].g_vram[j] = (vram[c] + vram[c + 1] * 256);
		outportb(0x3ce, 0x04); 	// read plane no
		outportb(0x3cf, 0x03);	// plane
		temp |= chr_8_2[no].i_vram[j] = (vram[c] + vram[c + 1] * 256);
		chr_8_mask[j][no] = ~temp;
	}
}


void get_8_mask(unsigned short x, unsigned short y ,unsigned short no)
{
	return;
	unsigned short j, c;
	unsigned short temp;

	outportb(0x3ce, 0x05);	// read mode
	outportb(0x3cf, 0x00);	// normal
	outportb(0x3ce, 0x08);	// bit mask
	outportb(0x3cf, 0xff);	// none-mask

	for (j = 0; j < 8; j++){
		c = x / 8 + y * 80 + j * 80;
		outportb(0x3ce, 0x04); 	// read plane no
		outportb(0x3cf, 0x00);	// plane
		temp = (vram[c] + vram[c + 1] * 256);
		outportb(0x3ce, 0x04); 	// read plane no
		outportb(0x3cf, 0x01);	// plane
		temp |= (vram[c] + vram[c + 1] * 256);
		outportb(0x3ce, 0x04); 	// read plane no
		outportb(0x3cf, 0x02);	// plane
		temp |= (vram[c] + vram[c + 1] * 256);
		outportb(0x3ce, 0x04); 	// read plane no
		outportb(0x3cf, 0x03);	// plane
		temp |= (vram[c] + vram[c + 1] * 256);
		chr_8_mask[j][no] = ~temp;
	}
}

/*構造体配列pattern_8の内容を画面に書き出す*/
void put_8(unsigned short x, unsigned short y, unsigned short no)
{
	unsigned short j, c;

	for (j = 0; j < 8; j++){
//		_disable();
		outportb(0x3ce, 0x00);	// 
		outportb(0x3cf, 0x0f);	// 

		outportb(0x3ce, 0x05);	// write mode
		outportb(0x3cf, 0x00);	// normal
		outportb(0x3ce, 0x08);	// bit mask
		outportb(0x3cf, 0xff);	// none-mask

		c = x / 8 + y * 160 + j * 160;

		outportb(0x3c4, 0x02); 	// map mask
		outportb(0x3c5, 0x01);	// plane
//		_enable();
//		_disable();
		vram[c] = chr_8[no].b_vram[j] % 256;
		vram[c+1] = chr_8[no].b_vram[j] / 256;
		vram[c+80] = chr_8[no].b_vram[j] % 256;
		vram[c+81] = chr_8[no].b_vram[j] / 256;
//		_enable();
//		_disable();
		outportb(0x3c4, 0x02); 	// map mask
		outportb(0x3c5, 0x02);	// plane
		vram[c] = chr_8[no].r_vram[j] % 256;
		vram[c+1] = chr_8[no].r_vram[j] / 256;
		vram[c+80] = chr_8[no].r_vram[j] % 256;
		vram[c+81] = chr_8[no].r_vram[j] / 256;
//		_enable();
//		_disable();
		outportb(0x3c4, 0x02); 	// map mask
		outportb(0x3c5, 0x04);	// plane
		vram[c] = chr_8[no].g_vram[j] % 256;
		vram[c+1] = chr_8[no].g_vram[j] / 256;
		vram[c+80] = chr_8[no].g_vram[j] % 256;
		vram[c+81] = chr_8[no].g_vram[j] / 256;
//		_enable();
//		_disable();
		outportb(0x3c4, 0x02); 	// map mask
		outportb(0x3c5, 0x08);	// plane
		vram[c] = chr_8[no].i_vram[j] % 256;
		vram[c+1] = chr_8[no].i_vram[j] / 256;
		vram[c+80] = chr_8[no].i_vram[j] % 256;
		vram[c+81] = chr_8[no].i_vram[j] / 256;
//		_enable();
	}
}

void put_8_chr(unsigned short x, unsigned short y, unsigned short no, unsigned short no2)
{
	unsigned short j, c, temp;

	for (j = 0; j < 8; j++){
//		_disable();
		outportb(0x3ce, 0x00);	// 
		outportb(0x3cf, 0x0f);	// 

		outportb(0x3ce, 0x05);	// write mode
		outportb(0x3cf, 0x00);	// normal
		outportb(0x3ce, 0x08);	// bit mask
		outportb(0x3cf, 0xff);	// none-mask

		c = x / 8 + y * 160 + j * 160;
		outportb(0x3c4, 0x02); 	// map mask
		outportb(0x3c5, 0x01);	// plane
//		_enable();
//		_disable();
		temp = (chr_8[no].b_vram[j] & chr_8_mask[j][no2] | chr_8_2[no2].b_vram[j]);
		vram[c] = temp % 256;
		vram[c+1] = temp / 256;
		vram[c+80] = temp % 256;
		vram[c+81] = temp / 256;
		outportb(0x3c4, 0x02); 	// map mask
		outportb(0x3c5, 0x02);	// plane
//		_enable();
//		_disable();
		temp = (chr_8[no].r_vram[j] & chr_8_mask[j][no2] | chr_8_2[no2].r_vram[j]);
		vram[c] = temp % 256;
		vram[c+1] = temp / 256;
		vram[c+80] = temp % 256;
		vram[c+81] = temp / 256;
		outportb(0x3c4, 0x02); 	// map mask
		outportb(0x3c5, 0x04);	// plane
//		_enable();
//		_disable();
		temp = (chr_8[no].g_vram[j] & chr_8_mask[j][no2] | chr_8_2[no2].g_vram[j]);
		vram[c] = temp % 256;
		vram[c+1] = temp / 256;
		vram[c+80] = temp % 256;
		vram[c+81] = temp / 256;
		outportb(0x3c4, 0x02); 	// map mask
		outportb(0x3c5, 0x08);	// plane
//		_enable();
//		_disable();
		temp = (chr_8[no].i_vram[j] & chr_8_mask[j][no2] | chr_8_2[no2].i_vram[j]);
		vram[c] = temp % 256;
		vram[c+1] = temp / 256;
		vram[c+80] = temp % 256;
		vram[c+81] = temp / 256;
//		_enable();
	}
}


union REGS reg;
union REGS reg_out;

unsigned char old_mode;

void g_init(void)
{
	reg.h.ah = 0x0f;
	int86(0x10, &reg, &reg);
	old_mode = reg.h.al;

	reg.h.ah = 0x00;
	reg.h.al = 0x12; //0x72;	/* VGA mode 640x480/16colors */
	int86(0x10, &reg, &reg);
}

/*終了処理*/
void end()
{
	reg.h.ah = 0x00;
	reg.h.al = old_mode;
	int86(0x10, &reg, &reg);
}

void paint(unsigned char plane, unsigned char bit)
{
	unsigned short i, j;

	outportb(0x3ce, 0x05);	// write mode
	outportb(0x3cf, 0x00);	// normal
	outportb(0x3ce, 0x08);	// bit mask
	outportb(0x3cf, 0xff);	// none-mask

	for (i = 0; i < (480L); ++i){
		for (j = 0; j < 80; ++j){
			outportb( 0x3c4, 0x02);		/* mask */
			outportb( 0x3c5, plane);		/* plane */
			*(vram_adr + j + i * 80) = bit; /* bit */;
		}
	}
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(short type)
{
	if(type & 1)
		paint(0x0f, 0);

	if(type & 2)
		printf("\x1b*");
}


/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char blue,
	unsigned char green)
{
/*	reg.h.ah = 0x10;
	reg.h.al = 0x10;
	reg.x.bx = color;
	reg.h.dh = red;
	reg.h.ch = blue;
	reg.h.cl = green;
	int86(0x10, &reg, &reg);
*/
	outportb(0x3c8, color);
	outportb(0x3c9, red);
	outportb(0x3c9, blue);
	outportb(0x3c9, green);
}

void wait_vsync(void)
{
	while((inportb(0x3da) & 0x08)); /* WAIT VSYNC */
	while(!(inportb(0x3da) & 0x08));
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
//		pal_set(pal_no, j, pal[0], pal[1], pal[2]);
		pal_set(pal_no, j, ((pal[0] + 1)*4-1) * (pal[0] != 0), ((pal[1]+1)*4-1) * (pal[1] != 0), ((pal[2]+1)*4-1) * (pal[2] != 0));
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
	char j;
	for(j = 0; j < MAXCOLOR; j++)
		pal_set(pal_no, j, 0, 0, 0);
}

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned char i;
	for(i = 0; i < 16; i++)
		pal_set(pal_no, i, ((color[i][0] + 1)*4-1) * (color[i][0] != 0), ((color[i][1]+1)*4-1) * (color[i][1] != 0), ((color[i][2]+1)*4-1) * (color[i][2] != 0));
}

void __interrupt __far ip_key(void);
void __interrupt __far (*keepvector)(void);

unsigned char st2;

unsigned char keyscan(void)
{
	unsigned char key_status;
	do{
		key_status = inportb(0x64);
	}while(!(key_status & 0x01));

	return inportb(0x60);
}

#define KEY_MAX 32

unsigned char __far keydata[KEY_MAX], key_count = 0;

void __interrupt __far ip_key(void)
{
//	unsigned char key_data;
//	_disable();

	while(!(inportb(0x64) & 0x01));
	if(key_count < KEY_MAX)
		keydata[key_count++] = inportb(0x60);//keyscan();

	outportb(EOI, EOIDATA);
}

void key_sense(unsigned char key_data)
{
	switch(key_data){
	case 0xff:
//		goto key_end;
		break;
	case 0x2a:
		st2 = 0;
		break;
	case 0xe0:
//		key_data = keyscan();
		break;
//	}

//	if(!(key_data & 0x80)){
//		switch(key_data){
			case 1:	/* ESC */
				BITSET(4, st2);
				BITSET(5, st2);
				break;
			case 0x48:
				BITSET(0, st2);	/* 8 */
				break;
			case 0x4d:
				BITSET(1, st2);	/* 6 */
				break;
			case 0x50:
				BITSET(2, st2);	/* 2 */
				break;
			case 0x4b:
				BITSET(3, st2);	/* 4 */
				break;
			case 0x2c:	/* Z */
				BITSET(4, st2);	/* 4 */
				break;
			case 0x2d:	/* X */
				BITSET(5, st2);	/* 4 */
				break;
			case 0x39:	/* SPC */
				BITSET(4, st2);	/* 4 */
				break;
//		}
//	}else{
//		key_data &= 0x7f;
//		switch(key_data){
			case 0xc8:
				BITCLR(0, st2);	/* 8 */
				break;
			case 0xcd:
				BITCLR(1, st2);	/* 6 */
				break;
			case 0xd0:
				BITCLR(2, st2);	/* 2 */
				break;
			case 0xcb:
				BITCLR(3, st2);	/* 4 */
				break;
			case 0xac:	/* Z */
				BITCLR(4, st2);	/* 4 */
				break;
			case 0xad:	/* X */
				BITCLR(5, st2);	/* 4 */
				break;
			case 0x89:	/* SPC */
				BITCLR(4, st2);	/* 4 */
				break;
//		}
	}

//key_end:
//	outportb(EOI, EOIDATA);
}

#define KEY_IRQ 0x09

void init_key(void)
{
	_disable();
	keepvector = _dos_getvect(KEY_IRQ);
	_dos_setvect(KEY_IRQ, ip_key);
	_enable();
//	outportb(EOI, EOIDATA);
}


void term_key(void)
{
	_dos_setvect(KEY_IRQ, keepvector);
}

/*マップデータをファイルから配列に読み込む*/
unsigned short mapload(unsigned char *fil, unsigned char *mapdata)
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


void chr_sub(unsigned short vx, unsigned short vy, unsigned char i, unsigned char j, unsigned char map_no)
{
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

	unsigned short no2 = chr_tbl[dir * 2 + dir2][(i - CHR_X) + (j - CHR_Y) * 2];
	put_8_chr(vx , vy,	map_no, no2);
}

void pat_sub(unsigned short vx, unsigned short vy, unsigned char map_no)
{
	put_8(vx, vy, map_no);
}

unsigned char st0 = 0;

unsigned char stickmode = ON;

unsigned char GetJoy(unsigned char *x_count, unsigned char *y_count)
{
	unsigned char st;

	if(stickmode == OFF)
		return 0xff;

//	outportb(0x201, 0);
	*x_count = 0;
	*y_count = 0;

	_disable();
	outportb(0x201, 0); /* Joy Init */
	do{
		st = inportb(0x201);
		*x_count = *x_count + 1;
	}while((st & 0x01));
	outportb(0x201, 0); /* Joy Init */
	do{
		st = inportb(0x201);
		*y_count = *y_count + 1;
	}while((st & 0x02));

	_enable();

	return st;
}

unsigned char fadeflag = 0;

int	main(int argc,char **argv){
	unsigned short xx, yy, old_x = 255, old_y = 255;
	unsigned char i, j;

	unsigned char sub_flag;
	unsigned char *map_adr;
	unsigned char *old_map_adr;

	unsigned char *data, *data_tmp;
	unsigned char *pat_adr;
	unsigned char pat_no;

	unsigned char x_count_org = 0, y_count_org = 0;
	unsigned char x_count = 0, y_count = 0;
	unsigned char st1, st3, old_st1;

	unsigned char map_data[(X_SIZE+2) * 32];
	unsigned char old_map_data[(X_SIZE + 2) * 32];

	unsigned short vx, vy;
	unsigned char map_no; 
	unsigned char mapdata[BUFFSIZE];

	if(inportb(0x201) == 0xff)
		stickmode = OFF;
	GetJoy(&x_count_org, &y_count_org);

	vram_adr = MK_FP(0xa000,0);
	vram = MK_FP(0xa000,0);

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}

	g_init();
	for(i = 0; i < 16; ++i){
		reg.x.ax = 0x1000;
		reg.h.bl = i;
		reg.h.bh = i;

		int86(0x10, &reg, &reg);
	}
	pal_all(0, org_pal);

	clear(3);

	if(conv("ELMAPCHR.SC5")){
		exit(1);
	}
//	return 0;

/*画面からパーツデータを取り出す。*/

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

/*マップのロード to (unsigned char mapdata[])*/

	if (mapload("ELMSTMAP.mdt", mapdata)){
		exit(1);
	}

	clear(3);

	pal_allblack(0);

	init_key();

/* ESCで抜けるまでループ*/
	for(;;){

		_disable();
		for(i = 0; i < key_count; i++)
			key_sense(keydata[i]);
		key_count = 0;
		_enable();

		st3 = st2;

		st1 = ~GetJoy(&x_count, &y_count);

		if(BITTST(0, st3) || (y_count < y_count_org)){
			if(y > 0){
				--y;
				dir = 0;
			}
		}
		if(BITTST(1, st3) || (x_count > x_count_org)){
			if(x < (MAP_SIZE_X*2 - X_SIZE)){
				++x;
				dir = 1;
			}
		}
		if(BITTST(2, st3) || (y_count > y_count_org)){
			if(y < (MAP_SIZE_Y*2 - Y_SIZE)){
				++y;
				dir = 2;
			}
		}
		if(BITTST(3, st3) || (x_count < x_count_org)){
			if(x > 0){
				--x;
				dir = 3;
			}
		}
		if(BITTST(4, st3 || BITTST(4, st1)))
			if(BITTST(5, st3) || BITTST(5, st1))
				break;

		if((old_x != x) || (old_y != y)){
			xx = 1 - x % 2;
			yy = 1 - y % 2;

//			_disable();
			data = &mapdata[x / 2 + (y / 2) * MAP_SIZE_X];
			data_tmp = data;
			map_adr = &map_data[xx + yy * 32];
//			_enable();

			for(j = 0; j < Y_SIZE / 2; ++j){
				for(i = 0; i < X_SIZE / 2; ++i){
//					data = mapdata[x / 2 + i + (j + y / 2) * 128];
//					pat_no = ((data >> 4) & 0x0f) | ((data << 4) & 0xf0);
//					_disable();
					pat_no = ((*data >> 4) & 0x0f) | ((*data << 4) & 0xf0);
					pat_adr = &mapdata[PARTS_HEAD + pat_no * 4];

					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr++);
					map_adr += (32 - 1);
					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr);
//					_enable();
					map_adr -= (32 - 1);
//					for(k = 0; k < 4; ++k){
//						map_data[i * 2 + (k % 2) + xx][j * 2 + (k / 2) + yy] = mapdata[PARTS_HEAD + pat_no * 4 + k];
//					}
//					_disable();
					++data;
				}
				data_tmp += MAP_SIZE_X;
				data = data_tmp;
				map_adr += (32 * 2 - X_SIZE);
			}
			dir2 = 1 - dir2;
//			_enable();

			vx = (OFS_X) * 16;
			vy = (OFS_Y) * 8;
//			_disable();	
			map_adr = &map_data[1 + 1 * 32];
			old_map_adr = &old_map_data[1 + 1 * 32];
//			_enable();

			if(old_x != x){
				for(i = 1; i < (X_SIZE - 1); ++i, vx += 16){
					for(j = 1, vy = (OFS_Y) * 8; j < (Y_SIZE - 1); ++j, vy += 8){
//						_disable();	
						map_no = *map_adr;
//						_enable();
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
							chr_sub(vx, vy, i, j, map_no);
						}else if(map_no != *old_map_adr){
							pat_sub(vx, vy, map_no);
//							_disable();	
							*old_map_adr = map_no;
//							_enable();
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
//						_disable();	
						map_no = *map_adr;
//						_enable();	
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
							chr_sub(vx, vy, i, j, map_no);
						}else if(map_no != *old_map_adr){
							pat_sub(vx, vy, map_no);
//							_disable();	
							*old_map_adr = map_no;
//							_enable();	
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

		sys_wait(2);

		if(fadeflag == 0){
			fadeflag = 1;
			fadeinblack(org_pal, 0, 3);
		}
	}
	fadeoutblack(org_pal, 0, 3);

	term_key();

	end();

	return 0;
}
