#include <stdio.h>
#include <stdlib.h>

void main(void)
{
	unsigned char a, b, *mem, *mem2, *mem3;
	int i,j, k;
//    printf("hello, world");

#asm
	di
#endasm
	outp(0xcf, 0);

	outp(0xb4,5);
	a = inp(0xb5);
	outp(0xb4,6);
	b = inp(0xb5);

	outp(0xb4,5);
	outp(0xb5,0x38);
	outp(0xb4,6);
	outp(0xb5,0x39);

/*	for(mem = (unsigned char *)0xa000; mem <= 0xdfff; ++mem){
		*mem = 0xff;
	}
*/
	/* Set Text */
	mem = (unsigned char *)0xa000;
	for(i = 0; i < 256; ++i){
		mem[i] = i;			/* Text1 */
		mem[i+0x800] = 0x0f;			/* Atr */
		mem[i+0x1000] = 0 ;//(i % 2) * 2;	/* Text2 */
	}

	/* Set PCG */
//	mem2 = (unsigned char *)0xc000;
	mem3 = (unsigned char *)0x6000;
	for(j = 0; j < 3; ++j){
		mem2 = (unsigned char *)(0xc000 + 0x800 * (j+1));
		for(i = 0; i < 256 * 8 / 8; ++i){
//			mem3 = (unsigned char *)(0x6000 + k + table[i] * 8 + j*0x600);

			for(k = 0; k < 8; ++k){
			*(mem2++) = *(mem3++);
//			*(mem2++) = *(mem3++);
			}
		}
	}

	outp(0xb4,5);
	outp(0xb5,a);
	outp(0xb4,6);
	outp(0xb5,b);

#asm
	ei
#endasm
}