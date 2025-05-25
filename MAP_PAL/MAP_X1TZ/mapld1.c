/* z88dk X1/turbo/Z  ƒƒCƒ“RAM->G-VRAM BANK1“]‘—(1) */

#include <stdio.h>
#include <stdlib.h>
//#include <x1.h>

#define MAINRAM_ADR 0xd000
#define MAP_ADR 0x8000
#define MAP_SIZE 16384/2


void main(void)
{
	int i;
	unsigned char *data;
	unsigned short map_pointer;
//	unsigned char *WK1FD0 = (unsigned char *)0xf8d6;

	data = (unsigned char *)MAINRAM_ADR;
	map_pointer = MAP_ADR;

//	outp(0x1fd0, *WK1FD0 | 0x18); /* BANK1 */
	outp(0x1fd0, 0x18); /* BANK1 */

	for(i = 0; i < MAP_SIZE; ++i){
		outp(map_pointer++ , *data++);
	}

//	outp(0x1fd0, *WK1FD0); /* Œ³‚É–ß‚· */
	outp(0x1fd0, 0); /* Œ³‚É–ß‚· */
}
