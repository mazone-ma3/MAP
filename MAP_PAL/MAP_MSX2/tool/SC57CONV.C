/* MSX-SC5->SC7 CONV. for GCC */

#include <stdio.h>

#define WIDTH (256 / 2)
#define LINE 212

FILE *stream[2];

int conv(char *loadfil, char*savefil)
{

	long i, j,k,count, count2;
	unsigned char pattern[100], data1, data2;
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

	fread(pattern, 1, 1, stream[0]);	/* MSXSC5先頭ヘッダ 0xFE */
	fwrite(pattern, 1, 1, stream[1]);	/* MSXSC7先頭ヘッダ 0xFE */

	fread(pattern, 1, 2, stream[0]);	/* MSXSC5ヘッダ 開始番地 */
	if(pattern[0] || pattern[1]){
		fprintf(stderr, "Can\'t convert file %s.", loadfil);

		fclose(stream[0]);
		fclose(stream[1]);
		return 1;
	}
	fwrite(pattern, 1, 2, stream[1]);	/* MSXSC7ヘッダ 開始番地 */

	fread(pattern, 1, 2, stream[0]);	/* MSXSC5ヘッダ 終了番地 */
	size = pattern[0] + pattern[1] * 256 + 1;
	size *= 2;
	--size;
	pattern[0] = size % 256;
	pattern[1] = size / 256;
	fwrite(pattern, 1, 2, stream[1]);	/* MSXSC7ヘッダ 終了番地 */

	fread(pattern, 1, 2, stream[0]);	/* MSXSC5ヘッダ 実行番地 */
	fwrite(pattern, 1, 2, stream[1]);	/* MSXSC7ヘッダ 実行番地 */

	for(count = 0; count < LINE; ++count){

		for(count2 = 0; count2 < WIDTH ; ++count2){
	
			fread(pattern, 1, 1, stream[0]);	/* 1Byte=2dot分 */
			data1 = pattern[0] /  16;
			data2 = pattern[0] %  16;
			/* 横2倍に拡大する */
			pattern[0] = data1 * 16 + data1;
			pattern[1] = data2 * 16 + data2;
			fwrite(pattern, 1, 2, stream[1]);
		}
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
