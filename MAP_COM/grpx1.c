/* z88dk X1turboZ  キャラMAP表示実験 ZSDCC版 By m@3 */
/* .com版 */
/* キャラを出す  */

#include <stdio.h>
#include <stdlib.h>

#define CHR_SIZE 0x1E00 //0xf00

unsigned char vram_data[CHR_SIZE];

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

short pat_tmp;
unsigned char no;

unsigned char i, j;

#define VRAM_MACRO(X,Y) (X + (Y / 8) * 80 + (Y & 7) * 0x800)
unsigned char data2[3][2][8];

/* 3プレーン転送 */
/* 0x4000,0x8000,0xc000 */
void put_chrx1_pat(unsigned short patadr)
{
	unsigned char *adr_tmp;
	unsigned short adr_tmp2;
	unsigned short ii,jj;
	adr_tmp = &vram_data[patadr * 2 + patadr];
	for(jj = 0 ; jj < PARTS_Y; ++jj){
		for(ii = 0 ; ii < PARTS_X; ++ii){
			data2[0][ii][jj] = (*adr_tmp++);
			data2[1][ii][jj] = (*adr_tmp++);
			data2[2][ii][jj] = (*adr_tmp++);
		}
		adr_tmp += (32 * 3 - PARTS_X * 3);
	}

	for(jj = 0 ; jj < PARTS_Y; ++jj){
		adr_tmp2 = VRAM_MACRO((i * PARTS_X), (j * PARTS_Y + jj));
		for(ii = 0 ; ii < PARTS_X; ++ii){
			outp(0x4000 + adr_tmp2, data2[0][ii][jj]);
			outp(0x8000 + adr_tmp2, data2[1][ii][jj]);
			outp(0xc000 + adr_tmp2, data2[2][ii][jj]);
			++adr_tmp2;
		}
	}
}

unsigned char pat_no;
unsigned char *pat_adr;

int	main(int argc,char **argv)
{
	unsigned short pat_tmp;

	if (argc < 2){ //argv[1] == NULL){
		printf("X1 .grp file Loader.\n");
		return ERROR;
	}

	if(bload(argv[1], vram_data, CHR_SIZE))
		return ERROR;

	/* Pallet */
	outp(0x1000, 0xaa);
	outp(0x1100, 0xcc);
	outp(0x1200, 0xf0);

	for(j = 0; j < 10; ++j){
		for(i = 0; i < 16; ++i){
			pat_tmp = i * 2 + j * 16 * 16;

			put_chrx1_pat(pat_tmp);
		}
	}

	return NOERROR;
}

