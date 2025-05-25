/* MSX2 キャラMAP移動実験  2x2圧縮展開 ZSDCC版 SCREEN5 By m@3 */
/* .COM版 ダブルバッファ化 */
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
unsigned char map_page = 0;

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
//	outp(VDP_writeport(VDP_WRITECONTROL), data);
//	outp(VDP_writeport(VDP_WRITECONTROL), 0x80 | regno);
__asm
	ld	h,a
	ld	a,(_VDP_writeadr)
	inc	a
	ld	c,a
	ld	a,l
	out	(c),a
	ld	a,h
	set 7,a
	out	(c),a
__endasm;
}


unsigned char read_VDPstatus(unsigned char no) __sdcccall(1)
{
	unsigned char data;
__asm
	DI
__endasm;
	write_VDP(15, no);
//	data = inp(VDP_readport(VDP_READSTATUS));
__asm
	ld	a,(_VDP_readadr)
	inc	a
	ld	c,a
	in a,(c)
	push	af
__endasm;
	write_VDP(15, 0);
__asm
	pop	af
	EI
__endasm;
//	return data;
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
;	ld	b,a
	rra

;	xor	a
	ld	a,0
	out	(c),a
	ld	a,#0x80 + 15
	out	(c),a

	jr	c,waitloop

;	ld	a,b
;	and	a,#0x01
;	jr	nz,waitloop
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
/*
	outp(port4, 32);
	outp(port4, 0x80 | 17);
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

unsigned char sx, sy, dx, dy; //, nc, ny, dix, diy, 
unsigned char command;
unsigned char APAGE,VPAGE,XSIZE,XSIZA,YSIZE;

void VDPsetAREA2(void)
/*unsigned short sx, unsigned short sy, unsigned short dx, unsigned short dy, unsigned short nx, unsigned short ny, unsigned char dix, unsigned char diy, unsigned char command)*/
{
	port3 = VDP_writeport(VDP_WRITEINDEX);
	port4 = VDP_writeport(VDP_WRITECONTROL);

//	vdpdata[0] = (sx & 0xff);		/* 32 */
//	vdpdata[1] = ((sx >> 8) & 0x01);	/* 33 */
//	vdpdata[2] = (sy & 0xff);		/* 34 */
//	vdpdata[3] = ((sy >> 8) & 0x03);	/* 35 */
//	vdpdata[4] = (dx & 0xff);		/* 36 */
//	vdpdata[5] = ((dx >> 8) & 0x01);	/* 37 */
//	vdpdata[6] = (dy & 0xff);		/* 38 */
//	vdpdata[7] = ((dy >> 8) & 0x03);	/* 39 */
//	vdpdata[8] = (nx & 0xff);		/* 40 */
//	vdpdata[9] = ((nx >> 8) & 0x01);	/* 41 */
//	vdpdata[0xa] = (ny & 0xff);		/* 42 */
//	vdpdata[0xb] = ((ny >> 8) & 0x03);	/* 43 */
//	vdpdata[0xc] = 0;
//	vdpdata[0xd] = ((diy << 3) & 0x08) | ((dix << 2) & 0x04);	/* 45 */
//	vdpdata[0xe] = command;
__asm
	ld	a,(_sx)	;SX
	ld	h,a
	ld	a,(_sy)	;SY
	ld	l,a
;	ld	a,(_vdpdata+3)
;	ld	(_APAGE),a
	ld	a,(_dx)	;DX
	ld	d,a
	ld	a,(_dy)	;DY
	ld	e,a
;	ld	a,(_vdpdata+7)
;	ld	(_VPAGE),a
;	ld	a,(_vdpdata+8)
;	ld	(_XSIZE),a
;	ld	a,(_vdpdata+9)
;	ld	(_XSIZA),a
;	ld	a,(_vdpdata+0xa)
;	ld	(_YSIZE),a
;	exx
__endasm;
	wait_VDP();
/*
	outp(port4, 32);
	outp(port4, 0x80 | 17);
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
;	exx

	ld	b,0x0f
	ld	a,(_port3)
	ld	c,a
;	ld	hl,_vdpdata

	XOR	A
	OUT	(C),H	;SX
	OUT	(C),A	
	LD	A,(_APAGE)
	OUT	(C),L	;SY
	OUT	(C),A	

	XOR	A
	OUT	(C),D	;DX
	OUT	(C),A	
	LD	A,(_VPAGE)
	OUT	(C),E	;DY
	OUT	(C),A
	LD	A,(_XSIZE)
	LD	B,A
	LD	A,(_XSIZA)
	OUT	(C),B
	OUT	(C),A
	LD	A,(_YSIZE)
	LD	B,A
	XOR	A
	OUT	(C),B
	OUT	(C),A
	OUT	(C),A	;DUMMY

	LD	A,H
	SUB	D
	LD	A,0
	JR	C,DQ
DQ:	OR	2

	OUT	(C),A	;DIX and DIY

	ld	a,(_command)
	out	(C),a	/* com */
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

short bload(char *loadfil) __sdcccall(1)
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
unsigned short i, j, k;
unsigned char vx, vy;

void chr_sub(void)
{
	unsigned short no = map_no;
	unsigned short no2 = chr_tbl[dir * 2 + dir2][(i - CHR_X) + (j - CHR_Y) * 2];

//	VDPsetAREA((no & 0x0f) * PARTS_X, (no & 0xf0) / (PARTS_Y / 4) + 512, 0, 212, PARTS_X, PARTS_Y, 0,0, HMMM);
	sx = (no & 0x0f) * PARTS_X;
	sy = (no & 0xf0) / (PARTS_Y / 4); // + 512;
	dx = 0;
	dy = 212;
	XSIZE = PARTS_X;
	XSIZA = 0;
	YSIZE = PARTS_Y;
	APAGE = 2; //map_page;
	VPAGE = map_page;
	command = HMMM;
	VDPsetAREA2();

//	VDPsetAREA((no2 & 0x0f) * PARTS_X + (128 * (PARTS_X / 8)), (no2 & 0xf0) / (PARTS_Y /4) + 512, 0, 212, PARTS_X, PARTS_Y, 0,0, LMMM | 0x08);
	sx = (no2 & 0x0f) * PARTS_X + (128 * (PARTS_X / 8));
	sy = (no2 & 0xf0) / (PARTS_Y /4); // + 512;
	command = LMMM | 0x08;
	VDPsetAREA2();

//	VDPsetAREA(0, 212, vx, vy + 256 * map_page, PARTS_X, PARTS_Y, 0,0, HMMM);
	sx = 0;
	sy = 212;
	dx = vx;
	dy = vy;
	APAGE = map_page;
	command = HMMM;
	VDPsetAREA2();
}

void chr_sub2(void)
{
	unsigned short no = map_no;
	unsigned short no2 = chr_tbl[dir * 2 + dir2][0];

//	VDPsetAREA((no2 & 0x0f) * PARTS_X + (128 * (PARTS_X / 8)), (no2 & 0xf0) / (PARTS_Y /4) + 512, 0, 212, PARTS_X, PARTS_Y, 0,0, LMMM | 0x08);
	sx = (no2 & 0x0f) * PARTS_X + (128 * (PARTS_X / 8));
	sy = (no2 & 0xf0) / (PARTS_Y /4); // + 512;
	dx = (CHR_X+1) * PARTS_X;
	dy = (CHR_Y+1) * PARTS_Y;
	XSIZE = PARTS_X*2;
	XSIZA = 0;
	YSIZE = PARTS_Y*2;
	APAGE = 2; //map_page;
	VPAGE = map_page;
	command = LMMM | 0x08;
	VDPsetAREA2();
}

void pat_sub(void)
{
//	VDPsetAREA((map_no & 0x0f) * PARTS_X, (map_no & 0xf0) / (PARTS_Y / 4) + 512, vx, vy + 256 * map_page, PARTS_X, PARTS_Y, 0,0, HMMM);

	sx = (map_no & 0x0f) * PARTS_X;
	sy = (map_no & 0xf0) / (PARTS_Y / 4); // + 512;
	dx = vx;
	dy = vy;
/*	XSIZE = PARTS_X;
	XSIZA = 0;
	YSIZE = PARTS_Y;
	APAGE = 2; //map_page;
	VPAGE = map_page;
	command = HMMM;
*/	VDPsetAREA2();

//unsigned char sx, sy, dx, dy; //, nc, ny, dix, diy, 
//unsigned char command;
//unsigned char APAGE,VPAGE,XSIZE,XSIZA,YSIZE;

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

unsigned char old_map_data[2][(X_SIZE + 2) * 32];
unsigned char sub_flag;
volatile unsigned char *map_adr;
volatile unsigned char *old_map_adr;

unsigned char fadeflag = 0;

unsigned char *forclr = 0xf3e9;
unsigned char *bakclr = 0xf3ea;
unsigned char *bdrclr = 0xf3eb;
unsigned char *clicksw = 0xf3db;
unsigned char *oldscr = 0xfcb0;

unsigned char forclr_old, bakclr_old, bdrclr_old, clicksw_old;

void map_sub(void)
{
/*
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
					map_adr -= (32 - 1);*/
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
*//*					++data;
				}
				data_tmp += MAP_SIZE_X;
				data = data_tmp;
				map_adr += (32 * 2 - X_SIZE);
			}*/
/*__asm
	ld	a,(_yy)
	ld	l,a
	ld	h,a
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	ld	a,(_xx)
	ld	c,a
	ld	b,0
	add	hl,bc
	ld	bc,_map_data
	add	hl,bc
	ld	(_map_adr),hl*/
__asm
	ld	de,(_map_adr)
	ld	b,Y_SIZE / 2
loop0:
	push	bc
	ld	b,X_SIZE / 2
loop1:
	push	bc
	ld	hl,(_data)
	ld	a,(hl)
	rlc	a
	rlc	a
	rlc	a
	rlc	a

	ld	l,a
	ld	h,0
	add	hl,hl
	add	hl,hl
	ld	bc,PARTS_HEAD
	add	hl,bc
	ld	bc,_mapdata
	add	hl,bc
	ex	de,hl

;	ld	hl,(_map_adr)

	ld	a,(de)
	ld	(hl),a
	inc	de
	inc	hl

	ld	a,(de)
	ld	(hl),a
	inc	de
	ld	bc,32-1
	add	hl,bc

	ld	a,(de)
	ld	(hl),a
	inc	de
	inc	hl

	ld	a,(de)
	ld	(hl),a
	ld	bc,-(32-1)
	add	hl,bc

;	ld	(_map_adr),hl

	ex	de,hl
	ld	hl,(_data)
	inc	hl
	ld	(_data),hl

	pop	bc
	djnz loop1

;	push	bc
	ex	de,hl
;	ld	hl,(_map_adr)
	ld	bc,32 * 2 - X_SIZE
	add	hl,bc
;	ld	(_map_adr),hl
	ex	de,hl

	ld	hl,(_data_tmp)
	ld	bc,MAP_SIZE_X
	add	hl,bc
	ld	(_data_tmp),hl
	ld	(_data),hl

	pop	bc
	djnz loop0

;	ld	hl,_map_data+1+1*32
;	ld	(_map_adr),hl
__endasm;
}

int main(void)
{
//	set_vol(16);
//	unsigned char ii,jj;
	short waku;
	map_page = 0;

	VDP_readadr = read_mainrom(0x0006);
	VDP_writeadr = read_mainrom(0x0007);

//	unsigned char ii,jj;

/*マップのロード to (unsigned char mapdata[])*/
	page = 2;
	if(bload("ELMAPCHR.SC5")){
		set_screenmode(*oldscr);
		return ERROR;
	}
	page = 1;
	waku = bload("ELMWAKU.SC5");

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

	if(waku)
		VDPsetAREA(0, 0, 0, 256, 256, 212, 0,0, HMMM);
	else
		VDPsetAREA(0, 256, 0, 0, 256, 212, 0,0, HMMM);

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			for(k = 0; k < 2; ++k){
				old_map_data[k][i +  j * 32] = 255-5;
			}
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

			map_sub();

			dir2 = 1 - dir2;

			vx = (OFS_X) * PARTS_X;
			vy = (OFS_Y) * PARTS_Y;
			map_adr = &map_data[1 + 1 * 32];
			map_page = 1 - map_page;
			old_map_adr = &old_map_data[map_page][1 + 1 * 32];

/*			if(old_x != x){
				for(i = 1; i < (X_SIZE - 1); ++i, vx += PARTS_X){
					for(j = 1, vy = (OFS_Y) * PARTS_Y; j < (Y_SIZE - 1); ++j, vy += PARTS_Y){*/
/*						sub_flag = 0;
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
//						map_no2 = *old_map_adr;
						if(sub_flag){
							chr_sub();
						}else{
//							if(map_no != map_no2){
							if(map_no != *old_map_adr){
								pat_sub();
								*old_map_adr = map_no;
							}
						}
*/
//	old_map_adr[(CHR_X - 1) + (CHR_Y - 1) * 32] = 255-5;
//	old_map_adr[(CHR_X - 0) + (CHR_Y - 1) * 32] = 255-5;
//	old_map_adr[(CHR_X - 1) + (CHR_Y - 0) * 32] = 255-5;
//	old_map_adr[(CHR_X - 0) + (CHR_Y - 0) * 32] = 255-5;
	XSIZE = PARTS_X;
	XSIZA = 0;
	YSIZE = PARTS_Y;
	APAGE = 2; //map_page;
	VPAGE = map_page;
	command = HMMM;
__asm
	ld	hl,(_old_map_adr)
	ld	bc,(CHR_X - 1) + (CHR_Y - 1) * 32
	add	hl,bc
	ld	a,255-5
	ld	(hl),a
	inc	hl
	ld	(hl),a
	ld	bc,32 - 1
	add	hl,bc
	ld	(hl),a
	inc	hl
	ld	(hl),a

	ld	hl,_old_x
	ld	a,(_x)
	cp	a,(hl)
	jp	z,skip00

	ld	hl,(_map_adr)
	ld	de,(_old_map_adr)

;	ld	c,1
	ld	c,X_SIZE - 2
skip01:
;	ld	a,c
;	add	a,-(X_SIZE - 2) + 1
;	ld	(_i),a
	ld	a,(OFS_Y) * PARTS_Y
	ld	(_vy),a
;	ld	b,1
	ld	b,Y_SIZE - 2
skip02:
;	ld	a,b
;	add	a,-(Y_SIZE - 2) + 1
;	ld	(_j),a
	push	bc
;	ld	hl,(_map_adr)
	ld	a,(hl)
	ld	(_map_no),a
;	ld	a,(_i)
;	cp	a,CHR_X	; i - CHR_X >= 0
;	jr	c,skip1
;	cp	a,CHR_X+2	; i - (CHR_X+1) <= 0
;	jr	nc,skip1
;	ld	a,(_j)
;	cp	a,CHR_Y	; j - CHR_Y >= 0
;	jr	c,skip1
;	cp	a,CHR_Y+2	; j - (CHR_Y+1) <= 0
;	jr	nc,skip1
;	push	bc
;	push	hl
;	push	de
;	call	_chr_sub
;	pop	de
;	pop	hl
;	pop	bc
;	jr	skip2
skip1:
;	ld	de,(_old_map_adr)
	ld	a,(de)
	cp	a,(hl)
	jr	z,skip2
	ld	a,(hl)
	ld	(de),a

	ld	a,c
	add	a,-(X_SIZE - 2) + 1
	ld	(_i),a
	ld	a,b
	add	a,-(Y_SIZE - 2) + 1
	ld	(_j),a
;	push	bc
	push	hl
	push	de
	call	_pat_sub
	pop	de
	pop	hl
;	pop	bc
skip2:
;	push	bc
	ld	bc,32
;	ld	hl,(_map_adr)
	add	hl,bc
;	ld	(_map_adr),hl
	ex	de,hl
;	ld	hl,(_old_map_adr)
	add	hl,bc
;	ld	(_old_map_adr),hl
	ex	de,hl
	ld	a,(_vy)
;	ld	de,PARTS_Y
;	add	hl,de
	add	a,PARTS_Y
	ld	(_vy),a
	pop	bc

;	ld	hl,_j
;	inc	(hl)
;	inc	b
;	ld	a,(hl)
;	ld	a,b
;	cp	a,Y_SIZE-1
;	jr	nz,skip02
	djnz	skip02

	push	bc
	ld	bc,1 - 32 * (X_SIZE - 2)
;	ld	hl,(_map_adr)
	add	hl,bc
;	ld	(_map_adr),hl
;	ld	hl,(_old_map_adr)
	ex	de,hl
	add	hl,bc
	ex	de,hl
;	ld	(_old_map_adr),hl
	ld	a,(_vx)
;	ld	de,PARTS_X
;	add	hl,de
	add	a,PARTS_X
	ld	(_vx),a

;	ld	hl,_i
;	inc	(hl)
;	inc	c
;	ld	a,(hl)
;	ld	a,c
;	cp	a,X_SIZE-1
	pop	bc
	dec	c
	jp	nz,skip01
	jp	skip05
__endasm;
/*						map_adr += 32;
						old_map_adr += 32;
					}
					map_adr += (1 - 32 * (X_SIZE - 2));
					old_map_adr += (1 - 32 * (X_SIZE - 2));
				}
			}
			else
			{
				for(j = 1; j < (Y_SIZE - 1); ++j, vy += PARTS_Y){
					for(i = 1, vx = (OFS_X) * PARTS_X; i < (X_SIZE - 1); ++i, vx += PARTS_X){*/
/*						sub_flag = 0;
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
//						map_no2 = *old_map_adr;
						if(sub_flag){
							chr_sub();
						}else{
//							if(map_no != map_no2){
							if(map_no != *old_map_adr){
								pat_sub();
								*old_map_adr = map_no;
							}
						}*/
__asm
skip00:
	ld	hl,(_map_adr)
	ld	de,(_old_map_adr)

;	ld	c,1
	ld	c,Y_SIZE - 2
skip03:
;	ld	a,c
;	add	a,-(Y_SIZE - 2) + 1
;	ld	(_j),a
	ld	a,(OFS_X) * PARTS_X
	ld	(_vx),a
;	ld	b,1
	ld	b,X_SIZE - 2
skip04:
;	ld	a,b
;	add	a,-(X_SIZE - 2) + 1
;	ld	(_i),a
;	push	bc
;	ld	hl,(_map_adr)
	ld	a,(hl)
	ld	(_map_no),a
;	ld	a,(_i)
;	cp	a,CHR_X	; i - CHR_X >= 0
;	jr	c,skip4
;	cp	a,CHR_X+2	; i - (CHR_X+1) <= 0
;	jr	nc,skip4
;	ld	a,(_j)
;	cp	a,CHR_Y	; j - CHR_Y >= 0
;	jr	c,skip4
;	cp	a,CHR_Y+2	; j - (CHR_Y+1) <= 0
;	jr	nc,skip4
;	push	bc
;	push	hl
;	push	de
;	call	_chr_sub
;	pop	de
;	pop	hl
;	pop	bc
;	jr	skip5
skip4:
;	ld	a,(hl)
;	ld	de,(_old_map_adr)
	ld	a,(de)
	cp	a,(hl)
	jr	z,skip5
	ld	a,(hl)
	ld	(de),a

	ld	a,b
	add	a,-(X_SIZE - 2) + 1
	ld	(_i),a
	ld	a,c
	add	a,-(Y_SIZE - 2) + 1
	ld	(_j),a
	push	bc
	push	hl
	push	de
	call	_pat_sub
	pop	de
	pop	hl
	pop	bc
skip5:
;	ld	hl,(_map_adr)
	inc	hl
;	ld	(_map_adr),hl
;	ld	de,(_old_map_adr)
	inc	de
;	ld	(_old_map_adr),de
	ld	a,(_vx)
;	ld	de,PARTS_X
;	add	hl,de
	add	a,PARTS_X
	ld	(_vx),a

;	pop	bc
;	ld	hl,_i
;	inc	(hl)
;	inc	b
;	ld	a,b
;	ld	a,(hl)
;	cp	a,X_SIZE-1
;	jr	nz,skip04
	djnz	skip04

	push	bc
	ld	bc,32 - (X_SIZE - 2)
;	ld	hl,(_map_adr)
	add	hl,bc
;	ld	(_map_adr),hl

	ex	de,hl
;	ld	hl,(_old_map_adr)
	add	hl,bc
;	ld	(_old_map_adr),hl
	ex	de,hl

	ld	a,(_vy)
;	ld	de,PARTS_Y
;	add	hl,de
	add	a,PARTS_Y
	ld	(_vy),a

;	ld	hl,_j
;	inc	(hl)
;	inc	c
;	ld	a,c
;	ld	a,(hl)
;	cp	a,Y_SIZE - 1

	pop	bc
	dec	c
	jp	nz,skip03
skip05:
__endasm;
/*						++map_adr;
						++old_map_adr;
					}
					map_adr += (32 - (X_SIZE - 2));
					old_map_adr += (32 - (X_SIZE - 2));
				}
			}*/
			chr_sub2();
			wait_VDP();
		}
		old_x = x;
		old_y = y;
		set_displaypage(map_page);

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

