/* X1 PC-98 BSAVE to Screen CONV.(Digital 8 colors) for ZSDCC By m@3 */
/* .COM”Å */

#include <stdio.h>
#include <stdlib.h>

#define PARTS_SIZE 0x4000 //0x1E00 //0xf00

unsigned short y_table[200];

unsigned char mainram_data[PARTS_SIZE];

FILE *stream[2];

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

#define SIZE 80
#define LINE 200

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


short bload(char *loadfil, unsigned char *buffer, unsigned short size)
{
	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
//	printf("Load file %s. Address %x Size %x End %x\n", loadfil, buffer, size, buffer+size);
//	fread( buffer, 1, 4, stream[0]);
	fread( buffer, 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}

//#define VRAM_MACRO(X,Y) (X + (Y / 8) * 80 + (Y & 7) * 0x800)
#define VRAM_MACRO(X,Y) (X + y_table[Y])

unsigned short adr_tmp;
unsigned char *mainram_tmp;

void trans_data(unsigned char *mainram_adr, unsigned short vram_adr)
{
	unsigned char ii,jj;
	mainram_tmp = mainram_adr;
	for(jj = 0 ; jj < LINE; ++jj){
		adr_tmp = VRAM_MACRO(0, jj) + vram_adr;
//		for(ii = 0 ; ii < SIZE; ++ii){
//			outp(adr_tmp++, *(mainram_adr++));

__asm
	exx
	ld	bc,(_adr_tmp)
	ld	hl,(_mainram_tmp)
	ld	d,SIZE
loop:
	ld	a,(hl)
	out	(c),a
	inc	bc
	inc	hl
	dec	d
	jr	nz,loop
	ld	(_mainram_tmp),hl
	exx
__endasm;
//		}
	}
}

int	main(int argc,char **argv)
{
	char dst[100];
	unsigned char i;

	for(i = 0; i < 200; ++i){
		y_table[i] = ((i / 8) * 80 + (i & 7) * 0x800);
	}

	/* Pallet */
	outp(0x1000, 0xaa);
	outp(0x1100, 0xcc);
	outp(0x1200, 0xf0);

	if (argc < 2){ //argv[1] == NULL){
		printf("X1 BRG file Loader.\n");
		return ERROR;
	}

	snprintf(dst, sizeof dst, "%s.grb", argv[1]);
	bload(dst, mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0x4000);

	snprintf(dst, sizeof dst, "%s.grr", argv[1]);
	bload(dst, mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0x8000);

	snprintf(dst, sizeof dst, "%s.grg", argv[1]);
	bload(dst, mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0xc000);

	return NOERROR;
}

