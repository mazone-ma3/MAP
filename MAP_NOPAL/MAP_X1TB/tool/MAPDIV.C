/* MAPDIV.C for GCC */
/* MSX2の16Kの画像マップファイルをX1用に8Kずつに2分割する */
#include <stdio.h>

FILE *stream[3];

#define MAPSIZE 16384
#define DIV 2

int conv(char *loadfil, char *savefil1, char *savefil2)
{
	long i,size;
	unsigned char pattern[16384+2];

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}
	if ((stream[1] = fopen( savefil1, "wb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", savefil1);

		fclose(stream[1]);
		return 1;
	}
	if ((stream[2] = fopen( savefil2, "wb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", savefil2);

		fclose(stream[2]);
		return 1;
	}
	/* MSXヘッダを読み捨てる */
	fread(pattern, 1, 1, stream[0]);
	fread(pattern, 1, 4, stream[0]);
	fread(pattern, 1, 2, stream[0]);

	size = fread(pattern, 1, MAPSIZE, stream[0]);
	if(size > 0){
		i = fwrite(pattern, 1, MAPSIZE/DIV, stream[1]);
		if(i > 0){
			size -= i;
			if(size > 0){
				i = fwrite(&pattern[MAPSIZE/DIV], 1, size, stream[2]);
			}
		}
	}
	fclose(stream[0]);
	fclose(stream[1]);
	fclose(stream[2]);

	return 0;
}


int	main(int argc,char **argv){

	if (argv[1] == NULL)
		return 1;
	if (argv[2] == NULL)
		return 1;
	if (argv[3] == NULL)
		return 1;

	conv(argv[1], argv[2], argv[3]);

	return 0;
}
