/* PC-88 キャラMAP表示実験 By m@3 */
/* .COM版 */
/* MASKを出す */
/* 転送部インラインアセンブラ化 ZSDCC版 */

#include <stdio.h>
#include <stdlib.h>

#define VRAM_ADR 0xc000

#define PARTS_SIZE 0x1e00 / 3 //0x2000

unsigned char mainram_data[PARTS_SIZE];

FILE *stream[2];

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

#define SIZE 80

#define PARTS_X 2
#define PARTS_Y 8

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
	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
	ld	a,#0x3b
	out(#0x31),a
__endasm;
}

void Set_ROM_MODE(void){
__asm
;	ld	a,(#0xe6c2)
	ld	a,#0x39
	out(#0x31),a
	EI
__endasm;
}

short bload(char *loadfil, unsigned char *buffer, unsigned short size)
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

void  put_chr88_pat_mask(unsigned char *mainram, unsigned char *vram)
{
__asm
	ld	hl, 2
	add	hl, sp
	ld	c, (hl)
	inc	hl
	ld	b, (hl)	; bc=mainam
	inc	hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)	; de = vram
	ld	l,c
	ld	h,b	; hl = mainram

	ld	b,8

looppat:
	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	out	(#0x5c),a
	ld	a,(hl)
	ld	(de),a
;	inc	hl
	out	(#0x5d),a
	ld	a,(hl)
	ld	(de),a
;	inc	hl
	out	(#0x5e),a
	ld	a,(hl)
	ld	(de),a
	inc	hl

	inc	de
	out	(#0x5c),a
	ld	a,(hl)
	ld	(de),a
;	inc	hl
	out	(#0x5d),a
	ld	a,(hl)
	ld	(de),a
;	inc	hl
	out	(#0x5e),a
	ld	a,(hl)
	ld	(de),a

	out	(#0x5f),a

;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI

	push	de	;	hl = mainram
	ld	de, 31 ;((256 / 8 - 2) * 1 + 1)
	add	hl,de
	pop	de

	push	hl	;de = vram
	ex	de,hl
	ld	de,79 ;(80 - 1)
	add	hl,de
	ex	de,hl
	pop	hl

	djnz looppat

__endasm;
}

unsigned char *vram_adr;
unsigned char i, j;


int	main(int argc,char **argv)
{
	unsigned short pat_tmp;

	if (argc < 2){ //argv[1] == NULL){
		printf("PC-88 .grp MASK file Loader.\n");
		return ERROR;
	}

	Set_RAM_MODE();

	if(bload(argv[1], mainram_data, PARTS_SIZE))
		return ERROR;

	for(j = 0; j < 10; ++j){
		for(i = 0; i < 16; ++i){
			vram_adr = (unsigned char *)(VRAM_ADR + (i * PARTS_X + j * PARTS_Y * SIZE));
			pat_tmp = i * 2 + j * 16 * 16;
			put_chr88_pat_mask(&mainram_data[pat_tmp], vram_adr);
		}
	}

	return NOERROR;
}

