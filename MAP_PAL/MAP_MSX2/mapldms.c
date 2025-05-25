/* z88dk MSX2 VRAMのマップを裏RAMに転送するだけのプログラム */
/* SCREEN5にしてから実行 */
/* DISK BASIC only */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#define SPR_ATR_ADR 0x7600
#define SPR_PAT_ADR 0x7800
#define SPR_COL_ADR (SPR_ATR_ADR-512)

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

/*unsigned char VDP_readport[4], VDP_writeport(4];*/

#define VDP_readport(no) (VDP_readadr + no)
#define VDP_writeport(no) (VDP_writeadr + no)

unsigned char VDP_readadr;
unsigned char VDP_writeadr;

void DI(void){
#asm
	DI
#endasm
}

void EI(void){
#asm
	EI
#endasm
}

/* DISK BASIC only. */
void Set_RAM_MODE(void){
#asm
//	DI
	ld	a,($f342)
	ld	hl,$4000
	call	$0024
#endasm
}

void Set_ROM_MODE(void){
#asm
	ld	a,($fcc1)
	ld	hl,$4000
	call	$0024
//	EI
#endasm
}
/* mainromの指定番地の値を得る */
int read_mainrom(int adr)
{
#asm
	ld	 hl, 2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)	; de=adr
	ld	h,d
	ld	l,e	; hl=adr

	ld	a,($fcc1)	; exptbl
	call	$000c	; RDSLT

	ld	l,a
	ld	h,0
#endasm
}

/* screenのBIOS切り替え */
void set_screenmode(int mode)
{
#asm
	ld	a,($fcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,$005f	; CHGMOD(MAINROM)

	ld	 hl, 2
	add	hl, sp
	ld	a, (hl)	; a = mode

	call	$001c	; CALSLT
#endasm
}

void write_VDP(unsigned char regno, unsigned char data)
{
	outp(VDP_writeport(VDP_WRITECONTROL), data);
	outp(VDP_writeport(VDP_WRITECONTROL), 0x80 | regno);
}

void write_vram_adr(unsigned char highadr, int lowadr)
{
	write_VDP(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
	outp(VDP_writeport(VDP_WRITECONTROL), (lowadr & 0xff));
	outp(VDP_writeport(VDP_WRITECONTROL), 0x40 | ((lowadr >> 8) & 0x3f));
}

void write_vram_data(unsigned char data)
{
	outp(VDP_writeport(VDP_WRITEDATA), data);
}

void read_vram_adr(unsigned char highadr, int lowadr)
{
	write_VDP(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
	outp(VDP_writeport(VDP_WRITECONTROL), (lowadr & 0xff));
	outp(VDP_writeport(VDP_WRITECONTROL), 0x00 | ((lowadr >> 8) & 0x3f));
}

unsigned char read_vram_data(void)
{
	return inp(VDP_readport(VDP_READDATA));
}

void set_screen5(void)
{
	DI();
	write_VDP(0, 0x06);
	write_VDP(1, 0x60);
	write_VDP(8, 0x08);
	write_VDP(9, 0x88);
	EI();
}

void set_displaypage(int page)
{
	DI();
	write_VDP(2, (page << 5) & 0x60 | 0x1f);
	EI();
}

void set_screen1(void)
{
	DI();
	write_VDP(0, 0x00);
	write_VDP(1, 0x60);
	write_VDP(8, 0x08);
	write_VDP(9, 0x08);
	EI();
}

void set_bgcolor(unsigned char color)
{
	DI();
	write_VDP(7, color & 0x0f);
	EI();
}

void set_spr_atr_adr(unsigned char highadr, int lowadr)
{
	DI();
	write_VDP(5, (lowadr >> (2 + 5)) & 0xf8 | 0x07);
	write_VDP(11, ((highadr << 1) & 0x02) | ((lowadr >> 15) & 0x01));
	EI();
}

void set_spr_pat_adr(unsigned char highadr, int lowadr)
{
	DI();
	write_VDP(6, ((highadr << 5) & 0x20) | ((lowadr >> 11) & 0x1f));
	EI();
}

void spr_on(void)
{
	DI();
	write_VDP(8, 0x02);
	EI();
}

void spr_off(void)
{
	DI();
	write_VDP(8, 0x00);
	EI();
}

unsigned char read_VDPstatus(int no)
{
	unsigned char data;
	DI();
	write_VDP(15, no);
	data = inp(VDP_readport(VDP_READSTATUS));
	write_VDP(15, 0);
	EI();
	return data;
}


void VDPsetbox(int dx, int dy, int nx, int ny, unsigned char dix, unsigned char diy, unsigned char data)
{
	unsigned char port = VDP_writeport(VDP_WRITEINDEX);
	write_VDP(17, 36);

	outp(port, dx & 0xff);			/* 36 */
	outp(port, (dx >> 8) & 0x01);	/* 37 */
	outp(port, dy & 0xff);			/* 38 */
	outp(port, (dy >> 8) & 0x03);	/* 39 */
	outp(port, nx & 0xff);			/* 40 */
	outp(port, (nx >> 8) & 0x01);	/* 41 */
	outp(port, ny & 0xff);			/* 42 */
	outp(port, (ny >> 8) & 0x03);	/* 43 */
	outp(port, data);				/* 44 */
	outp(port, ((diy << 3) & 0x80) | ((diy << 2) & 0x40));	/* 45 */
}

void VDPboxfill(void)
{
	outp(VDP_writeport(VDP_WRITEINDEX), 0xc0);	/* HMMV */
}

unsigned char *mainram = 0x4000; /* PAGE1 */

void main(void)
{
	int i;
	unsigned char status, data;
	unsigned char *pointer;

	DI();
	pointer = (unsigned char *)0x6;
	VDP_readadr = read_mainrom(0x0006);
	pointer = (unsigned char *)0x7;
	VDP_writeadr = read_mainrom(0x0007);
	EI();


/*	set_screen5();
	set_displaypage(0);*/
//	set_screenmode(5);

//	set_bgcolor(1);
//	set_spr_atr_adr(0, SPR_ATR_ADR); /* color table : atr-512 (0x7400) */
//	set_spr_pat_adr(0, SPR_PAT_ADR);

//	DI();
//	write_vram_adr(0, SPR_ATR_ADR);
//	for(i = 0; i < 32; ++i){
//		write_vram_data(255);		/* Y */
//		write_vram_data(0);		/* X */
//		write_vram_data(0);		/* No */
//		write_vram_data(0);		/* NoUse */
//	}
//	EI();

//	DI();
//	write_vram_adr(0, SPR_PAT_ADR);
//	for(i = 0; i < (8 * 4); ++i){
//		write_vram_data(0xff);
//	}
//	EI();

//	DI();
//	write_vram_adr(0, SPR_COL_ADR);
//	for(i = 0; i < 16; ++i){
//		write_vram_data(15 - i);
//	}
//	EI();

	read_VDPstatus(7);

	while((read_VDPstatus(2) & 0x01));

	DI();
	Set_RAM_MODE();

	read_vram_adr(0, 0x0000);
	for(i = 0; i < (256 / 2) * 128; i++){
		*mainram++ = read_vram_data();
	}

/*	mainram = (unsigned char *)0x4000;
	write_vram_adr(0, 0x0000 + (256 / 2) * 128 * 0);
	for(i = 0; i < (256 / 2) * 128; i++){
		write_vram_data(*mainram++);
	}*/

	Set_ROM_MODE();
end:
	EI();
	/*set_screen1();*/
//	set_screenmode(1);
}
