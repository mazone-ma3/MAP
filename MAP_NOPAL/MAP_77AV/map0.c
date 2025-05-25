/* CMOC FM77AVマップ表示テスト By m@3 */
/* キャラMAP移動実験 2x2圧縮展開 */
/* サブCPU止めてVRAMアクセス メインに1プレーン16K全展開する */
/* キャラを出す SUB CPUコマンド版 */

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

#define VRAM_DATA_ADR (PAGE * 0x1000)
#define MAP_ADR VRAM_DATA_ADR
#define PARTS_DATA (MAP_ADR+0x3c00)
#define MASK_DATA_ADR 0x1600
#define MAPPAT_ADR 0x2000
#define CHRPAT_ADR 0x4000

#define SIZE 80

#define X_SIZE 18
#define Y_SIZE 18

#define PARTS_X 2
#define PARTS_Y 8

#define MAP_SIZE_X 128
#define MAP_SIZE_Y 128

#define OFS_X 2
#define OFS_Y 2

#define CHR_X 8
#define CHR_Y 8

unsigned char vram_tmp[PARTS_X * PARTS_Y * 3];
unsigned char map_data[(X_SIZE+2) * 32];

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
unsigned char dir = 0, dir2 = 0;
unsigned short i, j;


unsigned char *mmr;
//unsigned char *vram;
unsigned char *mem;
unsigned char *msr;
unsigned char msr_sv;
unsigned char *keyport, key, st3;

#define OPNCOM 0xFD15
#define OPNDAT 0xFD16

unsigned char *opncom;
unsigned char *opndat;

void keyscan_on(void)
{
asm{
	JMP	SCAN
}

asm{
SUBOUT:
	FCB	16
	RMB	1
	FDB	KEYCON
	FDB	6
	RMB	2
KEYCON:
	RMB	2
	FCB	$45
	FCB	$00
	FCB	$02
	RMB	1

SCAN:
	LDX	#SUBOUT
	JSR [$FBFA]
	BCS	ERROR
	RTS
ERROR:
	RTS
}
}

void keyscan_off(void)
{
asm{
	bsr	SCAN2
	LDA #0
	STA KEYMODE
	BRA	ENDOFF
}
asm{
SUBOUT2:
	FCB	16
	RMB	1
	FDB	KEYCON2
	FDB	6
	RMB	2
KEYCON2:
	RMB	2
	FCB	$45
	FCB	$00
KEYMODE:
	FCB	$00
	RMB	1

SCAN2:
	LDX	#SUBOUT2
	JSR [$FBFA]
	BCS	ERROR2
	RTS
ERROR2:
	RTS

ENDOFF:
}
}

void keyrepeat_on(void)
{
asm{
	jmp	_repeat
_SUBOUT3:
	fcb	16
	rmb	1
	fdb	_KEYREP
	fdb	6
	fcb	2
_KEYREP:
	rmb	2
	fcb	0x45
	fcb	4
	fcb	0
	rmb	1
_repeat:
	ldx	#_SUBOUT3
	jsr [0xFBFA]
}
}

void keyrepeat_off(void)
{
asm{
	jmp	_repeat2
_SUBOUT4:
	fcb	16
	rmb	1
	fdb	_KEYREP2
	fdb	6
	fcb	2
_KEYREP2:
	rmb	2
	fcb	0x45
	fcb	4
	fcb	1
	rmb	1
_repeat2:
	ldx	#_SUBOUT4
	jsr [0xFBFA]
}
}

unsigned char keycode, keyflag;

void  key_sense(void)
{
//	unsigned char keycode = 0, old_keycode = 0;
//	while((keycode = *keyport) != old_keycode){
		keycode = *keyport;
		if((keycode == 0x4d) || (keycode == 0x3b)){
			BITSET(0, st3);
			BITCLR(1, st3);
		}else if((keycode == 0xcd) || (keycode == 0xbb)){
			BITCLR(0, st3);
		}else if((keycode == 0x51) || (keycode == 0x40)){
			BITSET(3, st3);
			BITCLR(2, st3);
		}else if((keycode == 0xd1) || (keycode == 0xc0)){
			BITCLR(3, st3);
		}else if((keycode == 0x50) || (keycode == 0x43)){
			BITSET(1, st3);
			BITCLR(0, st3);
		}else if((keycode == 0xD0) || (keycode == 0xc3)){
			BITCLR(1, st3);
		}else if((keycode == 0x4f) || (keycode == 0x3e)){
			BITSET(2, st3);
			BITCLR(3, st3);
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
asm{
_TINIT:
	orcc	#$10
	ldx		0xFFF8
	cmpx	#_IRQET
	beq		_TCLOSE
	stx		_IRQJP
	ldx		#_IRQET
	stx		0x0FFF8
	bra	_ENDIRQ
_TCLOSE:
	ldx		_IRQJP
	stx		0xFFF8
	bra	_ENDIRQ

_IRQET:
	pshs	D,X
	lda		0xFD03
	anda	#1
	beq		_IRQRT

	jsr	_key_sense

_IRQRT:
	puls	D,X
	jmp		[_IRQJP]

_IRQJP:	rmb	2
_TCOUNT:	rmb	1
_ENDIRQ:
	andcc	#$EF

}

}

void sub_disable(void)
{
asm{
_SUBHLT:
	lda	$FD05
	bmi	_SUBHLT
	orcc	#$50
	lda	#$80
	sta	$FD05
	lda	$FD05
	bpl	*-3
}

//	msr_sv = *msr;
asm{
	lda	0xfd93
	sta	_msr_sv
	ora	#0x80
	sta	0xfd93
}
//	*msr |= 0x80;
}

void sub_enable(void)
{
//	*msr = msr_sv;

asm{
	lda	_msr_sv
	sta	0xfd93

	ldb	$FC80
	orb	#$80
	stb	$FC80

	clr	$FD05
	andcc	#$AF
}
}

void bank1_on()
{
asm{
	lda	#0x1d
	sta	0xfd8a
	lda	#0
	sta	0xa410
	lda	#0x20
	sta	0xa430
}
/*	mmr[PAGE] = 0x1d;
	vram[0x410] = 0;
	vram[0x430] = 0x20;
*/
}

void bank1_off(void)
{
asm{
	lda	#0x1d
	sta	0xfd8a
	lda	#0
	sta	0xa410
	sta	0xa430
}
/*	mmr[PAGE] = 0x1d;
	vram[0x410] = 0;
	vram[0x430] = 0x00;*/
}

void vram_b_on(void)
{
asm{
	lda	#0x10
	sta	0xfd8a
	inca
	sta	0xfd8b
	inca
	sta	0xfd8c
	inca
	sta	0xfd8d
}
/*	mmr[PAGE] = 0x10;
	mmr[PAGE+1] = 0x11;
	mmr[PAGE+2] = 0x12;
	mmr[PAGE+3] = 0x13;*/
}

void vram_r_on(void)
{
asm{
	lda	#0x14
	sta	0xfd8a
	inca
	sta	0xfd8b
	inca
	sta	0xfd8c
	inca
	sta	0xfd8d
}
/*	mmr[PAGE] = 0x14;
	mmr[PAGE+1] = 0x15;
	mmr[PAGE+2] = 0x16;
	mmr[PAGE+3] = 0x17;*/
}

void vram_g_on(void)
{
asm{
	lda	#0x18
	sta	0xfd8a
	inca
	sta	0xfd8b
	inca
	sta	0xfd8c
	inca
	sta	0xfd8d
}
/*	mmr[PAGE] = 0x18;
	mmr[PAGE+1] = 0x19;
	mmr[PAGE+2] = 0x1a;
	mmr[PAGE+3] = 0x1b;*/
}

void vram_off(void)
{
asm{
	lda	#0x3a
	sta	0xfd8a
	inca
	sta	0xfd8b
	inca
	sta	0xfd8c
	inca
	sta	0xfd8d
}
/*	mmr[PAGE] = 0x30+PAGE;
	mmr[PAGE+1] = 0x31+PAGE;
	mmr[PAGE+2] = 0x32+PAGE;
	mmr[PAGE+3] = 0x33+PAGE;*/
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

unsigned char *adr_tmp, *adr_tmp3;
unsigned short *adr_tmp2;
unsigned char ii, jj;

unsigned char subcpu_flag = 0;

void sub_draw(void)
{
asm{
	lda	#$3f
	sta	$fc82
	lda	_subcpu_flag
	bne	SKIP1
	lda	#1
	sta	_subcpu_flag
	jsr	MOVCMD
SKIP1:
	jsr	SUBMOV
	bra	ENDCD

SUBHLT:
	lda	$fd05
	bmi	*-3
	orcc	#$50
	lda	#$80
	sta	$fd05
	lda	$fd05
	bpl	*-3
	rts

RDYREQ:
	lda	$fc80
	ora	#$80
	sta	$fc80
	rts

SUBMOV:
	clra
	sta	$fd05
	andcc	#$af
	rts

MOVCMD:
	ldx	#TESTCD
	ldy	#$fc82
LOOP1:
	lda	,X+
	sta	,Y+
	cmpx	#ENDCD
	bne	LOOP1
	rts

TESTCD:
	fcb	$3f
	rmb	8
	fcb	$93
	fdb	$d38f
	fcb	$90

	ldy	$d3f8
	ldx	#$d3c8
	lda	#8
	sta	$d3fa

TESTLP:
	lda ,x
	ldb	3,x
	std $0000,y
	lda 1,x
	ldb	4,x
	std $4000,y
	lda 2,x
	ldb	5,x
	std $8000,y

	leax 6,x
	leay 80,y

	dec	$d3fa
	bne	TESTLP

	rts

ENDCD:
;	equ	*
}
}

/* RAM テンポラリ->VRAM 3プレーン転送(SUB CPU) */
void put_sub(void)
{
	adr_tmp = ((unsigned char *)vram_tmp);
	adr_tmp2 = (unsigned short *)(vram_adr);

asm{
	jsr	SUBHLT

	ldx	_adr_tmp
	ldy	#$fcc8
LOOP0:
	ldd	,X++
	std	,Y++
	cmpy	#$fcc8+48
	bne	LOOP0

	ldd	_adr_tmp2
	std	$fcf8
}
	sub_draw();
}

/* メインRAM->VRAM 3プレーン転送(SUB CPU) */
void put_sub2(unsigned short patadr)
{
	adr_tmp = (unsigned char *)MAPPAT_ADR + patadr * 2 + patadr; 
	adr_tmp2 = (unsigned short *)(vram_adr);

asm{
	jsr	SUBHLT

	ldx	_adr_tmp
	ldy	#$fcc8
LOOP00:
	ldd	,X
	std	,Y++
	ldd	2,X
	std	,Y++
	ldd	4,X
	std	,Y++

	leax 96,x

	cmpy	#$fcc8+48
	bne	LOOP00

	ldd	_adr_tmp2
	std	$fcf8
}
	sub_draw();
}

/* RAM テンポラリ->VRAM 3プレーン転送 */
/*void copy_chr_fm_pat(void)
{
	unsigned char jj;

	adr_tmp = ((unsigned char *)vram_tmp);
	adr_tmp2 = (unsigned short *)vram_adr;

//	for(jj = 0 ; jj < PARTS_Y; ++jj){
//		sub_disable();

asm{
	ldx _adr_tmp
	ldy _adr_tmp2

	lda	#8
	sta	_jj
loop_pat:
;	jsr _sub_disable
}
//		vram_b_on();
asm{
	lda	#0x10
	sta	0xfd8a
	inca
	sta	0xfd8b
	inca
	sta	0xfd8c
	inca
	sta	0xfd8d
}
//		*adr_tmp2 = (unsigned short)(adr_tmp[0] * 256 | adr_tmp[3]);
asm{
	lda ,x
	ldb 3,x
	std ,y
}
//		vram_r_on();
asm{
	lda	#0x14
	sta	0xfd8a
	inca
	sta	0xfd8b
	inca
	sta	0xfd8c
	inca
	sta	0xfd8d
}
//		*adr_tmp2 = (unsigned short)(adr_tmp[1] * 256 | adr_tmp[4]);
asm{
	lda 1,x
	ldb 4,x
	std ,y
}
//		vram_g_on();
asm{
	lda	#0x18
	sta	0xfd8a
	inca
	sta	0xfd8b
	inca
	sta	0xfd8c
	inca
	sta	0xfd8d
}
//		*adr_tmp2 = (unsigned short)(adr_tmp[2] * 256 | adr_tmp[5]);
asm{
	lda 2,x
	ldb 5,x
	std ,y

;	jsr	_sub_enable
}
//		sub_enable();
//		adr_tmp +=(PARTS_X * 3);
//		adr_tmp2 += (SIZE / 2);
asm{
	leax 6,x
	leay 80,y

	dec _jj
	bne loop_pat
}*/
//	}

//		vram_off();
/*asm{
	lda	#0x3a
	sta	0xfd8a
	inca
	sta	0xfd8b
	inca
	sta	0xfd8c
	inca
	sta	0xfd8d
}*/
//}

/* メインRAM->RAM テンポラリ 3プレーンMASK転送 AND */
void put_chr_fm_pat_chr_and(unsigned short patadr)
{
//	unsigned char *adr_tmp, *adr_tmp2, data_tmp;
//	unsigned char ii,jj;

	adr_tmp3 = ((unsigned char *)vram_tmp);
	adr_tmp = ((unsigned char *)MASK_DATA_ADR + (patadr));
//	for(jj = 0 ; jj < PARTS_Y; ++jj){
//		for(ii = 0 ; ii < PARTS_X; ++ii){
//			adr_tmp = ((unsigned char *)MASK_DATA_ADR + (patadr + (ii + jj * 32)));
//			data_tmp =  ~(*(adr_tmp));
asm{
	ldx _adr_tmp
	ldy _adr_tmp3

	lda	#8
	sta	_ii
loop_and1:
	lda	#2
	sta	_jj
loop_and2:
}
/*			*(adr_tmp3) &= data_tmp;
			*(adr_tmp3+1) &= data_tmp;
			*(adr_tmp3+2) &= data_tmp;
*/
asm{
	lda ,x
	coma
	tfr a,b

	anda ,y
	sta ,y

	tfr b,a
	anda 1,y
	sta 1,y

	tfr b,a
	anda 2,y
	sta 2,y
}
//			adr_tmp3 += 3;
/*		}
		adr_tmp += 30; //(32 - PARTS_X);
	}*/
asm{
	leax 1,x
	leay 3,y

	dec _jj
	bne loop_and2

	leax 30,x

	dec _ii
	bne loop_and1
}
}

/* メインRAM->RAM テンポラリ 3プレーン転送 OR */
void put_chr_fm_pat_chr_or(unsigned short patadr)
{
//	unsigned char *adr_tmp, *adr_tmp2;
//	unsigned char ii,jj;

	adr_tmp = ((unsigned char *)CHRPAT_ADR + (patadr * 2 + patadr));
	adr_tmp3 = ((unsigned char *)vram_tmp);
//	for(jj = 0 ; jj < PARTS_Y; ++jj){
//		for(ii = 0 ; ii < PARTS_X; ++ii){
asm{
	ldx _adr_tmp
	ldy _adr_tmp3

	lda	#8
	sta	_ii
loop_or1:
	lda	#2
	sta	_jj
loop_or2:
}
/*			*(adr_tmp3) |= *(adr_tmp);
			*(adr_tmp3+1) |= *(adr_tmp+1);
			*(adr_tmp3+2) |= *(adr_tmp+2);
*/
asm{
	ldd	,x
	ora ,y
	orb 1,y
	std ,y

	lda 2,x
	ora 2,y
	sta 2,y
}
/*			adr_tmp += 3;
			adr_tmp3 += 3;
		}
		adr_tmp += (32 * 3 - PARTS_X * 3);
	}*/
asm{
	leax 3,x
	leay 3,y

	dec _jj
	bne loop_or2

	leax 90,x

	dec _ii
	bne loop_or1
}
}

/* メインRAM->RAM テンポラリ 3プレーン転送 */
void put_chr_fm_pat_map(unsigned short patadr)
{
//	unsigned char *adr_tmp, *adr_tmp2;
//	unsigned char ii,jj;

	adr_tmp = ((unsigned char *)MAPPAT_ADR + (patadr * 2 + patadr));
	adr_tmp3 = ((unsigned char *)vram_tmp);
//	for(jj = 0 ; jj < PARTS_Y; ++jj){
//		for(ii = 0 ; ii < PARTS_X; ++ii){
asm{
	ldx _adr_tmp
	ldy _adr_tmp3

	lda	#8
	sta	_ii
loop_map1:
	lda	#2
	sta	_jj
loop_map2:
}
/*			*(adr_tmp3) = *(adr_tmp);
			*(adr_tmp3+1) = *(adr_tmp+1);
			*(adr_tmp3+2) = *(adr_tmp+2);
*/
asm{
	ldd ,x
	std ,y
	lda 2,x
	sta 2,y
}
/*			adr_tmp += 3;
			adr_tmp3 += 3;
		}
		adr_tmp += (32 * 3 - PARTS_X * 3);
	}*/
asm{
	leax 3,x
	leay 3,y

	dec _jj
	bne loop_map2

	leax 90,x

	dec _ii
	bne loop_map1
}
}

/* メインRAM->VRAM 3プレーン転送 */
/*void put_chrfm_pat(unsigned short patadr)
{

	adr_tmp = (unsigned char *)MAPPAT_ADR + patadr * 2 + patadr; 
	adr_tmp2 = (unsigned short *)vram_adr;

//	for(jj = 0 ; jj < PARTS_Y; ++jj){
//		sub_disable();
//		vram_b_on();
//		*adr_tmp2 = (unsigned short)(adr_tmp[0] * 256 | adr_tmp[3]);
asm{
	ldx _adr_tmp
	ldy _adr_tmp2
	lda	#8
	sta _jj

loop_pt2:
;	jsr _sub_disable

	lda	#0x10
	sta	0xfd8a
	inca
	sta	0xfd8b
	inca
	sta	0xfd8c
	inca
	sta	0xfd8d

	lda ,x
	ldb 3,x
	std ,y
}
//		vram_r_on();
asm{
	lda	#0x14
	sta	0xfd8a
	inca
	sta	0xfd8b
	inca
	sta	0xfd8c
	inca
	sta	0xfd8d
}
//		*adr_tmp2 = (unsigned short)(adr_tmp[1] * 256 | adr_tmp[4]);
asm{
	lda 1,x
	ldb 4,x
	std ,y
}
//		vram_g_on();
asm{
	lda	#0x18
	sta	0xfd8a
	inca
	sta	0xfd8b
	inca
	sta	0xfd8c
	inca
	sta	0xfd8d
}
//		*adr_tmp2 = (unsigned short)(adr_tmp[2] * 256 | adr_tmp[5]);
asm{
	lda 2,x
	ldb 5,x
	std ,y
}

//		sub_enable();
//		adr_tmp += (32 * 3);
//		adr_tmp2 += ((SIZE) / 2);
asm{
;	jsr _sub_enable

	leax 96,x
	leay 80,y

	dec _jj
	bne loop_pt2
}*/
//	}
//		vram_off();
/*asm{
	lda	#0x3a
	sta	0xfd8a
	inca
	sta	0xfd8b
	inca
	sta	0xfd8c
	inca
	sta	0xfd8d
}*/
//}

void pat_sub(void)
{
/*	sub_disable();
	put_chrfm_pat((no % 16 * 2) + (no / 16) * 256);
	sub_enable();*/
	put_sub2((no % 16 * 2) + (no / 16) * 256);
}

void chr_sub(void)
{
	unsigned short pat_tmp2;

	unsigned char no2 = chr_tbl[dir * 2 + dir2][(i - CHR_X) + (j - CHR_Y) * 2];

	pat_tmp2 = ((no2 % 16 * 2) + (no2 / 16) * 256);

	put_chr_fm_pat_map((no % 16 * 2) + (no / 16) * 256);
	/* マスクを書く */
	put_chr_fm_pat_chr_and(pat_tmp2);
	put_chr_fm_pat_chr_or(pat_tmp2);
/*	sub_disable();
	copy_chr_fm_pat();
	sub_enable();*/
	put_sub();
}

void get_key(void)
{
asm{
	bra	key1
keyin:
	fcb	21
	rmb	1
	fdb	keydt
	rmb	2
	rmb	2
keydt:
	rmb	2
key1:
	ldx	#keyin
	jsr	[0xfbfa]
	bcs	error
	lda	keydt+1
	sta	_keyflag
	lda	keydt
	sta	_keycode
	bra	end
error:
	lda	#0
	sta	_keyflag
end:
}
}

void key_clear(void)
{
	while(1){
		get_key();
		if(!keyflag)
			break;
	}
}

int main(void)
{
	unsigned char k0, k1, k2, k3, st, st2, data_no;
	unsigned char *data, *data_tmp;

	unsigned short pat_no;
	unsigned char *pat_adr;

	unsigned short x = 165, y = 30,xx, yy, old_x = 255, old_y = 255;
	unsigned char k;
//	unsigned short ii, jj;
	unsigned char *vram_adr_tmp;

	unsigned char old_map_data[(X_SIZE + 2) * 32];
	unsigned char sub_flag;
	unsigned char *map_adr;
	unsigned char *old_map_adr;

	dir = 2;

	for(i = 0; i < X_SIZE; ++i){
		for(j = 0; j < Y_SIZE; ++j){
			map_data[i + j * 32] = 0; //255-5;
			old_map_data[i +  j * 32] = 255-5;
		}
	}

	mmr = (unsigned char *)0xFD80;
//	vram = (unsigned char *)VRAM_DATA_ADR;
	mem = (unsigned char *)0x6AFF;
	msr = (unsigned char *)0xFD93;
	keyport = (unsigned char *)0xFD01;

	opncom = OPNCOM;
	opndat = OPNDAT;
	subcpu_flag = 0;

	/* ジョイスティック設定 */
asm{
	orcc	#$10
}
	write_opn(15, 0x3f);
	write_opn(7, 0xbf);
asm{
	andcc	#$EF
}

	st3 = 0;

	keyscan_on();
	keyrepeat_off();
	set_key_irq();

	key_clear();

	do{
/*		while(1){
			get_key();
			if(!keyflag)
				break;
			key_sense();
		}*/
		/* ジョイスティック読み込み */
asm{
	orcc	#$10
}
		write_opn(15, 0x2f);
		st = ~read_opn(14);
		st2 = st3;
asm{
	andcc	#$EF
}

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

			sub_disable();
			bank1_on();
			vram_b_on();

			for(j = 0; j < Y_SIZE / 2; ++j){

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

/*					map_data[ii][jj] = *(pat_adr++);
					map_data[ii+1][jj] = *(pat_adr++);
					map_data[ii][jj+1] = *(pat_adr++);
					map_data[ii+1][jj+1] = *(pat_adr);
*/					++data;
				}
				data_tmp += MAP_SIZE_X;
				data = data_tmp;
				map_adr += (32 * 2 - X_SIZE);
			}

			bank1_off();
//			vram_off();
			sub_enable();

			dir2 = 1 - dir2;
			map_adr = &map_data[1 + 1 * 32];
			old_map_adr = &old_map_data[1 + 1 * 32];

			if((old_x != x)){
				for(i = 1; i < (X_SIZE - 1); ++i){
					for(j = 1; j < (Y_SIZE - 1); ++j){
						no = *map_adr;
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
						vram_adr += SIZE * PARTS_Y;
						map_adr += 32;
						old_map_adr += 32;
					}
					vram_adr_tmp += PARTS_X;
					vram_adr = vram_adr_tmp;
					map_adr += (1 - 32 * (X_SIZE - 2));
					old_map_adr += (1 - 32 * (X_SIZE - 2));
				}
			}
			else
			{
				for(j = 1; j < (Y_SIZE - 1); ++j){
					for(i = 1; i < (X_SIZE - 1); ++i){
						no = *map_adr;
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
						vram_adr += PARTS_X;
						++map_adr;
						++old_map_adr;
					}
					vram_adr += (SIZE * PARTS_Y - (PARTS_X) * (X_SIZE - 2));
					map_adr += (32 - (X_SIZE - 2));
					old_map_adr += (32 - (X_SIZE - 2));
				}
			}
		}
		old_x = x;
		old_y = y;
	}while((*keyport != 0x01) && (*keyport != 0x5c));	/* ESC */

	vram_off();
	set_key_irq();
	keyrepeat_on();
	keyscan_off();
	key_clear();
	return 0;
}

