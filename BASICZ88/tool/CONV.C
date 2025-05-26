/* MSX-BIN->PC-88-BIN CONV. for GCC */
/* zccb88.batと組み合わせる(pc88_crt0.asmの値を$8A00->$A700に変更) */
#include <stdio.h>

FILE *stream[2];


int conv(char *loadfil, char*savefil)
{
	long i;
	unsigned char pattern[5];
	unsigned short size;

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}
	if ((stream[1] = fopen( savefil, "wb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", savefil);

		fclose(stream[1]);
		return 1;
	}

	fread(pattern, 1, 1, stream[0]);		/* ヘッダ(MSX) */
	fread(pattern, 1, 2, stream[0]);		/* 先頭番地(MSX) */
	fwrite(pattern, 1, 2, stream[1]);
	fread(pattern, 1, 2, stream[0]);		/* 終了番地(MSX) */
	size = pattern[0] + pattern[1] * 256;
	++size;
	pattern[0] = size % 256;
	pattern[1] = size / 256;
	fwrite(pattern, 1, 2, stream[1]);		/* 終了番地(PC-88)に変換 */
	fread(pattern, 1, 2, stream[0]);		/* 実行番地(MSX) 読み捨てる */

	for(;;){
		i = fread(pattern, 1, 1, stream[0]);
		if(i < 1)
			break;
		i = fwrite(pattern, 1, 1, stream[1]);
		if(i < 1)
			break;
	}
	fclose(stream[0]);
	fclose(stream[1]);

	return 0;
}


int	main(int argc,char **argv){

	if (argv[1] == NULL)
		return 1;
	if (argv[2] == NULL)
		return 1;

	conv(argv[1], argv[2]);

	return 0;
}
