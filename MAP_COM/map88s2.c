/* PC-88 キャラMAP移動実験 ALU部分的に使う 2x2圧縮展開 By m@3 */
/* .COM版 */
/* キャラを出す */
/* 転送部インラインアセンブラ化 ZSDCC版 V1モード対応 */

#include <stdio.h>
#include <stdlib.h>

#define VRAM_ADR 0xc000
//#define MAINRAM_ADR 0x2000
//#define MAP_ADR 0x4000
//#define VRAM_DATA_ADR 0xc700
//#define PARTS_DATA (MAP_ADR+0x3c00)
//#define MASK_DATA_ADR 0x1600
#define VRAM_TMP_ADR 0xfe80

#define MASK_SIZE 0xa00
#define CHR_SIZE 0x900 //0x1e00
#define PARTS_SIZE 0x1e00 //0x2000

unsigned char mask_data[MASK_SIZE];
//#define MASK_DATA_ADR mask_data
unsigned char mainram_data[PARTS_SIZE];
//#define MAINRAM_ADR mainram_data
unsigned char vram_data[CHR_SIZE];
//#define VRAM_DATA_ADR vram_data

#define PARTS_HEAD 0x3c00 /*組み合わせキャラデータの先頭番地*/
#define BUFFSIZE 16384
unsigned char mapdata[BUFFSIZE];

FILE *stream[2];

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

#define SIZE 80

#define X_SIZE 18
#define Y_SIZE 18

#define PARTS_X 2
#define PARTS_Y 8

#define MAP_SIZE_X 128
#define MAP_SIZE_Y 128

#define OFS_X 2
#define OFS_Y 2

#define CHR_X 8
#define CHR_Y 8

unsigned char tmp_vram[PARTS_X * PARTS_Y * 3];
unsigned char map_data[(X_SIZE+2) * 32];
unsigned char basic_mode = 0;
unsigned char *n88rom;
unsigned char rom_mode = 0;

#define MAXCOLOR 8

unsigned char org_pal[MAXCOLOR][3] =
	{{ 0, 0, 0 },
	{ 0, 0, 15 },
	{ 15, 0, 0 },
	{ 15, 0, 15 },
	{ 0, 15, 0 },
	{ 0, 15, 15 },
	{ 15, 15, 0 },
	{ 15, 15, 15 },};

unsigned char romport;

void DI(void){
__asm
	DI
__endasm;
}

void EI(void){
__asm
	EI
__endasm;
}

void Set_RAM_MODE(void){
__asm
;	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
	ld	a,#0x3b
	out(#0x31),a
	EI
__endasm;
}

void Set_ROM_MODE(void){
__asm
	DI
;	ld	a,(#0xe6c2)
	ld	a,#0x39
	out(#0x31),a
;	EI
__endasm;
}/*
void set_mainROM(void)
{
	romport = inp(0x71);
	outp(0x71, 0xff);
}

void reset_mainROM(void)
{
	outp(0x71, romport);
}*/

/*マップデータをファイルから配列に読み込む*/
short mapload(char *loadfil) __sdcccall(1)
{
//	short inhandle;

//	if ((inhandle = open( loadfil, O_RDONLY )) == -1) { //| O_BINARY
	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
//	printf("Load file %s. Address %x Size %x End %x\n", loadfil, mapdata, BUFFSIZE, mapdata+BUFFSIZE);
//	read( inhandle, mapdata, 7);
	fread( mapdata, 1, 7, stream[0]);
//	read( inhandle, mapdata, BUFFSIZE);
	fread( mapdata, 1, BUFFSIZE, stream[0]);
//	close(inhandle);
	fclose(stream[0]);
	return NOERROR;
}

short bload(char *loadfil, unsigned char *buffer, unsigned short size) __sdcccall(1)
{
	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
//	printf("Load file %s. Address %x Size %x End %x\n", loadfil, buffer, size, buffer+size);
//	Set_RAM_MODE();
	fread( buffer, 1, 4, stream[0]);
	fread( buffer, 1, size, stream[0]);
//	Set_ROM_MODE();
	fclose(stream[0]);
	return NOERROR;
}

void  set_mapchr(unsigned char *srcram, unsigned char *dstram) __sdcccall(1)
{
__asm
;	ld	hl, 2
;	add	hl, sp
;	ld	c, (hl)
;	inc	hl
;	ld	b, (hl)	; bc=srcram
;	inc	hl
;	ld	e,(hl)
;	inc	hl
;	ld	d,(hl)	; de = dstram
;	ld	l,c
;	ld	h,b	; hl = srcram


	ld	b,8
	push	hl
looppat11:
;	push	bc

	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	ld	a,(hl)	;b1
	ld	(de),a
	inc	hl
	inc	de
	inc	de
	ld	a,(hl)	;r1
	ld	(de),a
	inc	hl
	inc	de
	inc	de
	ld	a,(hl)	;g1
	ld	(de),a
	inc	hl
	dec	de
	dec	de
	dec	de

	ld	a,(hl)	;b2
	ld	(de),a
	inc	hl
	inc	de
	inc	de
	ld	a,(hl)	;r2
	ld	(de),a
	inc	hl
	inc	de
	inc	de
	ld	a,(hl)	;g2
	ld	(de),a
	inc	de

;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI

	push	de	;	hl = mainram
	ld	de, 91 ;((256 / 8 - 2) * 3 + 1)
	add	hl,de
	pop	de

;	pop	bc
	djnz looppat11
	pop	hl

__endasm;
}

void  put_chr88_pat(unsigned char *vram, unsigned char *mainram) __sdcccall(1)
{
__asm
;	ld	hl, 2
;	add	hl, sp
;	ld	e, (hl)
;	inc	hl
;	ld	d, (hl)	; bc = vram
;	inc	hl
;	ld	c,(hl)
;	inc	hl
;	ld	b,(hl)	; de = mainram
;	ld	l,c
;	ld	h,b	; hl = vram

;	ex	de,hl

; Init draw address
	ld	bc, 80 ;SCRWidth
	ld	(_put00 + 1), hl
	ld	(_put01 + 1), hl
	ld	(_put02 + 1), hl
	add	hl, bc

	ld	(_put03 + 1), hl
	ld	(_put04 + 1), hl
	ld	(_put05 + 1), hl
	add	hl, bc

	ld	(_put06 + 1), hl
	ld	(_put07 + 1), hl
	ld	(_put08 + 1), hl
	add	hl, bc

	ld	(_put09 + 1), hl
	ld	(_put0A + 1), hl
	ld	(_put0B + 1), hl
	add	hl, bc

	ld	(_put10 + 1), hl
	ld	(_put11 + 1), hl
	ld	(_put12 + 1), hl
	add	hl, bc

	ld	(_put13 + 1), hl
	ld	(_put14 + 1), hl
	ld	(_put15 + 1), hl
	add	hl, bc

	ld	(_put16 + 1), hl
	ld	(_put17 + 1), hl
	ld	(_put18 + 1), hl
	add	hl, bc

	ld	(_put19 + 1), hl
	ld	(_put1A + 1), hl
	ld	(_put1B + 1), hl

	; draw
	ex	de, hl
	ld	c,l
	ld	b,h

	ld	(_stack + 1), sp
	di
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	ld	sp, hl

	pop	hl
	out	(#0x5c),a
_put00:	ld	(#0x0000), hl	;1
	pop	hl
	out	(#0x5d),a
_put01:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put02:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put03:	ld	(#0x0000), hl	;2
	pop	hl
	out	(#0x5d),a
_put04:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put05:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put06:	ld	(#0x0000), hl	;3
	pop	hl
	out	(#0x5d),a
_put07:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put08:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put09:	ld	(#0x0000), hl	;4
	pop	hl
	out	(#0x5d),a
_put0A:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put0B:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put10:	ld	(#0x0000), hl	;5
	pop	hl
	out	(#0x5d),a
_put11:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put12:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put13:	ld	(#0x0000), hl	;6
	pop	hl
	out	(#0x5d),a
_put14:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put15:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put16:	ld	(#0x0000), hl	;7
	pop	hl
	out	(#0x5d),a
_put17:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put18:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put19:	ld	(#0x0000), hl	;8
	pop	hl
	out	(#0x5d),a
_put1A:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put1B:	ld	(#0x0000), hl

_stack:	ld	sp, #0x0000

__endasm;

__asm
	out	(#0x5f),a

;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI
__endasm;
}

/*
void  put_chr88_pat(unsigned char *mainram, unsigned char *vram) __sdcccall(1)
{
__asm
;	ld	hl, 2
;	add	hl, sp
;	ld	c, (hl)
;	inc	hl
;	ld	b, (hl)	; bc=mainam
;	inc	hl
;	ld	e,(hl)
;	inc	hl
;	ld	d,(hl)	; de = vram
;	ld	l,c
;	ld	h,b	; hl = mainram

	ld	b,8

looppat:
	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	out	(#0x5c),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	hl
	out	(#0x5d),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	hl
	out	(#0x5e),a
	ld	a,(hl)
	ld	(de),a

	dec	hl
	dec	hl
	dec	hl

	inc	de
	out	(#0x5c),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	hl
	out	(#0x5d),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	hl
	out	(#0x5e),a
	ld	a,(hl)
	ld	(de),a
	inc	hl

	out	(#0x5f),a

;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI

;	push	de	;	hl = mainram
;	ld	de, 91 ;((256 / 8 - 2) * 3 + 1)
;	add	hl,de
;	pop	de

	push	hl	;de = vram
	ex	de,hl
	ld	de,79 ;(80 - 1)
	add	hl,de
	ex	de,hl
	pop	hl

	djnz looppat

__endasm;
}
*/

void  put_chr88_pat2(unsigned char *vram, unsigned char *mainvram) __sdcccall(1)
{
__asm
;	ld	hl, 2
;	add	hl, sp
;	ld	e, (hl)
;	inc	hl
;	ld	d, (hl)	; bc = vram
;	inc	hl
;	ld	c,(hl)
;	inc	hl
;	ld	b,(hl)	; de = mainram
;	ld	l,c
;	ld	h,b	; hl = vram

;	ex	de,hl

; Init draw address
	ld	bc, 2 ;SCRWidth
	ld	(_put100 + 1), hl
	add	hl, bc
	ld	(_put101 + 1), hl
	add	hl, bc
	ld	(_put102 + 1), hl
	add	hl, bc

	ld	(_put103 + 1), hl
	add	hl, bc
	ld	(_put104 + 1), hl
	add	hl, bc
	ld	(_put105 + 1), hl
	add	hl, bc

	ld	(_put106 + 1), hl
	add	hl, bc
	ld	(_put107 + 1), hl
	add	hl, bc
	ld	(_put108 + 1), hl
	add	hl, bc

	ld	(_put109 + 1), hl
	add	hl, bc
	ld	(_put10A + 1), hl
	add	hl, bc
	ld	(_put10B + 1), hl
	add	hl, bc

	ld	(_put110 + 1), hl
	add	hl, bc
	ld	(_put111 + 1), hl
	add	hl, bc
	ld	(_put112 + 1), hl
	add	hl, bc

	ld	(_put113 + 1), hl
	add	hl, bc
	ld	(_put114 + 1), hl
	add	hl, bc
	ld	(_put115 + 1), hl
	add	hl, bc

	ld	(_put116 + 1), hl
	add	hl, bc
	ld	(_put117 + 1), hl
	add	hl, bc
	ld	(_put118 + 1), hl
	add	hl, bc

	ld	(_put119 + 1), hl
	add	hl, bc
	ld	(_put11A + 1), hl
	add	hl, bc
	ld	(_put11B + 1), hl

	; draw
	ex	de, hl
	ld	c,l
	ld	b,h

	ld	(_stack2 + 1), sp
	di
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	ld	sp, hl

	pop	hl
_put100:	ld	(#0x0000), hl	;1
	pop	hl
_put101:	ld	(#0x0000), hl
	pop	hl
_put102:	ld	(#0x0000), hl

	pop	hl
_put103:	ld	(#0x0000), hl	;2
	pop	hl
_put104:	ld	(#0x0000), hl
	pop	hl
_put105:	ld	(#0x0000), hl

	pop	hl
_put106:	ld	(#0x0000), hl	;3
	pop	hl
_put107:	ld	(#0x0000), hl
	pop	hl
_put108:	ld	(#0x0000), hl

	pop	hl
_put109:	ld	(#0x0000), hl	;4
	pop	hl
_put10A:	ld	(#0x0000), hl
	pop	hl
_put10B:	ld	(#0x0000), hl

	pop	hl
_put110:	ld	(#0x0000), hl	;5
	pop	hl
_put111:	ld	(#0x0000), hl
	pop	hl
_put112:	ld	(#0x0000), hl

	pop	hl
_put113:	ld	(#0x0000), hl	;6
	pop	hl
_put114:	ld	(#0x0000), hl
	pop	hl
_put115:	ld	(#0x0000), hl

	pop	hl
_put116:	ld	(#0x0000), hl	;7
	pop	hl
_put117:	ld	(#0x0000), hl
	pop	hl
_put118:	ld	(#0x0000), hl

	pop	hl
_put119:	ld	(#0x0000), hl	;8
	pop	hl
_put11A:	ld	(#0x0000), hl
	pop	hl
_put11B:	ld	(#0x0000), hl

_stack2:	ld	sp, #0x0000

__endasm;

__asm
	out	(#0x5f),a

;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI
__endasm;
}
/*
void  put_chr88_pat2(unsigned char *mainram, unsigned char *vram_tmp) __sdcccall(1)
{
__asm
;	ld	hl, 2
;	add	hl, sp
;	ld	c, (hl)
;	inc	hl
;	ld	b, (hl)	; bc=mainram
;	inc	hl
;	ld	e,(hl)
;	inc	hl
;	ld	d,(hl)	; de = vram_tmp
;	ld	l,c
;	ld	h,b	; hl = mainram

	ld	b,8

looppat2:
	push	bc

	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	ldi			;b1
	inc	hl

	ldi			;r1
	inc	hl

	ldi			;g1
	ld	bc,-4
	add	hl,bc

	ldi			;b2
	inc	hl

	ldi			;r2
	inc	hl

	ldi			;g2
;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI

;	push	de	;	hl = mainram
;	ld	de, 90 ;((256 / 8 - 2) * 3 + 0)
;	add	hl,de
;	pop	de

	pop	bc
	djnz looppat2

__endasm;
}
*/
void  put_chr_or(unsigned char *mainram, unsigned char *vram_tmp) __sdcccall(1)
{
__asm
;	ld	hl, 2
;	add	hl, sp
;	ld	c, (hl)
;	inc	hl
;	ld	b, (hl)	; bc = mainram
;	inc	hl
;	ld	e,(hl)
;	inc	hl
;	ld	d,(hl)	; de = vram_tmp
;	ld	l,c
;	ld	h,b	; hl = mainram

	ld	b,8

loopor:
	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	ld	a,(de)		;b1
	or	(hl)
	ld	(de),a
	inc	hl
	inc	de

	ld	a,(de)		;b2
	or	(hl)
	ld	(de),a
	inc	hl
	inc	de

	ld	a,(de)		;r1
	or	(HL)
	ld	(de),a
	inc	hl
	inc	de

	ld	a,(de);		;r2
	or	(hl)
	ld	(de),a
	inc	hl
	inc	de

	ld	a,(de)		;g1
	or	(hl)
	ld	(de),a
	inc	hl
	inc	de

	ld	a,(de)		;g2
	or	(HL)
	ld	(de),a
	inc	hl
	inc	de

;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI

;	push	de	;	hl = mainram
;	ld	de, 91 ;((256 / 8 - 2) * 3 + 1)
;	add	hl,de
;	pop	de

	djnz loopor

__endasm;
}

void mask(unsigned char *mainram, unsigned char *vram_tmp) __sdcccall(1)
{
__asm
;	ld	hl, 2
;	add	hl, sp
;	ld	c, (hl)
;	inc	hl
;	ld	b, (hl)	; bc = mainram
;	inc	hl
;	ld	e,(hl)
;	inc	hl
;	ld	d,(hl)	; de = vram_tmp

;	ld	l,c
;	ld	h,b	; hl = mainram


	ld	b,8
looppat3:
	push	bc

	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	ld	a,(hl)
	cpl
	ld	c,a
	inc	hl

	ld	a,(hl)
	cpl
	ld	b,a

	ld	a,(de)	;b1
	and	c
	ld	(de),a
	inc	de

	ld	a,(de)	;b2
	and	b
	ld	(de),a
	inc	de

	ld	a,(de)	;r1
	and	c
	ld	(de),a
	inc	de

	ld	a,(de)	;r2
	and	b
	ld	(de),a
	inc	de

	ld	a,(de)	;g1
	and	c
	ld	(de),a
	inc	de

	ld	a,(de)	;g2
	and	b
	ld	(de),a
	inc	de

;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI

;	push	de	;	hl = mainram
	ld	bc, 31 ;((256 / 8 - 2) * 1 + 1)
	add	hl,bc
;	pop	de

	pop	bc
	djnz looppat3

__endasm;
}

/*
void copy(unsigned char *mainram, unsigned char *vram) __sdcccall(1)
{
__asm
	DI

;	ld	hl, 2
;	add	hl, sp
;	ld	c, (hl)
;	inc	hl
;	ld	b, (hl)	; bc = mainram
;	inc	hl
;	ld	e,(hl)
;	inc	hl
;	ld	d,(hl)	; de = vram

;	ld	l,c
;	ld	h,b	; hl = mainram

	ld	b,8

looppat4:
	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	out	(#0x5c),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	out	(#0x5d),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	out	(#0x5e),a
	ld	a,(hl)
	ld	(de),a
	inc	hl

	inc	de
	out	(#0x5c),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	out	(#0x5d),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	out	(#0x5e),a
	ld	a,(hl)
	ld	(de),a

	out	(#0x5f),a

;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI

	inc	hl

	push	hl	;de = vram
	ex	de,hl
	ld	de,79 ;(80 - 1)
	add	hl,de
	ex	de,hl
	pop	hl

	djnz looppat4

__endasm;
}
*/
void  set_chr88_pat(unsigned char *mainram, unsigned char *vram) __sdcccall(1)
{
__asm
;	ld	hl, 2
;	add	hl, sp
;	ld	c, (hl)
;	inc	hl
;	ld	b, (hl)	; bc=mainam
;	inc	hl
;	ld	e,(hl)
;	inc	hl
;	ld	d,(hl)	; de = vram
;	ld	l,c
;	ld	h,b	; hl = mainram

	ld	b,8

looppat5:
	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	out	(#0x5c),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	hl
	out	(#0x5d),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	hl
	out	(#0x5e),a
	ld	a,(hl)
	ld	(de),a
	dec	hl
	dec	hl
	dec	hl
	inc	de

	out	(#0x5c),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	hl
	out	(#0x5d),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	hl
	out	(#0x5e),a
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	de

	out	(#0x5f),a

;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI

;	push	de	;	hl = mainram
;	ld	de, 91 ;((256 / 8 - 2) * 3 + 1)
;	add	hl,de
;	pop	de

	djnz looppat5

__endasm;
}

void copy_ALU(unsigned char *srcvram, unsigned char *dstvram) __sdcccall(1)
{
__asm
;	ld	hl, 2
;	add	hl, sp
;	ld	c, (hl)
;	inc	hl
;	ld	b, (hl)	; bc = srcvram
;	inc	hl
;	ld	e,(hl)
;	inc	hl
;	ld	d,(hl)	; de = dstvram

;	ld	l,c
;	ld	h,b	; hl = srcvram

	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	in	a,(#0x32)
	or	#0x40
	out	(#0x32),a	; USE ALU
	ld	a,#0x90
	out	(#0x35),a	; Access GVRAM

	ldi			;1
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;2
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;3
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;4
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;5
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;6
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;7
	ldi
	ex	de,hl
	ld	bc,80-2
	add	hl,bc
	ex	de,hl

	ldi			;8
	ld	a,(hl)
	ld	(de),a

	xor a
	out(#0x35),a	; Access MAINRAM
	in	a,(#0x32);
	and #0xbf;
	out(#0x32),a	; NOUSE ALU

;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI
__endasm;
}


unsigned char no;
unsigned char *vram_adr;
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
volatile unsigned char i, j;

//unsigned char *vram_tmp_adr;

void chr_sub(void)
{
	unsigned short pat_tmp, pat_tmp2;
	unsigned char no2 = chr_tbl[dir * 2 + dir2][(i - CHR_X) + (j - CHR_Y) * 2];

//	vram_tmp_adr = (unsigned char *)VRAM_TMP_ADR;
	pat_tmp2 = (no2 & 0x0f) * 2 + (no2 & 0xf0) * 16;
	pat_tmp = (no & 0x0f) * 2 + (no & 0xf0) * 16;

//	put_chr88_pat2(&mainram_data[pat_tmp * 2 + pat_tmp], tmp_vram);
	put_chr88_pat2(tmp_vram, &mainram_data[no * PARTS_X * PARTS_Y * 3]);
/* マスクを書く */
	mask(&mask_data[pat_tmp2], tmp_vram);

//	put_chr_or(&vram_data[pat_tmp2 * 2 + pat_tmp2], tmp_vram);
	put_chr_or(&vram_data[no2 * PARTS_X * PARTS_Y * 3], tmp_vram);

//	copy(tmp_vram,vram_adr);
	put_chr88_pat(vram_adr, tmp_vram);
}

void pat_sub(void)
{
	unsigned short pat_tmp;
	if((basic_mode) || (no >= 24)){
		pat_tmp = (no & 0x0f) * 2 + (no & 0xf0) * 16;
//		put_chr88_pat(&mainram_data[pat_tmp * 2 + pat_tmp], vram_adr);
		put_chr88_pat(vram_adr, &mainram_data[no * PARTS_X * PARTS_Y * 3]);
	}else{
		copy_ALU((unsigned char *)(VRAM_TMP_ADR + (no * PARTS_X * PARTS_Y)), vram_adr);
	}
}

/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	outp(0x54 + color, red * 8 | blue);
	outp(0x54 + color, green | 0x40);
}

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned short i;
	for(i = 0; i < MAXCOLOR; i++)
		pal_set(pal_no, i, color[i][0]/2, color[i][1]/2, color[i][2]/2);
}

void wait_vsync(void)
{
	while((inp(0x40) & 0x20)); /* WAIT VSYNC */
	while(!(inp(0x40) & 0x20));
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
	unsigned char j, k;
	unsigned char pal[3];

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
		if(j == (MAXCOLOR / 2))
			wait_vsync();
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
	unsigned char j;
	for(j = 0; j < MAXCOLOR; j++)
		pal_set(pal_no, j, 0, 0, 0);
}

unsigned char k0, k1, k8, ka, st, pd, k5, k9, *data, *data_tmp;
unsigned char pat_no, *pat_adr;
unsigned char x = 165, y = 30,xx, yy, old_x = 255, old_y = 255, k;
//unsigned char ii, jj;
unsigned char *vram_adr_tmp;

unsigned char old_map_data[(X_SIZE + 2) * 32];
unsigned char sub_flag;
unsigned char *map_adr;
unsigned char *old_map_adr;

unsigned char fadeflag = 0;

unsigned char *sysport = (unsigned char *)0xe6c2;

void map_sub(void)
{
/*			for(j = 0; j < Y_SIZE / 2; ++j){

				for(i = 0; i < X_SIZE / 2; ++i){
//					Set_RAM_MODE();
					pat_no = ((*data >> 4) & 0x0f) | ((*data << 4) & 0xf0);
//					pat_adr = (unsigned char *)(PARTS_DATA + pat_no * 4);
					pat_adr = &mapdata[PARTS_HEAD + pat_no * 4];
//					ii = i * 2 + xx;
//					jj = j * 2 + yy;
					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr++);
					map_adr += (32 - 1);
					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr);
					map_adr -= (32 - 1);
*/
/*					map_data[ii][jj] = *(pat_adr++);
					map_data[ii+1][jj] = *(pat_adr++);
					map_data[ii][jj+1] = *(pat_adr++);
					map_data[ii+1][jj+1] = *(pat_adr);*/
//					++data;
//					Set_ROM_MODE();
/*				}
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
	x = 165;
	y = 30;
	old_x = 255;
	old_y = 255;

//	printf("sys=%x mode=%x\n",*sysport, inp(0x32));

	Set_RAM_MODE();

/*マップのロード to (unsigned char mapdata[])*/

	if(bload("elmstc.grp", mapdata, PARTS_SIZE))
		return ERROR;
	for(i = 0; i < 128; ++i){
		unsigned short pat_tmp = (i & 0x0f) * 2 + (i & 0xf0) * 16;
		set_mapchr(&mapdata[pat_tmp * 3], &mainram_data[i * PARTS_X * PARTS_Y * 3]);
	}
	if(bload("elmstm.grp", mapdata, CHR_SIZE))
		return ERROR;
	for(i = 0; i < 8*4; ++i){
		unsigned short pat_tmp = (i & 0x0f) * 2 + (i & 0xf0) * 16;
		set_mapchr(&mapdata[pat_tmp * 3], &vram_data[i * PARTS_X * PARTS_Y * 3]);
	}

	if(mapload("ELMSTMAP.MDT")){
		return ERROR;
	}
	if(bload("elmask.grp", mask_data, MASK_SIZE))
		return ERROR;
//	if(bload("elmstm.grp", vram_data, CHR_SIZE))
//		return ERROR;
//	if(bload("elmstc.grp", mainram_data, PARTS_SIZE))
//		return ERROR;

//	outp(0x32, inp(0x32) | 0x20);	/* 512 colors */
	basic_mode = inp(0x31) & 0x80;
//	n88rom = (unsigned char *)0x79d7;
//	Set_ROM_MODE();
//	rom_mode = *n88rom;
//	Set_RAM_MODE();

	if(basic_mode)
		outp(0x51, 0);

	if(!basic_mode){
		pal_allblack(0);
		for(i = 0; i < 24; ++i){
			unsigned short pat_tmp;
			pat_tmp = (i & 0x0f) * 2 + (i & 0xf0) * 16;
//			set_chr88_pat(&mainram_data[pat_tmp * 2 + pat_tmp], (unsigned char *)(VRAM_TMP_ADR + (i * PARTS_X * PARTS_Y)));
			set_chr88_pat(&mainram_data[i * PARTS_X * PARTS_Y * 3], (unsigned char *)(VRAM_TMP_ADR + (i * PARTS_X * PARTS_Y)));
		}
	}

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}

	st = pd = 255;
	while(((k9 = inp(0x09)) & 0x80)){ /* ESC */
		k0 = inp(0x00);
		k1 = inp(0x01);
		k8 = inp(0x08);
		ka = inp(0x0a);
//		if(!(rom_mode <= 0x33)){
		if(!basic_mode){
			DI();
			outp(0x44, 0x0e);
			st = inp(0x45);
			outp(0x44, 0x0f);
			pd = inp(0x45);
			EI();
		}
		k5 = inp(0x05);
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
//			data = (unsigned char *)(MAP_ADR + (x / 2) + (y / 2) * MAP_SIZE_X);
			data = &mapdata[x / 2 + (y / 2) * MAP_SIZE_X];
			data_tmp = data;
			vram_adr = (unsigned char *)(VRAM_ADR + (OFS_X * PARTS_X + SIZE * PARTS_Y * OFS_Y));
			vram_adr_tmp = vram_adr;
			map_adr = &map_data[xx + yy * 32];

			map_sub();

			dir2 = 1 - dir2;
			map_adr = &map_data[1 + 1 * 32];
			old_map_adr = &old_map_data[1 + 1 * 32];

/*			if((old_x != x)){
				for(i = 1; i < (X_SIZE - 1); ++i){
					for(j = 1; j < (Y_SIZE - 1); ++j){*/
/*						no = *map_adr;
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
						}else if(no != *old_map_adr){
							pat_sub();
							*old_map_adr = no;
						}*/
__asm
	ld	hl,_old_x
	ld	a,(_x)
	cp	a,(hl)
	jp	z,skip00

	ld	hl,(_map_adr)
	ld	de,(_old_map_adr)

	ld	c,1
skip01:
	ld	a,c
	ld	(_i),a
	ld	b,1
skip02:
	ld	a,b
	ld	(_j),a
	push	bc
;	ld	hl,(_map_adr)
	ld	a,(hl)
	ld	(_no),a
	ld	a,c ;(_i)
	cp	a,CHR_X	; i - CHR_X >= 0
	jr	c,skip1
	cp	a,CHR_X+2	; i - (CHR_X+1) <= 0
;	jr	z,skip0_1
	jr	nc,skip1
skip0_1:
	ld	a,b ;(_j)
	cp	a,CHR_Y	; j - CHR_Y >= 0
	jr	c,skip1
	cp	a,CHR_Y+2	; j - (CHR_Y+1) <= 0
;	jr	z,skip0_2
	jr	nc,skip1
skip0_2:
;	push	bc
	push	hl
	push	de
	call	_chr_sub
	pop	de
	pop	hl
;	pop	bc
	jr	skip2
skip1:
;	ld	a,(hl)
;	ld	de,(_old_map_adr)
	ld	a,(de)
	cp	a,(hl)
	jr	z,skip2
	ld	a,(hl)
	ld	(de),a
;	push	bc
	push	hl
	push	de
	call	_pat_sub
	pop	de
	pop	hl
;	pop	bc
skip2:
	push	hl
	ld	hl,(_vram_adr)
	ld	bc,SIZE * PARTS_Y
	add	hl,bc
	ld	(_vram_adr),hl
	pop	hl

	ld	bc,32
;	ld	hl,(_map_adr)
	add	hl,bc
;	ld	(_map_adr),hl
	ex	de,hl
;	ld	hl,(_old_map_adr)
	add	hl,bc
;	ld	(_old_map_adr),hl
	ex	de,hl

	pop	bc
;	ld	hl,_j
;	inc	(hl)
;	ld	a,(hl)
	inc	b
	ld	a,b
	cp	a,Y_SIZE-1
	jr	nz,skip02

	push	bc
	push	hl
	ld	bc,PARTS_X
	ld	hl,(_vram_adr_tmp)
	add	hl,bc
	ld	(_vram_adr_tmp),hl
	ld	(_vram_adr),hl
	pop	hl

	ld	bc,1 - 32 * (X_SIZE - 2)
;	ld	hl,(_map_adr)
	add	hl,bc
;	ld	(_map_adr),hl
	ex	de,hl
;	ld	hl,(_old_map_adr)
	add	hl,bc
;	ld	(_old_map_adr),hl
	ex	de,hl
	pop	bc

;	ld	hl,_i
;	inc	(hl)
;	ld	a,(hl)
	inc	c
	ld	a,c
	cp	a,X_SIZE-1
	jp	nz,skip01
	jp	skip05
__endasm;
/*						vram_adr += SIZE * PARTS_Y;
						map_adr += 32;
						old_map_adr += 32;
					}
					vram_adr_tmp += PARTS_X;
					vram_adr = vram_adr_tmp;
					map_adr += (1 - 32 * (X_SIZE - 2));
					old_map_adr += (1 - 32 * (X_SIZE - 2));
				}
			}
			else
			{
				for(j = 1; j < (Y_SIZE - 1); ++j){
					for(i = 1; i < (X_SIZE - 1); ++i){*/
/*						no = *map_adr;
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
						}else if(no != *old_map_adr){
							pat_sub();
							*old_map_adr = no;
						}*/
__asm
skip00:
	ld	hl,(_map_adr)
	ld	de,(_old_map_adr)
	ld	c,1
skip03:
	ld	a,c
	ld	(_j),a
	ld	b,1
skip04:
	ld	a,b
	ld	(_i),a
	push	bc
;	ld	hl,(_map_adr)
	ld	a,(hl)
	ld	(_no),a
	ld	a,b ;(_i)
	cp	a,CHR_X	; i - CHR_X >= 0
	jr	c,skip4
	cp	a,CHR_X+2	; i - (CHR_X+1) <= 0
;	jr	z,skip3_1
	jr	nc,skip4
skip3_1:
	ld	a,c; (_j)
	cp	a,CHR_Y	; j - CHR_Y >= 0
	jr	c,skip4
	cp	a,CHR_Y+2	; j - (CHR_Y+1) <= 0
;	jr	z,skip3_2
	jr	nc,skip4
skip3_2:
;	push	bc
	push	hl
	push	de
	call	_chr_sub
	pop	de
	pop	hl
;	pop	bc
	jr	skip5
skip4:
;	ld	de,(_old_map_adr)
	ld	a,(de)
	cp	a,(hl)
	jr	z,skip5
	ld	a,(hl)
	ld	(de),a
;	push	bc
	push	hl
	push	de
	call	_pat_sub
	pop	de
	pop	hl
;	pop	bc
skip5:
	push	hl
	ld	hl,(_vram_adr)
	ld	bc,PARTS_X
	add	hl,bc
	ld	(_vram_adr),hl
	pop	hl
;	ld	hl,(_map_adr)
	inc	hl
;	ld	(_map_adr),hl
;	ld	de,(_old_map_adr)
	inc	de
;	ld	(_old_map_adr),de

	pop	bc
;	ld	hl,_i
;	inc	(hl)
;	ld	a,(hl)
	inc	b
	ld	a,b
	cp	a,X_SIZE-1
	jr	nz,skip04

	push	bc
	push	hl
	ld	hl,(_vram_adr)
	ld	bc,SIZE * PARTS_Y - (PARTS_X) * (X_SIZE - 2)
	add	hl,bc
	ld	(_vram_adr),hl
	pop	hl
	ld	bc,32 - (X_SIZE - 2)
;	ld	hl,(_map_adr)
	add	hl,bc
;	ld	(_map_adr),hl
	ex	de,hl
;	ld	de,(_old_map_adr)
	add	hl,bc
;	ld	(_old_map_adr),de
	ex	de,hl
	pop	bc

;	ld	hl,_j
;	inc	(hl)
;	ld	a,(hl)
	inc	c
	ld	a,c
	cp	a,Y_SIZE-1
	jp	nz,skip03
skip05:
__endasm;

/*						vram_adr += PARTS_X;
						++map_adr;
						++old_map_adr;
					}
					vram_adr += (SIZE * PARTS_Y - (PARTS_X) * (X_SIZE - 2));
					map_adr += (32 - (X_SIZE - 2));
					old_map_adr += (32 - (X_SIZE - 2));
				}
			}*/
		}
		old_x = x;
		old_y = y;

		if(fadeflag == 0){
			fadeflag = 1;
			if(!basic_mode)
				fadeinblack(org_pal, 0, 3);
		}
	}
	if(!basic_mode){
		fadeoutblack(org_pal, 0, 3);
		pal_all(0, org_pal);
	}
	if(basic_mode){
		outp(0x51, 0x20);
	}
	return NOERROR;
}

