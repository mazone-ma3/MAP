/* MSX2 キャラMAP移動実験  2x2圧縮展開 ZSDCC版 SCREEN5 By m@3 */
/* .COM版 */
/* キャラを出す */

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 32
#define LINE 212

#define HMMM 0xD0
#define LMMM 0x90

enum {
	VDP_READDATA = 0,
	VDP_READSTATUS = 1
};

enum {
	VDP_WRITEDATA = 0,
	VDP_WRITECONTROL = 1,
	VDP_WRITEPAL = 2,
	VDP_WRITEINDEX = 3
};

#define VDP_readport(no) (VDP_readadr + no)
#define VDP_writeport(no) (VDP_writeadr + no)

unsigned char VDP_readadr;
unsigned char VDP_writeadr;

//#define MAP_ADR 0x4000
//#define PARTS_DATA (MAP_ADR+0x3c00)

#define PARTS_HEAD 0x3c00 /*組み合わせキャラデータの先頭番地*/
#define BUFFSIZE 16384
unsigned char mapdata[BUFFSIZE];

FILE *stream[2];

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

#define PARTS_X 8
#define PARTS_Y 8

#define X_SIZE 18
#define Y_SIZE 18

#define MAP_SIZE_X 128
#define MAP_SIZE_Y 128

#define OFS_X 2
#define OFS_Y 2

#define CHR_X 8
#define CHR_Y 8

unsigned char map_data[(X_SIZE+2) * 32];

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

unsigned short vram_start_adr, vram_end_adr;
unsigned char page = 0;
unsigned char mode = 0;


/* mainromの指定番地の値を得る */
unsigned char read_mainrom(unsigned short adr) __sdcccall(1)
{
__asm
;	ld	 hl, #2
;	add	hl, sp
;	push	ix
;	ld	e, (hl)
;	inc	hl
;	ld	d, (hl)	; de=adr
;	ld	h,d
;	ld	l,e	; hl=adr

	ld	a,(#0xfcc1)	; exptbl
	call	#0x000c	; RDSLT

;	ld	l,a
;	ld	h,#0
;	pop	ix
__endasm;
}


void write_VDP(unsigned char regno, unsigned char data) __sdcccall(1)
{
	outp(VDP_writeport(VDP_WRITECONTROL), data);
	outp(VDP_writeport(VDP_WRITECONTROL), 0x80 | regno);
}


unsigned char read_VDPstatus(int no) __sdcccall(1)
{
	unsigned char data;
__asm
	DI
__endasm;
	write_VDP(15, no);
	data = inp(VDP_readport(VDP_READSTATUS));
	write_VDP(15, 0);
__asm
	EI
__endasm;
	return data;
}

unsigned char port,port2;

void wait_VDP(void) {
//	unsigned char data;
	port = VDP_writeport(VDP_WRITECONTROL);
	port2 = VDP_readport(VDP_READSTATUS);

/*	do{
__asm
	EI
__endasm;
__asm
	DI
__endasm;
		outp(port, 2);
		outp(port, 0x80 + 15);

		data = inp(port2);

		outp(port, 0);
		outp(port, 0x80 + 15);
	}while((data & 0x01));
*/

__asm
waitloop:
	ei
	di
	ld	a,(_port)
	ld	c,a
	ld	a,2
	out	(c),a
	ld	a,#0x80 + 15
	out	(c),a
	ld	b,c

	ld	a,(_port2)
	ld	c,a
	in a,(c)
	ld	c,b
	ld	b,a

	xor	a
	out	(c),a
	ld	a,#0x80 + 15
	out	(c),a

	ld	a,b
	and	a,#0x01
	jr	nz,waitloop
__endasm;
}

unsigned char vdpdata[0xf];

unsigned char port3, port4;

void VDPsetAREA(unsigned short sx, unsigned short sy, unsigned short dx, unsigned short dy, unsigned short nx, unsigned short ny, unsigned char dix, unsigned char diy, unsigned char command)
{
	port3 = VDP_writeport(VDP_WRITEINDEX);
	port4 = VDP_writeport(VDP_WRITECONTROL);

	vdpdata[0] = (sx & 0xff);		/* 32 */
	vdpdata[1] = ((sx >> 8) & 0x01);	/* 33 */
	vdpdata[2] = (sy & 0xff);		/* 34 */
	vdpdata[3] = ((sy >> 8) & 0x03);	/* 35 */
	vdpdata[4] = (dx & 0xff);		/* 36 */
	vdpdata[5] = ((dx >> 8) & 0x01);	/* 37 */
	vdpdata[6] = (dy & 0xff);		/* 38 */
	vdpdata[7] = ((dy >> 8) & 0x03);	/* 39 */
	vdpdata[8] = (nx & 0xff);		/* 40 */
	vdpdata[9] = ((nx >> 8) & 0x01);	/* 41 */
	vdpdata[0xa] = (ny & 0xff);		/* 42 */
	vdpdata[0xb] = ((ny >> 8) & 0x03);	/* 43 */
	vdpdata[0xc] = 0;
	vdpdata[0xd] = ((diy << 3) & 0x08) | ((dix << 2) & 0x04);	/* 45 */
	vdpdata[0xe] = command;

	wait_VDP();

/*	outp(port2, 32);
	outp(port2, 0x80 | 17);
*/
__asm
	ld	a,(_port4)
	ld	c,a
	ld	a,32
	out	(c),a
	ld	a,#0x80 | 17
	out	(c),a
__endasm;

//	outp(port3, data0);			/* 32 */
//	outp(port3, data1);			/* 33 */
//	outp(port3, data2);			/* 34 */
//	outp(port3, data3);			/* 35 */
//	outp(port3, data4);			/* 36 */
//	outp(port3, data5);			/* 37 */
//	outp(port3, data6);			/* 38 */
//	outp(port3, data7);			/* 39 */
//	outp(port3, data8);			/* 40 */
//	outp(port3, data9);			/* 41 */
//	outp(port3, dataa);			/* 42 */
//	outp(port3, datab);			/* 43 */
//	outp(port3, 0);				/* 44 */

//	outp(port3, datad);	/* 45 */

//	outp(port3, command);
__asm
	ld	b,0x0f
	ld	a,(_port3)
	ld	c,a
	ld	hl,_vdpdata

	outi	/* 32 */
	outi	/* 33 */
	outi	/* 34 */
	outi	/* 35 */
	outi	/* 36 */
	outi	/* 37 */
	outi	/* 38 */
	outi	/* 39 */
	outi	/* 40 */
	outi	/* 41 */
	outi	/* 42 */
	outi	/* 43 */
	outi	/* 44 */
	outi	/* 45 */
	outi	/* com */
	ei
__endasm;
}

void write_vram_adr(unsigned char highadr, int lowadr) __sdcccall(1)
{
	write_VDP(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
	outp(VDP_writeport(VDP_WRITECONTROL), (lowadr & 0xff));
	outp(VDP_writeport(VDP_WRITECONTROL), 0x40 | ((lowadr >> 8) & 0x3f));
}

void write_vram_data(unsigned char data) __sdcccall(1)
{
	outp(VDP_writeport(VDP_WRITEDATA), data);
}

unsigned char pattern[10];

/* screenのBIOS切り替え */
void set_screenmode(unsigned char mode) __sdcccall(1)
{
__asm
;	ld	 hl, 2
;	add	hl, sp
	ld	l,a
	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x005f	; CHGMOD(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

void set_displaypage(int page) __sdcccall(1)
{
__asm
	DI
__endasm;
	write_VDP(2, (page << 5) & 0x60 | 0x1f);
__asm
	EI
__endasm;
}


long l, count, count2, n = 0;
short m = 0;
#define read_pattern mapdata

int bload(char *loadfil) __sdcccall(1)
{
	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);

//		fclose(stream[0]);
		return ERROR;
	}

	fread(pattern, 1, 1, stream[0]);	/* MSX先頭ヘッダ */
	if(pattern[0] != 0xfe){
		printf("Not BSAVE,S file %s.", loadfil);
		fclose(stream[0]);
//		term();
//		exit(1);
		return ERROR;
	}

	set_screenmode(5);
	VDP_readadr = read_mainrom(#0x0006);
	VDP_writeadr = read_mainrom(#0x0007);

	fread(pattern, 1, 2, stream[0]);	/* MSXヘッダ 開始アドレス */
	vram_start_adr = pattern[0] + pattern[1] * 256;

	fread(pattern, 1, 2, stream[0]);	/* MSXヘッダ 終了アドレス */
	vram_end_adr = pattern[0] + pattern[1] * 256;

	fread(pattern, 1, 2, stream[0]);	/* MSXヘッダ 0 */

	switch(page){
		case 1:
			write_vram_adr(0x00, vram_start_adr + 0x8000);
			break;
		case 2:
			write_vram_adr(0x01, vram_start_adr);
			break;
		case 3:
			write_vram_adr(0x01, vram_start_adr + 0x8000);
			break;
		default:
			write_vram_adr(0x00, vram_start_adr);
			break;
	}
	if(page < 4)
		if(mode)
			set_displaypage(page);

	n = vram_start_adr;
	for(count = 0; count < 4; ++count){
		l = fread(read_pattern, 1, WIDTH * LINE, stream[0]);
		if(l < 1)
			break;
		m = 0;
		for(count2 = 0; count2 < WIDTH * LINE; ++count2){
			write_vram_data(read_pattern[m]);
			if(n == vram_end_adr){
				fclose(stream[0]);
				return NOERROR;
			}
			++m;
			++n;
		}
	}
	fclose(stream[0]);

	return NOERROR;
}

/*マップデータをファイルから配列に読み込む*/
short mapload(char *loadfil) __sdcccall(1)
{
//	short inhandle;

//	if ((inhandle = open( loadfil, O_RDONLY )) == -1) { //| O_BINARY
	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
//	read( inhandle, mapdata, 7);
	fread( mapdata, 1, 7, stream[0]);
//	read( inhandle, mapdata, BUFFSIZE);
	fread( mapdata, 1, BUFFSIZE, stream[0]);
//	close(inhandle);
	fclose(stream[0]);
	return NOERROR;
}

unsigned char get_key(unsigned char matrix) __sdcccall(1)
{
__asm
;	ld	 hl, #2
;	add	hl, sp
	ld	l,a
	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x0141	; SNSMAT(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT

;	ld	l,a
;	ld	h,#0

	pop	ix
__endasm;
}

unsigned char get_stick(unsigned char trigno) __sdcccall(1)
{
__asm
;	ld	 hl, #2
;	add	hl, sp
	ld	l,a
	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x00d5	; GTSTCK(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT
;	ld	l,a
;	ld	h,#0
	pop	ix
__endasm;
}

unsigned char get_pad(unsigned char trigno) __sdcccall(1)
{
__asm
;	ld	 hl, #2
;	add	hl, sp
	ld	l,a
	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x00d8	; GTTRIG(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT
;	ld	l,a
;	ld	h,#0
	pop	ix
__endasm;
}

/* DISK BASIC only. */
/*volatile void Set_RAM_MODE(void){
__asm
	ld	a,(#0xf342)
	ld	hl,#0x4000
	call	#0x0024
__endasm;
}

volatile void Set_ROM_MODE(void){
__asm
	ld	a,(#0xfcc1)
	ld	hl,#0x4000
	call	#0x0024
__endasm;
}
*/

void set_screencolor(void)
{
__asm
	push	ix
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x0062	; CHGCLR(MAINROM)

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

void key_flush(void)
{
__asm
	push	ix
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x0156	; KILBUF(MAINROM)

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

unsigned char map_no;
unsigned char map_no2;
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
unsigned short vx, vy;

void chr_sub(void)
{
	unsigned short no = map_no;
	unsigned short no2 = chr_tbl[dir * 2 + dir2][(i - CHR_X) + (j - CHR_Y) * 2];

	VDPsetAREA((no & 0x0f) * PARTS_X, (no & 0xf0) / (PARTS_Y / 4) + 256, 0, 212, PARTS_X, PARTS_Y, 0,0, HMMM);

	VDPsetAREA((no2 & 0x0f) * PARTS_X + (128 * (PARTS_X / 8)), (no2 & 0xf0) / (PARTS_Y /4) + 256, 0, 212, PARTS_X, PARTS_Y, 0,0, LMMM | 0x08);

	VDPsetAREA(0, 212, vx, vy, PARTS_X, PARTS_Y, 0,0, HMMM);
}

void pat_sub(void)
{
	VDPsetAREA((map_no & 0x0f) * PARTS_X, (map_no & 0xf0) / (PARTS_Y / 4) + 256, vx, vy, PARTS_X, PARTS_Y, 0,0, HMMM);
}

/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	unsigned char port = VDP_writeport(VDP_WRITEPAL);
	write_VDP(16, color);
	outp(port, red * 16 | blue);
	outp(port, green);
}

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned short i;
	for(i = 0; i < MAXCOLOR; i++)
		pal_set(pal_no, i, color[i][0]/2, color[i][1]/2, color[i][2]/2);
}

void wait_vsync(void)
{
	while((read_VDPstatus(2) & 0x40));
	while(!(read_VDPstatus(2) & 0x40)); /* WAIT VSYNC */
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
	char j;
	for(j = 0; j < MAXCOLOR; j++)
		pal_set(pal_no, j, 0, 0, 0);
}

unsigned char st0, st1, pd0, pd1, pd2, k3, k5, k7, k9, k10;
unsigned char *data, *data_tmp;
unsigned short pat_no;
unsigned char *pat_adr;
unsigned short x = 165, y = 30,xx, yy, old_x = 255, old_y = 255;
//unsigned short k;

unsigned char old_map_data[(X_SIZE + 2) * 32];
unsigned char sub_flag;
unsigned char *map_adr;
unsigned char *old_map_adr;

unsigned char fadeflag = 0;

unsigned char *forclr = 0xf3e9;
unsigned char *bakclr = 0xf3ea;
unsigned char *bdrclr = 0xf3eb;
unsigned char *clicksw = 0xf3db;
unsigned char *oldscr = 0xfcb0;

unsigned char forclr_old, bakclr_old, bdrclr_old, clicksw_old;

int main(void)
{
//	set_vol(16);
//	unsigned char ii,jj;

	VDP_readadr = read_mainrom(0x0006);
	VDP_writeadr = read_mainrom(0x0007);

//	unsigned char ii,jj;

/*マップのロード to (unsigned char mapdata[])*/
	page = 1;
	if(bload("ELMAPCHR.SC5"))
		return ERROR;

	set_screenmode(*oldscr);

	if(mapload("ELMSTMAP.MDT")){
//		term();
		return ERROR;
	}

	forclr_old = *forclr;
	bakclr_old = *bakclr;
	bdrclr_old = *bdrclr;

	*forclr = 15;
	*bakclr = 0;
	*bdrclr = 0;
	set_screencolor();

	clicksw_old = *clicksw;
	*clicksw = 0;

	set_screenmode(5);
	pal_allblack(0);

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}

//	Set_RAM_MODE();

	while(((k7 = get_key(7)) & 0x04)){ /* ESC */
		st0 = get_stick(0);
		st1 = get_stick(1);

		pd0 = get_pad(0);
		pd1 = get_pad(1);
		pd2 = get_pad(3);

		k3 = get_key(3);
		k9 = get_key(9);
		k10 = get_key(10);
		k5 = get_key(5);

__asm
	EI
__endasm;

		if((st0 >= 1 && st0 <=2) || (st0 == 8) || (st1 >= 1 && st1 <=2) || (st1 ==8) || !(k10 & 0x08)){ /* 8 */
			if(y > 0){
				--y;
				dir = 0;
			}
		}
		if((st0 >= 2 && st0 <=4) || (st1 >= 2 && st1 <=4) || !(k10 & 0x02)){ /* 6 */
			if(x < (MAP_SIZE_X*2 - X_SIZE)){
				++x;
				dir = 1;
			}
		}
		if((st0 >= 4 && st0 <=6) || (st1 >= 4 && st1 <=6) || !(k9 & 0x20)){ /* 2 */
			if(y < (MAP_SIZE_Y*2 - Y_SIZE)){
				++y;
				dir = 2;
			}
		}
		if((st0 >= 6 && st0 <=8) || (st1 >= 6 && st1 <=8) || !(k9 & 0x80)){ /* 4 */
			if(x > 0){
				--x;
				dir = 3;
			}
		}
		if((pd0) || (pd1) || !(k5 & 0x20)) /* X,SPACE */
			if((pd2) || !(k3 & 0x01)) /* C */
				break;

		if((old_x != x) || (old_y != y)){
			xx = 1 - x % 2;
			yy = 1 - y % 2;
//			data = (unsigned char *)((x / 2) + (y / 2) * MAP_SIZE_X);
//			data += MAP_ADR;
			data = &mapdata[x / 2 + (y / 2) * MAP_SIZE_X];
			data_tmp = data;
			map_adr = &map_data[xx + yy * 32];

			for(j = 0; j < Y_SIZE / 2; ++j){

				for(i = 0; i < X_SIZE / 2; ++i){
					pat_no = (((*data >> 4) & 0x0f) | ((*data << 4) & 0xf0));

//					pat_adr = (unsigned char *)(pat_no * 4 + PARTS_DATA);
					pat_adr = &mapdata[PARTS_HEAD + pat_no * 4];
//					ii = i * 2 + xx;
//					jj = j * 2 + yy;
					*map_adr = *pat_adr;
					++map_adr;
					++pat_adr;
					*map_adr =  *pat_adr;
					++pat_adr;
					map_adr += (32 - 1);
					*map_adr = *pat_adr;
					++map_adr;
					++pat_adr;
					*map_adr =  *pat_adr;
					map_adr -= (32 - 1);
/*					map_data[ii][jj] = *pat_adr;
					++pat_adr;
					++ii;
					map_data[ii][jj] = *pat_adr;
					++pat_adr;
					--ii;
					++jj;
					map_data[ii][jj] = *pat_adr;
					++pat_adr;
					++ii;
					map_data[ii][jj] = *pat_adr;
*/					++data;
				}
				data_tmp += MAP_SIZE_X;
				data = data_tmp;
				map_adr += (32 * 2 - X_SIZE);
			}
			dir2 = 1 - dir2;

			vx = (OFS_X) * PARTS_X;
			vy = (OFS_Y) * PARTS_Y;
			map_adr = &map_data[1 + 1 * 32];
			old_map_adr = &old_map_data[1 + 1 * 32];

			if(old_x != x){
				for(i = 1; i < (X_SIZE - 1); ++i, vx += PARTS_X){
					for(j = 1, vy = (OFS_Y) * PARTS_Y; j < (Y_SIZE - 1); ++j, vy += PARTS_Y){
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
						map_no  = *map_adr;
						map_no2 = *old_map_adr;
						if(sub_flag){
							chr_sub();
						}else{
							if(map_no != map_no2){
								pat_sub();
								*old_map_adr = map_no;
							}
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
				for(j = 1; j < (Y_SIZE - 1); ++j, vy += PARTS_Y){
					for(i = 1, vx = (OFS_X) * PARTS_X; i < (X_SIZE - 1); ++i, vx += PARTS_X){
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
						map_no  = *map_adr;
						map_no2 = *old_map_adr;
						if(sub_flag){
							chr_sub();
						}else{
							if(map_no != map_no2){
								pat_sub();
								*old_map_adr = map_no;
							}
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

		if(fadeflag == 0){
			fadeflag = 1;
			fadeinblack(org_pal, 0, 3);
		}
	}
	fadeoutblack(org_pal, 0, 3);
//	Set_ROM_MODE();

	set_screenmode(*oldscr);
	pal_all(0, org_pal);
	*forclr = forclr_old;
	*bakclr = bakclr_old;
	*bdrclr = bdrclr_old;
	set_screencolor();
	*clicksw = clicksw_old;
	key_flush();
	return NOERROR;
}

