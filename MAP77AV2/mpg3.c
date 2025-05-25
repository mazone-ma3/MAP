/* GCC6809 FM77AVマップ表示テスト By m@3 */
/* キャラMAP移動実験 2x2圧縮展開 */
/* サブCPU止めてVRAMアクセス メインに1プレーン16K全展開する */
/* キャラを出す 低解像度版 SUB CPUコマンド版 */

//#include "math.h"

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
#define PAGE 0xa

#define MAPPAT_ADR 0x1280 //0x2000
#define CHRPAT_ADR 0x1e80 //0x4000
#define MASK_DATA_ADR 0x2180 //0x1600
#define MAP_ADR 0x2280 //(PAGE * 0x1000)
#define PARTS_DATA (MAP_ADR+0x3c00)

#define VRAM_DATA_ADR (PAGE * 0x1000)

#define SUBPAT_ADR 0xa500
#define SUBPAT_ADR2 0xd500

#define SIZE 40

#define X_SIZE 18
#define Y_SIZE 18

#define PARTS_X 1
#define PARTS_Y 8

#define MAP_SIZE_X 128
#define MAP_SIZE_Y 128

#define OFS_X 2
#define OFS_Y 2

#define CHR_X 8
#define CHR_Y 8

unsigned char vram_tmp[PARTS_X * PARTS_Y * 3];
unsigned char map_data[(X_SIZE+2) * 32];

unsigned char *subram_table[30];

#define MAXCOLOR 8

/* BRG */

unsigned char org_pal[MAXCOLOR][3] =
	{{ 0, 0, 0 },
	{ 0, 0, 15 },
	{ 15, 0, 0 },
	{ 0, 0, 0 },//	{ 15, 0, 15 },
	{ 0, 15, 0 },
	{ 0, 15, 15 },
	{ 15, 15, 0 },
	{ 15, 15, 15 },};

unsigned char no;
unsigned char *vram_adr;

unsigned char chr_tbl[8][4] = {
		{0, 1, 0 + 16, 1 + 16},
		{2, 3, 2 + 16, 3 + 16},
		{4, 5, 4 + 16, 5 + 16},
		{6, 7, 6 + 16, 7 + 16},
		{8, 9, 8 + 16, 9 + 16},
		{10, 11, 10 + 16, 11 + 16},
		{12, 13, 12 + 16, 13 + 16},
		{14, 15, 14 + 16, 15 + 16},
};
unsigned char dir = 2, dir2 = 0;
unsigned char i, j;


unsigned char *mem;
unsigned char msr_sv;
unsigned char key, st3;

#define mmr ((unsigned char *)0xFD80)
//#define mem (((unsigned char *)0x6AFF)
#define msr ((unsigned char *)0xFD93)
#define keyport ((unsigned char *)0xFD01)
#define opncom ((unsigned char *)0xFD15)
#define opndat ((unsigned char *)0xFD16)

void keyscan_on(void)
{
asm(
	"	jmp	_SCAN\n"
"_SUBOUT:\n"
	"	.byte	16\n"
	"	.blkb	1\n"
	"	.word	_KEYCON\n"
	"	.word	6\n"
	"	.byte	2\n"
"_KEYCON:\n"
	"	.blkb	2\n"
	"	.byte	0x45\n"
	"	.byte	0\n"
	"	.byte	2\n"
	"	.blkb	1\n"
"_SCAN:\n"
	"	ldx	#_SUBOUT\n"
	"	jsr [0xFBFA]\n"
);
}

void keyscan_off(void)
{
asm(
	"	ldx	#_SUBOUT2\n"
	"	jsr [0xFBFA]\n"
	"	lda #0\n"
	"	sta _KEYMODE\n"
	"	bra	_ENDOFF\n"

"_SUBOUT2:\n"
	"	.byte	16\n"
	"	.blkb	1\n"
	"	.word	_KEYCON2\n"
	"	.word	6\n"
	"	.blkb	2\n"

"_KEYCON2:\n"
	"	.blkb	2\n"
	"	.byte	0x45\n"
	"	.byte	0x00\n"
"_KEYMODE:\n"
	"	.byte	0x00\n"
	"	.blkb	1\n"

"_ENDOFF:\n"
);
}

void keyrepeat_on(void)
{
asm(
	"	jmp	_repeat\n"
"_SUBOUT3:\n"
	"	.byte	16\n"
	"	.blkb	1\n"
	"	.word	_KEYREP\n"
	"	.word	6\n"
	"	.byte	2\n"
"_KEYREP:\n"
	"	.blkb	2\n"
	"	.byte	0x45\n"
	"	.byte	4\n"
	"	.byte	0\n"
	"	.blkb	1\n"
"_repeat:\n"
	"	ldx	#_SUBOUT3\n"
	"	jsr [0xFBFA]\n"
);
}

void keyrepeat_off(void)
{
asm(
	"	jmp	_repeat2\n"
"_SUBOUT4:\n"
	"	.byte	16\n"
	"	.blkb	1\n"
	"	.word	_KEYREP2\n"
	"	.word	6\n"
	"	.byte	2\n"
"_KEYREP2:\n"
	"	.blkb	2\n"
	"	.byte	0x45\n"
	"	.byte	4\n"
	"	.byte	1\n"
	"	.blkb	1\n"
"_repeat2:\n"
	"	ldx	#_SUBOUT4\n"
	"	jsr [0xFBFA]\n"
);
}


unsigned char keycode, keyflag;

void  key_sense(void)
{
//	unsigned char keycode = 0, old_keycode = 0;
//	while((keycode = *keyport) != old_keycode){
		keycode = *keyport;
		if((keycode == 0x4d) || (keycode == 0x3b)){
			BITSET(0, st3);
//			BITCLR(1, st3);
		}else if((keycode == 0xcd) || (keycode == 0xbb)){
			BITCLR(0, st3);
		}else if((keycode == 0x51) || (keycode == 0x40)){
			BITSET(3, st3);
//			BITCLR(2, st3);
		}else if((keycode == 0xd1) || (keycode == 0xc0)){
			BITCLR(3, st3);
		}else if((keycode == 0x50) || (keycode == 0x43)){
			BITSET(1, st3);
//			BITCLR(0, st3);
		}else if((keycode == 0xD0) || (keycode == 0xc3)){
			BITCLR(1, st3);
		}else if((keycode == 0x4f) || (keycode == 0x3e)){
			BITSET(2, st3);
//			BITCLR(3, st3);
		}else if((keycode == 0xcf) || (keycode == 0xbe)){
			BITCLR(2, st3);
		}else if((keycode == 0x2a) || (keycode == 0x58)){
			BITSET(4, st3);
		}else if((keycode == 0xaa) || (keycode == 0xd8)){
			BITCLR(4, st3);
		}else if((keycode == 0x2b)){
			BITSET(5, st3);
		}else if((keycode == 0xab)){
			BITCLR(5, st3);
		}
//		old_keycode = keycode;
//	}
}


void set_key_irq(void)
{
asm(

"_TINIT:\n"
	"	orcc	#0x10\n"
	"	ldx		0xFFF8\n"
	"	cmpx	#_IRQET\n"
	"	beq		_TCLOSE\n"
	"	stx		_IRQJP\n"
	"	ldx		#_IRQET\n"
	"	stx		0x0FFF8\n"
	"	bra	_ENDIRQ\n"
"_TCLOSE:\n"
	"	ldx		_IRQJP\n"
	"	stx		0xFFF8\n"
	"	bra	_ENDIRQ\n"

"_IRQET:\n"
	"	pshs	D,X\n"
	"	lda		0xFD03\n"
	"	anda	#1\n"
	"	beq		_IRQRT\n"

	"	jsr	_key_sense\n"

"_IRQRT:\n"
	"	puls	D,X\n"
	"	jmp		[_IRQJP]\n"

"_IRQJP:	.blkb	2\n"
"_TCOUNT:	.blkb	1\n"
"_ENDIRQ:\n"
	"	andcc	#0xEF\n"
);

}

void sub_disable(void)
{
asm(
"_SUBHLT:\n"
	"	lda	0xFD05\n"
	"	bmi	_SUBHLT\n"
	"	orcc	#(0x50)\n"
	"	lda	#0x80\n"
	"	sta	0xFD05\n"
"_LOOP:\n"
	"	lda	0xFD05\n"
	"	bpl	_LOOP\n" // *-3\n"
);

//	msr_sv = *msr;
asm(
	"	lda	0xfd93\n"
	"	sta	_msr_sv\n"
	"	ora	#0x80\n"
	"	sta	0xfd93\n"
);
//	*msr |= 0x80;
}

void sub_enable(void)
{
//	*msr = msr_sv;

asm(
	"	lda	_msr_sv\n"
	"	sta	0xfd93\n"

	"	ldb	0xFC80\n"
	"	orb	#0x80\n"
	"	stb	0xFC80\n"

	"	clr	0xFD05\n"
	"	andcc	#0xAF\n"
);
}

void bank1_on()
{
asm(
	"	lda	#0x1d\n"
	"	sta	0xfd8a\n"
	"	lda	#0\n"
	"	sta	0xa410\n"
	"	lda	#0x20\n"
	"	sta	0xa430\n"
);
//	mmr[PAGE] = 0x1d;
//	vram[0x410] = 0;
//	vram[0x430] = 0x20; //0x60;
}

void bank1_off(void)
{
asm(
	"	lda	#0x1d\n"
	"	sta	0xfd8a\n"
	"	lda	#0\n"
	"	sta	0xa410\n"
	"	sta	0xa430\n"
);
//	mmr[PAGE] = 0x1d;
//	vram[0x410] = 0;
//	vram[0x430] = 0x00;
}

void vram_b_on(void)
{
asm(
	"	lda	#0x10\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"
);
//	mmr[PAGE] = 0x10;
//	mmr[PAGE+1] = 0x11;
//	mmr[PAGE+2] = 0x12;
//	mmr[PAGE+3] = 0x13;
}

void vram_r_on(void)
{
asm(
	"	lda	#0x14\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"
);
//	mmr[PAGE] = 0x14;
//	mmr[PAGE+1] = 0x15;
//	mmr[PAGE+2] = 0x16;
//	mmr[PAGE+3] = 0x17;
}

void vram_g_on(void)
{
asm(
	"	lda	#0x18\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"
);
//	mmr[PAGE] = 0x18;
//	mmr[PAGE+1] = 0x19;
//	mmr[PAGE+2] = 0x1a;
//	mmr[PAGE+3] = 0x1b;
}

void vram_off(void)
{
asm(
	"	lda	#0x3a\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"
);
//	mmr[PAGE] = 0x30+PAGE;
//	mmr[PAGE+1] = 0x31+PAGE;
//	mmr[PAGE+2] = 0x32+PAGE;
//	mmr[PAGE+3] = 0x33+PAGE;
}


void write_opn(unsigned char reg, unsigned char data)
{
	*opndat = reg;
	*opncom = 0x03;
	*opncom = 0x00;
	*opndat = data;
	*opncom = 0x02;
	*opncom = 0x00;
}

unsigned char read_opn(unsigned char reg)
{
	unsigned char data;

	*opndat = reg;
	*opncom = 0x03;
	*opncom = 0x00;
	*opncom = 0x09;
	data = *opndat;
	*opncom = 0x00;

	return data;
}

//unsigned char *adr_tmp, *adr_tmp3;
//unsigned short *adr_tmp2;
unsigned char ii, jj;

unsigned char subcpu_flag = 0;

void sub_draw(void)
{
asm(
	"	lda	#0x3f\n"
	"	sta	0xfc82\n"
	"	lda	_subcpu_flag\n"
	"	bne	_SKIP1\n"
	"	lda	#1\n"
	"	sta	_subcpu_flag\n"
	"	jsr	_MOVCMD\n"
"_SKIP1:\n"
	"	jsr	_SUBMOV\n"
	"	bra	_ENDCD\n"

"_SUBHLT1:\n"
	"	lda	0xfd05\n"
	"	bmi	_SUBHLT1\n"
	"	orcc	#0x50\n"
	"	lda	#0x80\n"
	"	sta	0xfd05\n"
"_SUBHLT2:\n"
	"	lda	0xfd05\n"
	"	bpl	_SUBHLT2\n"
	"	rts\n"

"_RDYREQ:\n"
	"	lda	0xfc80\n"
	"	ora	#0x80\n"
	"	sta	0xfc80\n"
	"	rts\n"

"_SUBMOV:\n"
	"	clra\n"
	"	sta	0xfd05\n"
	"	andcc	#0xaf\n"
	"	rts\n"

"_MOVCMD:\n"
	"	ldx	#_TESTCD\n"
	"	ldy	#0xfc82\n"
"_LOOP1:\n"
	"	lda	,X+\n"
	"	sta	,Y+\n"
	"	cmpx	#_ENDCD\n"
	"	bne	_LOOP1\n"
	"	rts\n"

"_TESTCD:\n"
	"	.byte	0x3f\n"
	"	.blkb	8\n"
	"	.byte	0x93\n"
	"	.word	0xd38f\n"
	"	.byte	0x90\n"

	"	ldu	0xd3e0\n"
	"	ldx	0xd3e2\n"	//	"	ldx	#0xd3c0\n"
	"	ldy	#8\n"
//	"	sta	0xd3e2\n"

"_TESTLP:\n"
	"	lda ,x\n"
	"	sta 0x0000,u\n"
	"	lda 1,x\n"
	"	sta 0x4000,u\n"
	"	lda 2,x\n"
	"	sta 0x8000,u\n"

	"	leax 3,x\n"
	"	leau 40,u\n"

//	"	dec	0xd3e2\n"
	"	leay	-1,y\n"
	"	bne	_TESTLP\n"

	"	rts\n"

"_ENDCD:\n"
//";	equ	*\n"
);
}

/* RAM テンポラリ->VRAM 3プレーン転送(SUB CPU) */
void put_sub(void)
{
	register unsigned char *rx asm("x");
asm(
	"	jsr	_SUBHLT1\n"
);

	rx = (unsigned char *)vram_tmp;

asm(
//	"	jsr	_SUBHLT1\n"

//	"	ldx	_adr_tmp\n"
//	"	ldx	%0\n"
	"	ldy	#0xfcc0\n"
"_LOOP0:\n"
	"	lda	,X+\n"
	"	sta	,Y+\n"
	"	cmpy	#0xfcc0+24\n"
	"	bne	_LOOP0\n"

	"	ldd	_vram_adr\n" //_adr_tmp2\n"
	"	std	0xfce0\n"
	"	ldd	#0xd3c0\n"
	"	std	0xfce2\n"

		:	/* 値が返るレジスタ変数 */
		:"r"(rx)	/* 引数として使われるレジスタ変数 */
		:"y","d"	/* 破壊されるレジスタ */
);
	sub_draw();
}

/* メインRAM->VRAM 3プレーン転送(SUB CPU) */
void put_sub2(unsigned short patadr)
{
	register unsigned char *rx asm("x");

asm(
	"	jsr	_SUBHLT1\n"
);

	rx = (unsigned char *)MAPPAT_ADR + patadr * 2 + patadr; 
//	adr_tmp2 = (unsigned short *)(vram_adr);

asm(
//	"	jsr	_SUBHLT1\n"

//	"	ldx	_adr_tmp\n"
	"	ldy	#0xfcc0\n"
"_LOOP00:\n"
	"	lda	,X\n"
	"	sta	,Y+\n"
	"	lda	1,X\n"
	"	sta	,Y+\n"
	"	lda	2,X\n"
	"	sta	,Y+\n"

	"	leax 48,x\n"

	"	cmpy	#0xfcc0+24\n"
	"	bne	_LOOP00\n"

	"	ldd	_vram_adr\n" //_adr_tmp2\n"
	"	std	0xfce0\n"
	"	ldd	#0xd3c0\n"
	"	std	0xfce2\n"
		:	/* 値が返るレジスタ変数 */
		:"r"(rx)	/* 引数として使われるレジスタ変数 */
		:"y","d"	/* 破壊されるレジスタ */
);
	sub_draw();
}

/* SUB RAM->VRAM 3プレーン転送(SUB CPU) */
void put_sub3(void)
{
	register unsigned char *rd asm("d");

asm(
	"	jsr	_SUBHLT1\n"
);
//	adr_tmp = (unsigned char *)SUBPAT_ADR2 + no * PARTS_X * PARTS_Y * 3;
//	adr_tmp = subram_table[no];
	rd = subram_table[no];
asm(
//	"	jsr	_SUBHLT1\n"

//	"	ldd	_adr_tmp\n"
	"	std	0xfce2\n"
	"	ldd	_vram_adr\n"
	"	std	0xfce0\n"

		:	/* 値が返るレジスタ変数 */
		:"r"(rd)	/* 引数として使われるレジスタ変数 */
		:"y","x"	/* 破壊されるレジスタ */
);
	sub_draw();
}

/* RAM テンポラリ->VRAM 3プレーン転送 */
/*void copy_chr_fm_pat(void)
{
	unsigned char jj;

	adr_tmp = ((unsigned char *)vram_tmp);
	adr_tmp2 = vram_adr;

//	for(jj = 0 ; jj < PARTS_Y; ++jj){
//		sub_disable();
//		vram_b_on();
//		*adr_tmp2 = (unsigned short)(adr_tmp[0] * 256 | adr_tmp[3]);
//		vram_r_on();
//		*adr_tmp2 = (unsigned short)(adr_tmp[1] * 256 | adr_tmp[4]);
//		vram_g_on();
//		*adr_tmp2 = (unsigned short)(adr_tmp[2] * 256 | adr_tmp[5]);
//		sub_enable();
//		adr_tmp += 3;//(PARTS_X * 3);
//		adr_tmp2 += 40; //(SIZE / 2);
//	}

asm(
	"	ldx _adr_tmp\n"
	"	ldy _adr_tmp2\n"
	"	lda	#8\n"
	"	sta	_jj\n"
"loop_pat:\n"
	"	;jsr _sub_disable\n"

	"	lda	#0x10\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"

	"	lda ,x\n"
	"	sta ,y\n"

	"	lda	#0x14\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"

	"	lda 1,x\n"
	"	sta ,y\n"

	"	lda	#0x18\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"

	"	lda 2,x\n"
	"	sta ,y\n"

	"	;jsr _sub_enable\n"

	"	leax 3,x\n"
	"	leay 40,y\n"

	"	dec _jj\n"
	"	bne loop_pat\n"
);
*/
//		vram_off();
/*asm(
	"	lda	#0x3a\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"
);*/

//}

/* メインRAM->RAM テンポラリ 3プレーンMASK転送 AND */
void put_chr_fm_pat_chr_and(unsigned short patadr)
{
	register unsigned char *rx asm("x");
	register unsigned char *ry asm("y");

//	unsigned char *adr_tmp, *adr_tmp2,;
//	unsigned char data_tmp;
//	unsigned char ii,jj;
//	adr_tmp
	rx = ((unsigned char *)MASK_DATA_ADR + (patadr));
//	adr_tmp3
	ry = ((unsigned char *)vram_tmp);
/*	for(jj = 0 ; jj < PARTS_Y; ++jj){
		for(ii = 0 ; ii < PARTS_X; ++ii){
			adr_tmp = ((unsigned char *)MASK_DATA_ADR + (patadr + (ii + jj * 32)));
			data_tmp =  ~*(adr_tmp);
			*(adr_tmp3) &= data_tmp;
			*(adr_tmp3+1) &= data_tmp;
			*(adr_tmp3+2) &= data_tmp;
			++adr_tmp;
			adr_tmp3 += 3;
		}
		adr_tmp += 15; //(16 - PARTS_X);	//(15)
	}
*/
asm(
//	"	ldx _adr_tmp\n"
//	"	ldy _adr_tmp3\n"

	"	lda	#8\n"
	"	sta	_ii\n"
"loop_and1:"
	"	lda	#1\n"
	"	sta	_jj\n"
"loop_and2:"
	"	lda ,x\n"
	"	coma\n"
	"	tfr a,b\n"

	"	anda ,y\n"
	"	sta ,y\n"

	"	tfr b,a\n"
	"	anda 1,y\n"
	"	sta 1,y\n"

	"	tfr b,a\n"
	"	anda 2,y\n"
	"	sta 2,y\n"

	"	leax 1,x\n"
	"	leay 3,y\n"

	"	dec _jj\n"
	"	bne loop_and2\n"

	"	leax 15,x\n"

	"	dec _ii\n"
	"	bne loop_and1\n"

		:	/* 値が返るレジスタ変数 */
		:"r"(rx),"r"(ry)	/* 引数として使われるレジスタ変数 */
		:"d"	/* 破壊されるレジスタ */
);
}

/* メインRAM->RAM テンポラリ 3プレーン転送 OR */
void put_chr_fm_pat_chr_or(unsigned short patadr)
{
//	unsigned char *adr_tmp, *adr_tmp2;
//	unsigned char ii,jj;
	register unsigned char *rx asm("x");
	register unsigned char *ry asm("y");

//	adr_tmp 
	rx = ((unsigned char *)CHRPAT_ADR + (patadr * 2 + patadr));
//	adr_tmp3 
	ry = ((unsigned char *)vram_tmp);
/*	for(jj = 0 ; jj < PARTS_Y; ++jj){
		for(ii = 0 ; ii < PARTS_X; ++ii){
			*(adr_tmp3) |= *(adr_tmp);
			*(adr_tmp3+1) |= *(adr_tmp+1);
			*(adr_tmp3+2) |= *(adr_tmp+2);
			adr_tmp += 3;
			adr_tmp3 += 3;
		}
		adr_tmp += (16 * 3 - PARTS_X * 3);		//(45)
	}
*/
asm(
//	"	ldx _adr_tmp\n"
//	"	ldy _adr_tmp3\n"

	"	lda	#8\n"
	"	sta	_ii\n"
"loop_or1:"
	"	lda	#1\n"
	"	sta	_jj\n"
"loop_or2:"
	"	ldd ,x\n"
	"	ora ,Y\n"
	"	orb 1,Y\n"
	"	std ,y\n"

	"	lda 2,x\n"
	"	ora 2,Y\n"
	"	sta 2,y\n"

	"	leax 3,x\n"
	"	leay 3,y\n"

	"	dec _jj\n"
	"	bne loop_or2\n"

	"	leax 45,x\n"

	"	dec _ii\n"
	"	bne loop_or1\n"
		:	/* 値が返るレジスタ変数 */
		:"r"(rx),"r"(ry)	/* 引数として使われるレジスタ変数 */
		:"d"	/* 破壊されるレジスタ */
);
}

/* メインRAM->RAM テンポラリ 3プレーン転送 */
void put_chr_fm_pat_map(unsigned short patadr)
{
//	unsigned char *adr_tmp, *adr_tmp2;
//	unsigned char ii,jj;
	register unsigned char *rx asm("x");
	register unsigned char *ry asm("y");

//	adr_tmp 
	rx = (unsigned char *)MAPPAT_ADR + patadr * 2 + patadr;
//	adr_tmp3 
	ry = ((unsigned char *)vram_tmp);
/*	for(jj = 0 ; jj < PARTS_Y; ++jj){
		for(ii = 0 ; ii < PARTS_X; ++ii){
			*(adr_tmp3) = *(adr_tmp);
			*(adr_tmp3+1) = *(adr_tmp+1);
			*(adr_tmp3+2) = *(adr_tmp+2);
			adr_tmp += 3;
			adr_tmp3 += 3;
		}
		adr_tmp += ((16 * 3) - PARTS_X * 3);		//(45)
	}
*/
asm(
//	"	ldx _adr_tmp\n"
//	"	ldy _adr_tmp3\n"

	"	lda	#8\n"
	"	sta	_ii\n"
"loop_map1:"
	"	lda	#1\n"
	"	sta	_jj\n"
"loop_map2:"
	"	ldd ,x\n"
	"	std ,y\n"
	"	lda 2,x\n"
	"	sta 2,y\n"

	"	leax 3,x\n"
	"	leay 3,y\n"

	"	dec _jj\n"
	"	bne loop_map2\n"

	"	leax 45,x\n"

	"	dec _ii\n"
	"	bne loop_map1\n"
		:	/* 値が返るレジスタ変数 */
		:"r"(rx),"r"(ry)	/* 引数として使われるレジスタ変数 */
		:"d"/* 破壊されるレジスタ */
);
}

/* メインRAM->VRAM 3プレーン転送 */
/*void put_chrfm_chr(unsigned short patadr)
{

	adr_tmp = (unsigned char *)CHRPAT_ADR + patadr * 2 + patadr; 
	adr_tmp2 = vram_adr + 0x2000;

//	for(jj = 0 ; jj < PARTS_Y; ++jj){
//		sub_disable();
//		vram_b_on();
//		*adr_tmp2 = (unsigned short)(adr_tmp[0] * 256 | adr_tmp[3]);
//		vram_r_on();
//		*adr_tmp2 = (unsigned short)(adr_tmp[1] * 256 | adr_tmp[4]);
//		vram_g_on();
//		*adr_tmp2 = (unsigned short)(adr_tmp[2] * 256 | adr_tmp[5]);
//		sub_enable();
//		adr_tmp += 48;//(16 * 3);
//		adr_tmp2 += 40; //(SIZE / 2);
//	}


asm(
	"	ldx _adr_tmp\n"
	"	ldy _adr_tmp2\n"
	"	lda	#8\n"
	"	sta	_jj\n"
"loop_pt3:\n"
	"	;jsr _sub_disable\n"
	"	lda	#0x10\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"

	"	lda ,x\n"
	"	sta ,y\n"

	"	lda	#0x14\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"

	"	lda 1,x\n"
	"	sta ,y\n"

	"	lda	#0x18\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"

	"	lda 2,x\n"
	"	sta ,y\n"

	"	;jsr _sub_enable\n"

	"	leax 48,x\n"
	"	leay 40,y\n"

	"	dec _jj\n"
	"	bne loop_pt3\n"
);
}*/


/* メインRAM->VRAM 3プレーン転送 */
/*void put_chrfm_pat(unsigned short patadr)
{

	adr_tmp = (unsigned char *)MAPPAT_ADR + patadr * 2 + patadr; 
	adr_tmp2 = vram_adr;

//	for(jj = 0 ; jj < PARTS_Y; ++jj){
//		sub_disable();
//		vram_b_on();
//		*adr_tmp2 = (unsigned short)(adr_tmp[0] * 256 | adr_tmp[3]);
//		vram_r_on();
//		*adr_tmp2 = (unsigned short)(adr_tmp[1] * 256 | adr_tmp[4]);
//		vram_g_on();
//		*adr_tmp2 = (unsigned short)(adr_tmp[2] * 256 | adr_tmp[5]);
//		sub_enable();
//		adr_tmp += 48;//(16 * 3);
//		adr_tmp2 += 40; //(SIZE / 2);
//	}


asm(
	"	ldx _adr_tmp\n"
	"	ldy _adr_tmp2\n"
	"	lda	#8\n"
	"	sta	_jj\n"
"loop_pt2:\n"
	"	;jsr _sub_disable\n"
	"	lda	#0x10\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"

	"	lda ,x\n"
	"	sta ,y\n"

	"	lda	#0x14\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"

	"	lda 1,x\n"
	"	sta ,y\n"

	"	lda	#0x18\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"

	"	lda 2,x\n"
	"	sta ,y\n"

	"	;jsr _sub_enable\n"

	"	leax 48,x\n"
	"	leay 40,y\n"

	"	dec _jj\n"
	"	bne loop_pt2\n"
);
}*/

/* メインRAM->SUB RAM 3プレーン転送 */
void put_chrsub_pat(unsigned short patadr, unsigned char no)
{
	register unsigned char *rx asm("x");
	register unsigned char *ry asm("y");

asm(
	"	jsr _sub_disable\n"
);
//	adr_tmp 
	rx = (unsigned char *)MAPPAT_ADR + patadr * 2 + patadr; 
//	adr_tmp2 
	ry = (unsigned char *)SUBPAT_ADR + no * PARTS_X * PARTS_Y * 3;

asm(
//	"	jsr _sub_disable\n"
//	"	ldx _adr_tmp\n"
//	"	ldy _adr_tmp2\n"
	"	lda	#8\n"
	"	sta	_jj\n"

	"	lda	#0x1d\n"
	"	sta	0xfd8a\n"
	"	inca	\n"
	"	sta	0xfd8b\n"
	"	inca	\n"
	"	sta	0xfd8c\n"
	"	inca	\n"
	"	sta	0xfd8d\n"

"loop_pt2:\n"
	"	ldd ,x\n"
	"	std ,y\n"
	"	lda 2,x\n"
	"	sta 2,y\n"

	"	leax 48,x\n"
	"	leay 3,y\n"

	"	dec _jj\n"
	"	bne loop_pt2\n"

	"	jsr _sub_enable\n"
		:	/* 値が返るレジスタ変数 */
		:"r"(rx),"r"(ry)	/* 引数として使われるレジスタ変数 */
		:"d"	/* 破壊されるレジスタ */
);
}

void pat_sub(void)
{
/*	sub_disable();
	put_chrfm_pat((no & 0x0f) * 1 + (no & 0xf0) * 8); //, vram_adr);
	sub_enable();*/

	if(no < 30)
		put_sub3(); //, vram_adr);
	else
		put_sub2((no & 0x0f) * 1 + (no & 0xf0) * 8); //, vram_a\dr);
}

void chr_sub(void)
{
	unsigned short pat_tmp2;

	unsigned char no2 = chr_tbl[dir * 2 + dir2][(i - CHR_X) + (j - CHR_Y) * 2];

	pat_tmp2 = (no2 & 0x0f) * 1 + (no2 & 0xf0) * 8;

//	put_chrfm_chr(pat_tmp2);
//	pat_sub();

	put_chr_fm_pat_map((no & 0x0f) * 1 + (no & 0xf0) * 8);
	/* マスクを書く */
	put_chr_fm_pat_chr_and(pat_tmp2);
	put_chr_fm_pat_chr_or(pat_tmp2);
/*	sub_disable();
	copy_chr_fm_pat();
	sub_enable();*/
	put_sub();
}

//unsigned char cblue,cred,cgreen;
unsigned char timer;
//unsigned short ccolor;

/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
//	unsigned char *pal_h = (unsigned char *)0xfd30;
//	unsigned char *pal_l = (unsigned char *)0xfd31;

//	unsigned short *pal = (unsigned char *)0xfd30;

/*	unsigned char *pblue = (unsigned char *)0xfd32;
	unsigned char *pred = (unsigned char *)0xfd33;
	unsigned char *pgreen = (unsigned char *)0xfd34;
*/

//	register unsigned char *rx asm("x");

	register unsigned short ry asm("y");
	register unsigned char ra asm("a");
	register unsigned char rb asm("d"); //("b");

	register unsigned short rd asm("d");
	register unsigned short rx asm("x");

	timer = 13;
//	cblue = blue;
//	cred = red;
//	cgreen = green;
//	ccolor = color;

//	*pal_h = color / 256;
//	*pal_l = color % 256;

//	*pal = color;
/*
asm(
//	"	ldy _ccolor\n"
	"	lda _cblue\n"
	"	ldb _cred\n"
);*/
	ra = blue;
	rb = red;
//	rd = (blue * 256) | red;
	rx = rd;
/*
asm(
	"	tfr d,x\n"
);*/
	rb = green;

	ry = color;

asm(
//	"	ldb _cgreen\n"

	"	sty 0xfd30\n"
	"	jsr timing\n"
	"	stx 0xfd32\n"
	"	stb 0xfd34\n"

	"	bra palend\n"

"timing:\n"
	"	lda 0xfd12\n"
	"	bita #0x01\n"
	"	bne tm04\n"
"tm01:\n"
	"	lda 0xfd12\n"
	"	bita #0x02\n"
	"	bne tm01\n"
"tm02:\n"
	"	lda 0xfd12\n"
	"	bita #0x02\n"
	"	beq tm02\n"
	"	lda _timer\n"
"tm03:\n"
	"	deca\n"
	"	bne tm03\n"
"tm04:\n"
	"	rts\n"

"palend:\n"
);
/*
	*pblue = blue;
	*pred = red;
	*pgreen = green;*/
}

void get_pal(unsigned short no, unsigned char *red, unsigned char *green, unsigned char *blue)
{
	*green = *red = *blue = 0;

/*	if(no & 0x4)
		*blue = 0xf;
	if(no & 0x40)
		*red = 0xf;
	if(no & 0x400)
		*green = 0xf;
*/
/*	if(!(*blue | *red | *green)){
*/		if(no & 0x8)
			*blue = 0xf;
		if(no & 0x80)
			*red = 0xf;
		if(no & 0x800)
			*green = 0xf;
/*	}
*/
	if(*red && *blue && !*green)
		*red = *blue = 0;
}

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned short no;
	unsigned char red,green,blue;

//	for(j  = 0; j < 8; j++){
	for(no = 1; no < 4096; no++){
//		no = (j & 1) * 8 + (j & 2) * 64 + (j & 4) * 512;
//	for(no = 4096-8; no < 4096; no++){
		get_pal(no, &red, &green, &blue);
		pal_set(0, no, red, green, blue);
	}
}

void wait_vsync(void)
{
	unsigned char *submode = (unsigned char *)0xfd12;
	while((*submode & 0x01)); /* WAIT VSYNC */
	while(!(*submode & 0x01));
}

void sys_wait(unsigned char wait)
{
	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync();
}

//value < 0 黒に近づける。
//value = 0 設定した色
//value > 0 白に近づける。
void set_constrast(short value, unsigned char org_pal[MAXCOLOR][3], int pal_no)
{
	unsigned short no;
	unsigned char i,j,k,l;
	signed char getpal[3], pal[3];
//	signed char 
//	short temp_pal;
//	unsigned char value2 = value;

//	for(i = 0; i < 16; i++){
//	for(j = 0; j < 16; j++){
//	for(k = 0; k < 16; k++){

	for(i = 1; i < MAXCOLOR; i++){
		no = (i & 1) * 8 + (i & 2) * 64 + (i & 4) * 512;
//	for(no = 0; no < 8; no++){
//		no = i*256+j*16+k;
//		if(!(no & 0xccc))
//			continue;
		get_pal(no, &getpal[0], &getpal[1], &getpal[2]);
//		if((getpal[0] | getpal[1] | getpal[2])){

		for(l = 0; l < 3; l++){
			if(value == 0){
				pal[l] = getpal[l];
			}else if(value > 0){
				pal[l] = getpal[l] + value;
			}else{
/*				temp_pal = (15+value);
				temp_pal *= getpal[l];
				pal[l] = temp_pal / 15;
*/
//				temp_pal = (short)getpal[l] * (15 + value) / 15;
//				pal[l] = (unsigned char)temp_pal;
				pal[l] = (getpal[l] + value) ;
			}

			if(pal[l] < 0)
				pal[l] = 0;
			else if(pal[l] > 15)
				pal[l] = 15;
//		}

		pal_set(pal_no, no, pal[0], pal[1], pal[2]);
		}
	}
//	}
//	}

}

//wait値の速度で黒からフェードインする。
void fadeinblack(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	short j;

//	for(j = -15+8; j <= 0; j+=8){
	for(j = -15; j <= 0; j++){
		set_constrast(j, org_pal, pal_no);
		sys_wait(wait);
	}
}

//wait値の速度で黒にフェードアウトする。
void fadeoutblack(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	short j;

//	for(j = -8; j >= -16; j-=8){
	for(j = 0; j >= -16; j--){
		set_constrast(j, org_pal, pal_no);
		sys_wait(wait);
	}
}

//wait値の速度で白にフェードアウトする。
/*void fadeoutwhite(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = 0; j < 16; j++){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}*/

//パレットを暗転する。
void pal_allblack(int pal_no)
{
	short j, no;
	unsigned short adr1, adr2;

	for(j = 1; j < 4096; j++){
		pal_set(0, j, 0, 0, 0);
	}
}

void get_key(void)
{
asm(
	"	bra	key\n"
"keyin:\n"
	"	.byte	21\n"
	"	.blkb	1\n"
	"	.word	keydt\n"
	"	.blkb	2\n"
	"	.blkb	2\n"
"keydt:\n"
	"	.blkb	2\n"
"key:\n"
	"	ldx	#keyin\n"
	"	jsr	[0xfbfa]\n"
	"	bcs	error\n"
	"	lda	keydt+1\n"
	"	sta	_keyflag\n"
	"	lda	keydt\n"
	"	sta	_keycode\n"
	"	bra	end\n"
"error:\n"
	"	lda	#0\n"
	"	sta	_keyflag\n"
"end:\n"
);
}

void key_clear(void)
{
	while(1){
		get_key();
		if(!keyflag)
			break;
	}
}

//	unsigned short pat_no;
//	unsigned char *pat_adr;
unsigned char data_no, *data, *data_tmp;
unsigned char *map_adr;
unsigned char *old_map_adr;

void map_sub(void)
{
/*			for(j = 0; j < Y_SIZE / 2; ++j){

				for(i = 0; i < X_SIZE / 2; ++i){

					data_no = *data;

					pat_no = ((data_no >> 4) & 0x0f) | ((data_no << 4) & 0xf0);
					pat_adr = (((unsigned char *)PARTS_DATA + pat_no * 4));
//					ii = i * 2 + xx;
//					jj = j * 2 + yy;
					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr++);
					map_adr += (32 - 1);
					*(map_adr++) = *(pat_adr++);
					*map_adr = *(pat_adr);
					map_adr -= (32 - 1);
*/
/*					map_data[ii][jj] = *(pat_adr++);
					map_data[ii+1][jj] = *(pat_adr++);
					map_data[ii][jj+1] = *(pat_adr++);
					map_data[ii+1][jj+1] = *(pat_adr);
*///					++data;
/*				}
				data_tmp += MAP_SIZE_X;
				data = data_tmp;
				map_adr += (32 * 2 - X_SIZE);
			}*/
asm(
	"	lda	#9\n" //Y_SIZE / 2
	"	sta	_j\n"
	"	ldx	_map_adr\n"
"_loop0:"
//	"	lda	#9\n" //X_SIZE / 2
//	"	sta	_i\n"
//	"	ldx	_map_adr\n"
	"	ldy	#9\n"
"_loop1:"
	"	ldu	_data\n"
	"	lda	,u\n"
	"	sta	_data_no\n"
	"	lsra\n"	// / 2
	"	lsra\n"
	"	lsra\n"
	"	lsra\n"
//	"	anda	#0x0f\n"
	"	ldb	_data_no\n"
	"	aslb\n"	// * 2
	"	aslb\n"
	"	aslb\n"
	"	aslb\n"
//	"	andb	#0xf0\n"
	"	sta	_i\n"
	"	orb	_i\n"
//	"	lda	#0\n"
	"	clra\n"
//	"	std	_pat_no\n"
	"	aslb\n"	// d * 2
	"	rola\n"
	"	aslb\n"
	"	rola\n"
	"	ldu	#0x5e80\n" // PARTS_DATA = 0x2280+0x3c00
	"	leau	d,u\n"
//	"	stx	_pat_adr\n"

//	"	ldx	_map_adr\n"
//	"	ldu	_pat_adr\n"
	"	ldb	,u+\n"
	"	stb	,x+\n"
//	"	leax	1,x\n"
//	"	stx	_map_adr\n"
//	"	leau	1,u\n"
//	"	stu	_pat_adr\n"

//	"	ldx	_map_adr\n"
//	"	ldu	_pat_adr\n"
	"	ldb	,u+\n"
	"	stb	,x\n"
//	"	leax	1,x\n"
//	"	stu	_pat_adr\n"
//	"	ldx	_map_adr\n"
	"	leax	31,x\n"
//	"	stx	_map_adr\n"

//	"	ldx	_map_adr\n"
//	"	ldu	_pat_adr\n"
	"	ldb	,u+\n"
	"	stb	,x+\n"
//	"	leax	1,x\n"
//	"	stx	_map_adr\n"
//	"	leau	1,u\n"
//	"	stx	_pat_adr\n"

//	"	ldx	_map_adr\n"
//	"	ldu	_pat_adr\n"
	"	ldb	,u\n"
	"	stb	,x\n"
//	"	ldx	_map_adr\n"
	"	leax	-31,x\n"
//	"	stx	_map_adr\n"

	"	ldu	_data\n"
	"	leau	1,u\n"
	"	stu	_data\n"

//	"	dec	_i\n"
	"	leay	-1,y\n"
	"	bne	_loop1\n"
//	"	bne	_loop11\n"

	"	ldu	_data_tmp\n"
	"	leau	128,u\n"	// MAP_SIZE_X
	"	stu	_data_tmp\n"
//	"	ldu	_data_tmp\n"
	"	stu	_data\n"
//	"	ldx	_map_adr\n"
	"	leax	46,x\n"		// (32 * 2 - X_SIZE)
//	"	stx	_map_adr\n"

	"	dec	_j\n"
	"	bne	_loop0\n"
//	"	bne	_loop00\n"
//	"	bra	_end0\n"
//"_loop00:\n"
//	"	jmp	_loop0\n"
//"_loop11:\n"
//	"	jmp	_loop1\n"

"_end0:\n"
);
}

unsigned char *vram_adr_tmp;
unsigned short old_x = 255, old_y = 255;
unsigned short x = 165, y = 30;
unsigned char old_map_data[(X_SIZE + 2) * 32];

int main(void)
{
	unsigned char k0, k1, k2, k3, st, st2;

	unsigned short xx, yy;
	unsigned char k;
//	unsigned char ii, jj;

	unsigned char sub_flag;

	unsigned char fadeflag = 0;

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}

	mem = (unsigned char *)0xfd12;
	mem[1] = 0x02;
	mem[0] = 0x40;	/* 多色モードにする */

	sub_disable();
	bank1_off();	/* 論理演算を切る */
	sub_enable();

	mem = (unsigned char *)0x6AFF;

	subcpu_flag = 0;

	for(no = 0; no < 30; ++no){
		unsigned short pat_tmp = (no & 0x0f) * 1 + (no & 0xf0) * 8;
		put_chrsub_pat(pat_tmp, no);
		subram_table[no] = (unsigned char *)SUBPAT_ADR2 + no * PARTS_X * PARTS_Y * 3;
	}

	/* ジョイスティック設定 */
asm(
	"	orcc	#0x10\n"
);
	write_opn(15, 0x3f);
	write_opn(7, 0xbf);
asm(
	"	andcc	#0xef\n"
);

	st3 = 0;

	keyscan_on();
	keyrepeat_off();
	set_key_irq();

	pal_allblack(0);
	key_clear();

	do{
/*		while(1){
			get_key();
			if(!keyflag)
				break;
			key_sense();
		}*/
		/* ジョイスティック読み込み */
asm(
	"	orcc	#0x10\n"
);
		write_opn(15, 0x2f);
		st = ~read_opn(14);
		st2 = st3;
asm(
	"	andcc	#0xef\n"
);

		if((st & 0x01) || (st2 & 0x01)){ /* U */
			if(y > 0){
				--y;
				dir = 0;
			}
		}
		if((st & 0x08) || (st2 & 0x08)){ /* R */
			if(x < (MAP_SIZE_X*2 - X_SIZE)){
				++x;
				dir = 1;
			}
		}
		if((st & 0x02) || (st2 & 0x02)){ /* D */
			if(y < (MAP_SIZE_Y*2 - Y_SIZE)){
				++y;
				dir = 2;
			}
		}
		if((st & 0x04) || (st2 & 0x04)){ /* L */
			if(x > 0){
				--x;
				dir = 3;
			}
		}
		if((st & 0x10) || (st2 & 0x10)) /* A */
			if((st & 0x20) || (st2 & 0x20)) /* B */
				break;

		if((old_x != x) || (old_y != y)){
			xx = 1 - x % 2;
			yy = 1 - y % 2;
			data = ((unsigned char *)MAP_ADR + (x / 2) + (y / 2) * MAP_SIZE_X);
			data_tmp = data;
			vram_adr = (unsigned char *)((OFS_X * PARTS_X + SIZE * PARTS_Y * OFS_Y));
			vram_adr_tmp = vram_adr;
			map_adr = &map_data[xx + yy * 32];

//			sub_disable();
//			bank1_on();
//			vram_b_on();

			map_sub();

//			bank1_off();
//			vram_off();
//			sub_enable();

			dir2 = 1 - dir2;
			map_adr = &map_data[1 + 1 * 32];
			old_map_adr = &old_map_data[1 + 1 * 32];

//			if((old_x != x)){
//				for(i = 1; i < (X_SIZE - 1); ++i){
//					for(j = 1; j < (Y_SIZE - 1); ++j){
/*						no = *map_adr;
						sub_flag = 0;
						if(i >= CHR_X){
							if(i <= (CHR_X+1)){
								if(j >= CHR_Y){
									if(j <= (CHR_Y+1)){
										sub_flag = 1;
									}
								}
							}
						}
						if(sub_flag){
							chr_sub();
						}else if(no != *old_map_adr){
							pat_sub();
							*old_map_adr = no;
						}
*/
asm(
	"	pshs u\n"
	"	ldx	_old_x\n"
	"	cmpx	_x\n"
	"	beq	_skip001\n"
//	"	jmp	_skip001\n"
//"_skip000:"
	"	lda	#1\n"
	"	ldx	_map_adr\n"
	"	ldu	_old_map_adr\n"
	"	ldy	_vram_adr\n"
"_skip00:"
	"	sta	_i\n"
	"	ldb	#1\n"
//	"	ldx	_map_adr\n"
"_skip01:"
	"	stb	_j\n"
	"	pshs	d,x,y,u\n"
//	"	ldx	_map_adr\n"
	"	lda	,x\n"
	"	sta	_no\n"
	"	lda	_i\n"
	"	cmpa	#7\n"
	"	bls	_skip1\n"
	"	cmpa	#9\n"
	"	bhi	_skip1\n"
//	"	ldb	_j\n"
	"	cmpb	#7\n"
	"	bls	_skip1\n"
	"	cmpb	#9\n"
	"	bhi	_skip1\n"
	"	sty	_vram_adr\n"
//	"	pshs	x,y,u\n"
	"	jsr	_chr_sub\n"
//	"	puls	x,y,u\n"
	"	bra	_skip11\n"
"_skip1:\n"
//	"	ldu	_old_map_adr\n"
	"	lda	_no\n"
	"	cmpa	,u\n"
	"	beq	_skip11\n"
//	"	lda	_no\n"
	"	sta	,u\n"
	"	sty	_vram_adr\n"
//	"	pshs	x,y,u\n"
	"	jsr	_pat_sub\n"
//	"	puls	x,y,u\n"

"_skip11:\n"
	"	puls	d,x,y,u\n"
//	"	ldy	_vram_adr\n"
	"	leay	320,y\n"	// SIZE * PARTS_Y
//	"	sty	_vram_adr\n"
//	"	ldx	_map_adr\n"
	"	leax	32,x\n"
//	"	stx	_map_adr\n"
//	"	ldu	_old_map_adr\n"
	"	leau	32,u\n"
//	"	stu	_old_map_adr\n"

//	"	lda	_j\n"
	"	incb\n"
//	"	stb	_j\n"
	"	cmpb	#17\n"	// X_SIZE - 1
	"	bne	_skip01\n"

	"	ldy	_vram_adr_tmp\n"
	"	leay	1,y\n"	// PARTS_X
	"	sty	_vram_adr_tmp\n"
//	"	sty	_vram_adr\n"
//	"	ldx	_map_adr\n"
	"	leax	-511,x\n" // (1 - 32 * (X_SIZE - 2))
//	"	stx	_map_adr\n"
//	"	ldu	_old_map_adr\n"
	"	leau	-511,u\n" // (1 - 32 * (X_SIZE - 2))
//	"	stu	_old_map_adr\n"

//	"	lda	_i\n"
	"	inca\n"
//	"	sta	_i\n"
	"	cmpa	#17\n"	// Y_SIZE - 1
	"	bne	_skip00\n"
//	"	beq	_skip2\n"
//	"	jmp	_skip00\n"
"_skip2:\n"
	"	jmp	_skip5\n"
);
/*						vram_adr += SIZE * PARTS_Y;
						map_adr += 32;
						old_map_adr += 32;
					}
					vram_adr_tmp += PARTS_X;
					vram_adr = vram_adr_tmp;
					map_adr += (1 - 32 * (X_SIZE - 2));
					old_map_adr += (1 - 32 * (X_SIZE - 2));
				}*/
/*			}
			else
			{*/
//				for(j = 1; j < (Y_SIZE - 1); ++j){
//					for(i = 1; i < (X_SIZE - 1); ++i){
/*						no = *map_adr;
						sub_flag = 0;
						if(i >= CHR_X){
							if(i <= (CHR_X+1)){
								if(j >= CHR_Y){
									if(j <= (CHR_Y+1)){
										sub_flag = 1;
									}
								}
							}
						}
						if(sub_flag){
							chr_sub();
						}else if(no != *old_map_adr){
							pat_sub();
							*old_map_adr = no;
						}*/
asm(
"_skip001:"
	"	lda	#1\n"
	"	ldx	_map_adr\n"
	"	ldu	_old_map_adr\n"
	"	ldy	_vram_adr\n"
"_skip02:"
	"	sta	_j\n"
	"	ldb	#1\n"
"_skip03:"
	"	stb	_i\n"
	"	pshs	d,x,y,u\n"
//	"	ldx	_map_adr\n"
	"	lda	,x\n"
	"	sta	_no\n"
//	"	ldb	_i\n"
	"	cmpb	#7\n"
	"	bls	_skip3\n"
	"	cmpb	#9\n"
	"	bhi	_skip3\n"
	"	lda	_j\n"
	"	cmpa	#7\n"
	"	bls	_skip3\n"
	"	cmpa	#9\n"
	"	bhi	_skip3\n"
	"	sty	_vram_adr\n"
//	"	pshs	x,y,u\n"
	"	jsr	_chr_sub\n"
//	"	puls	x,y,u\n"
	"	bra	_skip4\n"
"_skip3:\n"
//	"	ldu	_old_map_adr\n"
	"	lda	_no\n"
	"	cmpa	,u\n"
	"	beq	_skip4\n"
//	"	lda	_no\n"
	"	sta	,u\n"
	"	sty	_vram_adr\n"
//	"	pshs	x,y,u\n"
	"	jsr	_pat_sub\n"
//	"	puls	x,y,u\n"
"_skip4:\n"
	"	puls	d,x,y,u\n"
////	"	ldy	_vram_adr\n"
	"	leay	1,y\n"	// PARTS_X
//	"	sty	_vram_adr\n"
//	"	ldx	_map_adr\n"
	"	leax	1,x\n"
//	"	stx	_map_adr\n"
//	"	ldu	_old_map_adr\n"
	"	leau	1,u\n"
//	"	stu	_old_map_adr\n"

//	"	ldb	_i\n"
	"	incb\n"
//	"	stb	_i\n"
	"	cmpb	#17\n"	// X_SIZE - 1
	"	bne	_skip03\n"

//	"	ldy	_vram_adr\n"
	"	leay	304,y\n"	// (SIZE * PARTS_Y - (PARTS_X) * (X_SIZE - 2))
//	"	sty	_vram_adr\n"
//	"	ldx	_map_adr\n"
	"	leax	16,x\n" // (32 - (X_SIZE - 2))
//	"	stx	_map_adr\n"
//	"	ldu	_old_map_adr\n"
	"	leau	16,u\n" // (32 - (X_SIZE - 2))
//	"	stu	_old_map_adr\n"

//	"	lda	_j\n"
	"	inca\n"
//	"	sta	_j\n"
	"	cmpa	#17\n"	// Y_SIZE - 1
	"	bne	_skip02\n"
//	"	beq	_skip5\n"
//	"	jmp	_skip02\n"
"_skip5:\n"
	"	puls u\n"
);
/*						vram_adr += PARTS_X;
						++map_adr;
						++old_map_adr;
					}
					vram_adr += (SIZE * PARTS_Y - (PARTS_X) * (X_SIZE - 2));
					map_adr += (32 - (X_SIZE - 2));
					old_map_adr += (32 - (X_SIZE - 2));
				}
*///		}
		}
		old_x = x;
		old_y = y;

		if(fadeflag == 0){
			wait_vsync();
			fadeflag = 1;
			sub_disable();
			vram_off();
asm(
	"	orcc	#0x10\n"
	"	clr	0xfd93\n"
);
			fadeinblack(org_pal, 0, 3);
//			pal_all(0, org_pal);
//			set_constrast(0, org_pal, 0);
asm(
	"	lda	#0xc0\n"
	"	sta	0xfd93\n"
	"	andcc	#0xEF\n"
);
			sub_enable();
			key_clear();
		}
	}while((*keyport != 0x01) && (*keyport != 0x5c));	/* ESC */
	vram_off();
	set_key_irq();
	keyrepeat_on();
	keyscan_off();
	sub_disable();
asm(
	"	orcc	#0x10\n"
	"	clr	0xfd93\n"
);
	fadeoutblack(org_pal, 0, 3);
	pal_all(0, org_pal);
asm(
	"	lda	#0xc0\n"
	"	sta	0xfd93\n"
	"	andcc	#0xEF\n"
);
	sub_enable();

	key_clear();

	mem = (unsigned char *)0xfd12;
	mem[1] = 0x01;
	mem[0] = 0x00;	/* 8色2画面モードにする */

	return 0;
}

