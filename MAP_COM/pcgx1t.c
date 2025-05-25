/* z88dk X1turboZ  PCGキャラ表示実験 ZSDCC版 By m@3 */
/* .com版 */
/* キャラを出す  */

#include <stdio.h>
#include <stdlib.h>

#define PARTS_SIZE 0x1800
unsigned char pcg_data[PARTS_SIZE];

#define SIZE 80

FILE *stream[2];

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

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

/* turbo専用PCG設定 試験に出るX1ほぼまんま */
/* データを8バイトx256パターンx3プレーン=6144バイト設定 */
void set_pcgt(unsigned char *mainram)
{
__asm
	ld	hl, 2
	add	hl, sp
	ld	c, (hl)
	inc	hl
	ld	b, (hl)	; bc=mainram

	ld	l,c
	ld	h,b	; hl = mainram

;	ld	hl,#0x0d000	;data head

	ld	a,0	;256
	ld	c,0		;port low 0
	ld	b,0x15	;port high 0x15,0x16,0x17
	call	loop

	ld	a,0	;256
	ld	c,0		;port low 0
	ld	b,0x16
	call	loop

	ld	a,0	;256
	ld	c,0		;port low 0
	ld	b,0x17
	call	loop


;	ld	a,0x00
;	ld	bc,#0x1fd0 ;+#0x100	;0x23
;	out	(c),a

	xor	a
	ld	bc,#0x3fff ;+#0x100	;0x00
	out	(c),a

	ld	a,0x00
	ld	bc,#0x27ff ;+#0x100	;0x20
	out	(c),a

	xor	a

	ld	bc,#0x37ff ;+#0x100	;0x00
	out	(c),a

	jmp end

loop:
	push	af
	push	bc

	push	af

	ld	a,0x23	;0x23
	ld	bc,#0x1fd0 ;+#0x100	;0x23
	out	(c),a

	xor	a
	ld	bc,#0x3fff ;+#0x100	;0x00
	out	(c),a

	ld	a,0x20
	ld	bc,#0x27ff ;+#0x100	;0x20
	out	(c),a

	pop	af
	neg

	ld	bc,#0x37ff ;+#0x100	;0x00
	out	(c),a

	pop	bc
	push	bc

	ld	d,8	;counter 8bytes
	ld	e,2	;add 2

	call	write

	pop	bc
	pop	af
	dec	a
	jr nz,loop

	ret

write:
	inc	b
	outi
	ld	a,c
	add	a,e
	ld	c,a
	dec	d
	jp	nz,write
	ret

end:

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

unsigned char no;
unsigned short vram_ofs;
unsigned char i, j;


int	main(int argc,char **argv)
{
	for(j = 0; j < 8; ++j){
		for(i = 0; i < 32; ++i){
			no = i + j * 32;
			vram_ofs = i + j * SIZE;
			outp(0x3000 + vram_ofs, no);
			outp(0x2000 + vram_ofs, 0x27);
		}
	}

	if (argc >= 2){ //argv[1] == NULL){
		if(bload(argv[1], pcg_data, PARTS_SIZE))
			return ERROR;
		set_pcgt(pcg_data);
	}
	if((inp(0x1ff0) & 0x01))
		outp(0x1fd0, 0x00);

	return NOERROR;
}

