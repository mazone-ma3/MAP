/* PC-88 CHR88.BAS作成キャラMAP移動実験 ただのローダー */
/* 転送部インラインアセンブラ化 */

#include <stdio.h>
#include <stdlib.h>

#define MAINRAM_ADR 0x2000
#define VRAM_DATA_ADR 0xc700

void  put_chr88_pat2(unsigned char *mainram, unsigned char *vram)
{
#asm
	ld	hl, 2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)	; de=vram
	inc	hl
	ld	c,(hl)
	inc	hl
	ld	b,(hl)	; bc = mainram
	ld	l,c
	ld	h,b	; hl = mainram

	ld	b,8
;	ld	c,0

looppat:
	DI
	ld	a,($e6c2)
	or	$02
	out	($31),a		; RAM Mode

	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	de
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	de
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	de

	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	de
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	de
	ld	a,(hl)
	ld	(de),a

	out	($5f),a			; Main RAM

	ld	a,($e6c2)		; ROM Mode
	out($31),a
	EI

	push	de	;	hl = mainram source
	ld	de, 91 ;((256 / 8 - 2) * 3 + 1)
	add	hl,de
	pop	de

	push	hl	;de = mainram data
	ex	de,hl
	ld	de,91	;((256 / 8 - 2) * 3 + 1)
	add	hl,de
	ex	de,hl
	pop	hl

	djnz looppat

#endasm
}

void main(void)
{
	unsigned char no = 0;
	unsigned char x = 0;
	unsigned short pat_tmp;

	for(x = 0; x < 32; ++x){
		pat_tmp = ((no % 16 * 2) + ((no / 16) * 256));
		pat_tmp = pat_tmp * 2 + pat_tmp;
		put_chr88_pat2((unsigned char *)(MAINRAM_ADR + pat_tmp), (unsigned char *)(VRAM_DATA_ADR + pat_tmp));
		++no;
	}
}

