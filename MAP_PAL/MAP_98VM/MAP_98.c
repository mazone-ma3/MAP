/* PC-98 GCC-ia16 キャラMAP移動実験  2x2圧縮展開 By m@3 */
/* VM/UV以降(多分) */
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

void get_8(unsigned short ,unsigned short ,unsigned short);
void put_8(unsigned short ,unsigned short ,unsigned short);
void g_init(unsigned short);
void end(void);
void setpage(unsigned short, unsigned short);
void clear(unsigned short);
void pal_set(unsigned char, unsigned short,unsigned char,unsigned char,unsigned char);
void pal_all(unsigned char, unsigned char[MAXCOLOR][3]);

void screen_switch(unsigned short);
void cursor_switch(unsigned short);
void v_sync(void);

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

unsigned char __far *bvram, // = (unsigned char __far *)MK_FP(0xa800, 0),
	 *rvram, // = (unsigned char __far *)MK_FP(0xb000, 0),
	 *gvram, // = (unsigned char __far *)MK_FP(0xb800, 0),
	 *ivram; // = (unsigned char __far *)MK_FP(0xe000, 0);


#define WIDTH 32
#define LINE 200
#define RAM_ADR 0x2000


unsigned char __far *flame[4]
	 = {MK_FP(0xa800,0)	,MK_FP(0xb000,0),MK_FP(0xb800,0),MK_FP(0xe000,0)};

FILE *stream[2];

unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 , 15};


unsigned short conv(char *loadfil)
{
	long i, j,count, count2;
	unsigned short k=0, l=0;
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


/*カーソルのＯＦＦ、ＶＲＡＭアドレスの設定*/
void g_init(unsigned short mode_f)
{
/*unsigned short g_driver, g_mode;
	g_driver = DETECT;
	g_mode = 1;
	initgraph(&g_driver, &g_mode ,"");
*/
	_disable();

	outportb(0x6a, 1   ); /* 16色モード (0x6a=mode f/fp2)*/

/*	outportb(0x6a, 0x07);  拡張モード変更可能
	outportb(0x6a, 0x84);  gdc2.5mhz */

	outportb(0xa2, 0x4b); /* GDC CSRFORMコマンド */
	outportb(0xa0, (mode_f == 1)); /* L/R = 1 (縦方向の拡大係数)*/

	outportb(0x68, 8   ); /* モードF/F1 (8で高解像度)*/

	outportb(0x4a0,0xfff0);
	outportw(0x7c, 0);

	screen_switch(ON); /* 表示開始 */

	_enable();

	cursor_switch(OFF);

}

/*終了処理*/
void end()
{
	cursor_switch(ON);
}

/*カーソル及びファンクションキー表示の制御*/
void cursor_switch(unsigned short mode)
{
	if(mode)
		printf("\x1b[>1l\x1b[>5l");
	else
		printf("\x1b*\x1b[>1h\x1b[>5h");
}

void screen_switch(unsigned short mode)
{
	if(mode)
		outportb(0xa2, 0x0d); /* 表示開始 */
	else
		outportb(0xa2, 0x0c);
}

/*ページ切り替え*/
void setpage(unsigned short visual, unsigned short active)
{
	outportb(0xa4, visual);
	outportb(0xa6, active);
}


/*テキスト画面及びグラフィック画面の消去*/
void clear(unsigned short type)
{
	unsigned short i;

	if(type & 1){
		for (i = 0; i < 80 * 200; i++){
			*(bvram++) = 0;
			*(rvram++) = 0;
			*(gvram++) = 0;
			*(ivram++) = 0;
		}
	}

	if(type & 2)
		printf("\x1b*");
}

/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	outportw(0xa8, color);
	outportw(0xaa, green);
	outportw(0xac, red);
	outportw(0xae, blue);
}

/*垂直同期待ち*/
void wait_vsync(void)
{
	while((inportb(0x60) & 0x20));
	while(!(inportb(0x60) & 0x20));
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
		pal_set(pal_no, j, pal[0], pal[1], pal[2]);
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
	unsigned short i;
	wait_vsync();
	for(i = 0; i < MAXCOLOR; i++)
		pal_set(pal_no, i, color[i][0], color[i][1], color[i][2]);
}


/*キースキャン及びタイマウエイト・ルーチン*/

#define UP     (key_scan(0x7) & 0x04)
#define DOWN   (key_scan(0x7) & 0x20)
#define RIGHT  (key_scan(0x7) & 0x10)
#define LEFT   (key_scan(0x7) & 0x08)
#define C_UP   (key_scan(0x5) & 0x02)
#define C_DOWN (key_scan(0x5) & 0x04)
#define ESC    (key_scan(0x0) & 0x01)
#define SPACE  (key_scan(0x6) & 0x10)
#define TAB    (key_scan(0x1) & 0x80)
#define SHIFT  (key_scan(0xe) & 0x01)
#define F1     (key_scan(0xc) & 0x04)
#define F2     (key_scan(0xc) & 0x08)
#define F3     (key_scan(0xc) & 0x10)
#define F4     (key_scan(0xc) & 0x20)
#define F5     (key_scan(0xc) & 0x40)
#define F6     (key_scan(0xc) & 0x80)
#define F7     (key_scan(0xd) & 0x01)
#define F8     (key_scan(0xd) & 0x02)
#define F9     (key_scan(0xd) & 0x04)
#define F10    (key_scan(0xd) & 0x08)

#define EOIDATA 0x20
#define EOI 0

void __interrupt __far ip_v_sync(void);
void __interrupt __far (*keepvector)(void);
volatile unsigned char __far vs_count;
unsigned short keepport;

void key_wait(void);
unsigned char key_scan(unsigned short);
void key_beep_off(void);
void key_flash(void);

union REGS reg;
union REGS reg_out;

/*キーグループの参照*/
unsigned char key_scan(unsigned short group)
{
	reg.h.ah = 0x04;
	reg.h.al = (unsigned char)group;
	int86(0x18, &reg, &reg);
	return(reg.h.ah);
}

void init_v_sync(void)
{
	_disable();
	keepport = inportb(2);
	keepvector = _dos_getvect(10);
	_dos_setvect(10, ip_v_sync);

	outportb(EOI, EOIDATA);
	outportb(2, keepport & 0xfb);

	outportb(0x64, 1);
	_enable();
}

void term_v_sync(void)
{
	_disable();
	_dos_setvect(10, keepvector);
	outportb(2, keepport);
	vs_count = 0;
	_enable();
}

void __interrupt __far ip_v_sync(void)
{
	++vs_count;
	outportb(0x64, 1);	/* VSYNC初期化 */
	outportb(EOI, EOIDATA);
}

/*タイマウェイト*/
void wait(unsigned short wait)
{
	while(1){
		_disable();
		if(vs_count >= wait)
			break;
		_enable();
	}
	_enable();
}

/*キー待ち*/
void key_wait(void)
{
	while(!kbhit());
}


/*キーバッファオーバー時のキー音をなくす*/
void key_beep_off(void)
{
	poke(0, 0x0500, (peek(0, 0x0500) | 0x20));
	poke(0, 0x0500, (peek(0, 0x0500) | 0xdf));
}

/*キーバッファ・クリア == 残っているキー入力を読み捨てる*/
void key_flash(void)
{
	while(kbhit())
	getch();
}

/* ジョイスティック PC-9801プログラマーズBibleより */
#define FM_PORT1	0x0188
#define FM_PORT2	0x018a

#define OPN_IO_A	0x0e
#define OPN_IO_B	0x0f

#define JOY_UP	0x01
#define	JOY_DOWN	0x02
#define JOY_LEFT	0x04
#define JOY_RIGHT	0x08
#define	JOY_TRIG1	0x10
#define	JOYTRIG2	0x20

void	InitJoystick(unsigned short stick);
unsigned char GetJoystick(void);

unsigned char triger(unsigned char joy_status);
unsigned char joy_key(unsigned char joy_status);


void InitJoystick(unsigned short stick)
{
	unsigned char result;
	_disable();
	outportb(FM_PORT1, 0x07);
	result = inportb(FM_PORT2);
	result &= 0x1f;
	result |= 0x80;
	outportb(FM_PORT1, 0x07);
	outportb(FM_PORT2, result);

	outportb(FM_PORT1,OPN_IO_B);
	if(stick == 2){
		outportb(FM_PORT2, 0xcf);
	}else{
		outportb(FM_PORT2, 0x8f);
	}
	_enable();
}

unsigned char GetJoystick(void)
{
	unsigned char result;

	_disable();
	outportb(FM_PORT1, OPN_IO_A);
	result = inportb(FM_PORT2);
	_enable();
	result ^= 0xff;
	result &= 0x3f;
	return result;
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

unsigned char fadeflag = 0;

/*メインルーチン
　初期設定とメインループ*/
void main()
{
	unsigned short mode = 1;

	unsigned short code = 0, code2 = 0, code3 = 0, code4 = 0;
	unsigned char old_map_data[(X_SIZE + 2) * 32];
	unsigned char sub_flag;
	unsigned char *map_adr;
	unsigned char *old_map_adr;

	unsigned char *data, *data_tmp;
	unsigned char pat_no, *pat_adr;
	unsigned char st;

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}
	bvram = (unsigned char __far *)MK_FP(0xa800, 0),
	rvram = (unsigned char __far *)MK_FP(0xb000, 0),
	gvram = (unsigned char __far *)MK_FP(0xb800, 0),
	ivram = (unsigned char __far *)MK_FP(0xe000, 0);

	key_beep_off();

	screen_switch(OFF);
	g_init(1);
	pal_all(0, org_pal);
	setpage(1,1);
	screen_switch(ON);

	if(conv("ELMAPCHR.SC5")){
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

	setpage( 0, 0);

/*マップのロード to (unsigned char mapdata[])*/

	if (mapload("ELMSTMAP.mdt")){
		exit(1);
	 }

	InitJoystick(1);
	pal_allblack(0);

/* ESCで抜けるまでループ*/
	outportb(0x62,0x0c);	/**Text OFF */
//	init_v_sync();

	do{
//		_disable();
//		vs_count = 0;
//		_enable();

		code = key_scan(0x7);
		code2 = key_scan(0x8);
		code3 = key_scan(0x9);
		code4 = key_scan(0x5);
		st = GetJoystick();

/*(	UP     (key_scan(0x7) & 0x04)
	DOWN   (key_scan(0x7) & 0x20)
	RIGHT  (key_scan(0x7) & 0x10)
	LEFT   (key_scan(0x7) & 0x08)	)*/

		if ((code & 0x04) || (code2 & 0x08) || (st & 0x01)){	/* 8 */
			if(y > 0){
				--y;
				dir = 0;
			}
		}
		if ((code & 0x10) || (code3 & 0x01) || (st & 0x08)){	/* 6 */
			if(x < (MAP_SIZE_X*2 - X_SIZE)){
				++x;
				dir = 1;
			}
		}
		if ((code & 0x20) || (code3 & 0x08) || (st & 0x02)){	/* 2 */
			if(y < (MAP_SIZE_Y*2 - Y_SIZE)){
				++y;
				dir = 2;
			}
		}
		if ((code & 0x08) || (code2 & 0x40) || (st & 0x04)){	/* 4 */
			if(x > 0){
				--x;
				dir = 3;
			}
		}
		if (ESC){
//			term_v_sync();
			break;
		}
		if ((st & 0x10) || (code4 & 0x02) || (SPACE)){	/* Z */
			if((st & 0x020) || (code4 & 0x04)){	/* X */
//				term_v_sync();
				break;
			}
		}

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

		sys_wait(2);
//		wait(2);

		if(fadeflag == 0){
			fadeflag = 1;
			fadeinblack(org_pal, 0, 3);
		}
	}while(1);
	fadeoutblack(org_pal, 0, 3);

//	term_v_sync();

	outportb(0x62,0x0d);	/**Text ON */

	end();
}
