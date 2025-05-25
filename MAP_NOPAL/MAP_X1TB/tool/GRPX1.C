/* MSX-SC5->X1 GRP CONV. for GCC */
/* 2分割する */
/* 右半分版 */

#include <stdio.h>

/************************************************************************/
/*		BIT操作マクロ定義												*/
/************************************************************************/

/* BITデータ算出 */
#define BITDATA(n) (1 << (n))

/* BITセット */
#define BITSET(BITNUM, NUMERIC) {	\
	NUMERIC |= BITDATA(BITNUM);		\
}

/* BITクリア */
#define BITCLR(BITNUM, NUMERIC) {	\
	NUMERIC &= ~BITDATA(BITNUM);	\
}

/* BITチェック */
#define BITTST(BITNUM, NUMERIC) (NUMERIC & BITDATA(BITNUM))

/* BIT反転 */
#define BITNOT(BITNUM, NUMERIC) {	\
	NUMERIC ^= BITDATA(BITNUM);		\
}

#define WIDTH 64	/* MSX2の画面半分(byte)だけ保存する */
#define LINE 40 // 40
#define RAM_ADR 0x2000

FILE *stream[2];

/* 黒をシアンで出力して、X1でシアンを黒に設定する(マスクの代わり) */
/* 1色減る代わりにORを取る必要が無い */
unsigned char conv_tbl[16] = { 0, 3, 4, 4, 1, 1, 2, 5, 2, 2, 6, 6 ,4 ,2 ,7 ,7 };

int conv(char *loadfil, char*savefil1, char *savefil2)
{

	long i, j,k,count, count2, size = 0;
	unsigned char pattern[100];
	unsigned char x1color[3];
	unsigned char msxcolor[8];
	unsigned char color;
	unsigned short header;

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

	fread(pattern, 1, 1, stream[0]);	/* MSX先頭を読み捨てる */
	fread(pattern, 1, 4, stream[0]);	/* MSXヘッダも読み捨てる */
	fread(pattern, 1, 2, stream[0]);	/* MSXヘッダを読み捨てる */


	for(count = 0; count < LINE * 2; ++count){

		for(count2 = 0; count2 < WIDTH / 2 / 2; ++count2){
			i = fread(pattern, 1, 2, stream[0]);	/* 読み捨てる */
			if(i < 1)
				break;
			i = fread(pattern, 1, 2, stream[0]);	/* 読み捨てる */
			if(i < 1)
				break;
		}

		for(count2 = 0; count2 < WIDTH / 2 / 2; ++count2){
	
		i = fread(pattern, 1, 2, stream[0]);	/* 4dot分 */
		if(i < 1)
			break;

		/* 色分解と拡大 */
		msxcolor[0] = (pattern[0] >>4) & 0x0f;
		msxcolor[1] = (pattern[0] >>4) & 0x0f;
		msxcolor[2] = pattern[0] & 0x0f;
		msxcolor[3] = pattern[0] & 0x0f;
		msxcolor[4] = (pattern[1] >>4) & 0x0f;
		msxcolor[5] = (pattern[1] >>4) & 0x0f;
		msxcolor[6] = pattern[1] & 0x0f;
		msxcolor[7] = pattern[1] & 0x0f;
		for(i = 0; i < 3; ++i){
			x1color[i] = 0;
		}

		for(j = 0; j < 8; ++j){
			for(i = 0; i < 3; ++i){
				color = conv_tbl[msxcolor[j]];	/* 色変換 */
				if(BITTST(i, color)){
					BITSET(7-j, x1color[i]);
				}else{
					BITCLR(7-j, x1color[i]);
				}
			}
		}


		i = fread(pattern, 1, 2, stream[0]);	/* 4dot分 */
		if(i < 1)
			break;

		/* 色分解と拡大 */
		msxcolor[0] = (pattern[0] >>4) & 0x0f;
		msxcolor[1] = (pattern[0] >>4) & 0x0f;
		msxcolor[2] = pattern[0] & 0x0f;
		msxcolor[3] = pattern[0] & 0x0f;
		msxcolor[4] = (pattern[1] >>4) & 0x0f;
		msxcolor[5] = (pattern[1] >>4) & 0x0f;
		msxcolor[6] = pattern[1] & 0x0f;
		msxcolor[7] = pattern[1] & 0x0f;

		for(i = 0; i < 3; ++i){
			pattern[i] = x1color[i];
		}

		for(i = 0; i < 3; ++i){
			x1color[i] = 0;
		}

		for(j = 0; j < 8; ++j){
			for(i = 0; i < 3; ++i){
				color = conv_tbl[msxcolor[j]];	/* 色変換 */
				if(BITTST(i, color)){
					BITSET(7-j, x1color[i]);
				}else{
					BITCLR(7-j, x1color[i]);
				}
			}
		}

		for(i = 0; i < 3; ++i){
			pattern[3 + i] = x1color[i];
		}

		if(size < 0x2000){ //WIDTH * LINE * 3
			i = fwrite(pattern, 1, 6, stream[1]);	/* 8dot分*2 */
			size += i;
			if(i < 1)
				break;
		}else{
			i = fwrite(pattern, 1, 6, stream[2]);	/* 8dot分*2 */
			size += i;
			if(i < 1)
				break;
		}
		}
	}
	fclose(stream[0]);
	fclose(stream[1]);
	fclose(stream[2]);
	printf("All Size = %d\n",size);

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
