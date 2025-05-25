/* PC-88 PC-98 BSAVE to Screen CONV.(Digital 8 colors) for ZSDCC By m@3 */
/* .COM”Å */

#include <stdio.h>
#include <stdlib.h>

#define VRAM_ADR 0xc000

#define PARTS_SIZE 0x4000 //0x1e00 //0x2000

unsigned char mainram_data[PARTS_SIZE];

FILE *stream[2];

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

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
//	fread( buffer, 1, 4, stream[0]);
	fread( buffer, 1, size, stream[0]);
//	Set_ROM_MODE();
	fclose(stream[0]);
	return NOERROR;
}

void  trans_data(unsigned char *mainram, unsigned char *vram, unsigned char port)
{
__asm
	ld	hl, 2
	add	hl, sp
	ld	c, (hl)
	inc	hl
	ld	b, (hl)	; bc=mainram
	inc	hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)	; de = vram
	inc	hl
	ld	a,(hl)	; a= port

	ld	l,c
	ld	h,b	; hl = mainram

	ld	c,a
	xor	a

	DI
	out	(c),a
looppat:
	ldi

	cp	d
	jr	nz,looppat

	out	(#0x5f),a
	EI

__endasm;
}

unsigned char *vram_adr;
unsigned char i, j;

int	main(int argc,char **argv)
{
	char dst[100];

	Set_RAM_MODE();

	if (argc < 2){ //argv[1] == NULL){
		printf("PC-88 BRG file Loader.\n");
		return ERROR;
	}

	snprintf(dst, sizeof dst, "%s.grb", argv[1]);
	bload(dst, mainram_data, PARTS_SIZE);
	trans_data(mainram_data, (unsigned char *)VRAM_ADR, 0x5c);

	snprintf(dst, sizeof dst, "%s.grr", argv[1]);
	bload(dst, mainram_data, PARTS_SIZE);
	trans_data(mainram_data, (unsigned char *)VRAM_ADR, 0x5d);

	snprintf(dst, sizeof dst, "%s.grg", argv[1]);
	bload(dst, mainram_data, PARTS_SIZE);
	trans_data(mainram_data, (unsigned char *)VRAM_ADR, 0x5e);

	return NOERROR;
}

