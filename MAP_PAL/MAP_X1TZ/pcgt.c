/* turbo専用PCG設定 pcgt.c 試験に出るX1ほぼまんま */
/* $0d000にデータを8バイトx256パターンx3プレーン=6144バイト設定 */
/* 最後の画面設定は本当はどうしたらいいか分からない */

/* outi */
/*  (C) ← (HL) */
/*  B ← B - 1 */
/*  HL ← HL + 1 */

void main(void)
{
#asm

	ld	hl,$0d000	;data head

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


	ld	a,0x00
	ld	bc,$1fd0 ;+$100	;0x23
	out	(c),a

	xor	a
	ld	bc,$3fff ;+$100	;0x00
	out	(c),a

	ld	a,0x00
	ld	bc,$27ff ;+$100	;0x20
	out	(c),a

	xor	a

	ld	bc,$37ff ;+$100	;0x00
	out	(c),a

	jmp end

loop:
	push	af
	push	bc

	push	af

	ld	a,0x23	;0x23
	ld	bc,$1fd0 ;+$100	;0x23
	out	(c),a

	xor	a
	ld	bc,$3fff ;+$100	;0x00
	out	(c),a

	ld	a,0x20
	ld	bc,$27ff ;+$100	;0x20
	out	(c),a

	pop	af
	neg

	ld	bc,$37ff ;+$100	;0x00
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

#endasm
}
