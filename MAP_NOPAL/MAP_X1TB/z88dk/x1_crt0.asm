;	CRT0 for the Sharp X1
;
;	Karl Von Dyson (for X1s.org)
;
;    $Id: x1_crt0.asm,v 1.17 2016-07-20 05:45:02 stefano Exp $
;

	MODULE x1_crt0

;
; Initially include the zcc_opt.def file to find out lots of lovely
; information about what we should do..
;

        defc    crt0 = 1
        INCLUDE "zcc_opt.def"

;--------
; Some scope definitions
;--------

        EXTERN    _main
        EXTERN    _x1_printf

        PUBLIC    cleanup 
        PUBLIC    l_dcal

	PUBLIC	_wait_sub_cpu

;--------
; Non-zero origins must be >=32768 (startup=2 must be used for this)
;--------

        IF      !DEFINED_CRT_ORG_CODE
 	    IF (startup=3)
                defc    CRT_ORG_CODE  = $C000
        	defc    TAR__register_sp = 0xFDFF
        ELSE
 	    IF (startup=2)
                defc    CRT_ORG_CODE  = 32768
        	defc    TAR__register_sp = 0xFDFF
            ELSE
                defc    CRT_ORG_CODE  = 0
        	defc    TAR__register_sp = 65535
            ENDIF
        ENDIF
		ENDIF

        defc    CONSOLE_COLUMNS = 40
        defc    CONSOLE_ROWS = 25

	defc	TAR__no_ansifont = 1
        defc    TAR__clib_exit_stack_size = 32
	defc	__CPU_CLOCK = 4000000
        INCLUDE "crt/classic/crt_rules.inc"

        org     CRT_ORG_CODE


;--------
; Execution starts here
;--------
start:


        
IF (!DEFINED_startup | (startup=1))
	di

if (CRT_ORG_CODE > 0)
        defs    ZORG_NOT_ZERO
endif
	INCLUDE	"crt/classic/crt_init_sp.asm"
	im 1
	ei
ENDIF


IF (startup=2)
	di

if (CRT_ORG_CODE < 32768)
        defs    ZORG_TOO_LOW
endif

	INCLUDE	"crt/classic/crt_init_sp.asm"

	; re-activate IPL
	ld bc,$1D00
	xor a
	out (c),a

	ld	hl,$FE00
	push hl
	EXTERN im2_Init
	call im2_Init
	pop hl
	
	EXTERN im2_EmptyISR
	ld hl,im2_EmptyISR
	ld b,128
isr_table_fill:
	ld ($FE00),hl
	inc hl
	inc hl
	djnz isr_table_fill
	ld	hl,_kbd_isr
	ld ($FE52),hl
	
	im 2
	ei

ENDIF

	call _wait_sub_cpu
	ld bc, $1900
	ld a, $E4	; Interrupt vector set
	out (c), a
	call _wait_sub_cpu
	ld bc, $1900
	ld a, $52	; 
	out (c), a

;IF !DEFINED_x1_no_clrscr
;	EXTERN _x1_cls
;	call _x1_cls
;ENDIF

if(startup != 3)
	INCLUDE	"crt/classic/crt_init_atexit.asm"
	call	crt0_init_bss
endif

; INIT math identify platform

; Optional definition for auto MALLOC init
; it assumes we have free space between the end of 
; the compiled program and the stack pointer
IF DEFINED_USING_amalloc
	ld	hl,0
	add	hl,sp
	INCLUDE "crt/classic/crt_init_amalloc.asm"
ENDIF

        call    _main

cleanup:

IF CRT_ENABLE_STDIO = 1
        EXTERN     closeall
        call    closeall
ENDIF

IF ((startup!=3))

	push    hl				; return code
end:	
	jr	end
	rst	0
ENDIF

cleanup_exit:
	ret


IF (!DEFINED_startup | (startup=1))

        defs    56-cleanup_exit-1

if (ASMPC<>$0038)
        defs    CODE_ALIGNMENT_ERROR
endif

ENDIF

; -------------------------------------------
; Don't move the following block !
; It must be aligned when startup=1 !!

_kbd_isr:
	push af
	push bc
	;push de
	push hl
	
	ld bc, $1A01
.iiki1	in a, (c)
	and $20
	jr nz, iiki1
	ld bc, $1900
	in a, (c)
	ld hl, _x1_keyboard_io+1
	ld (hl), a
	dec hl
	ld bc, $1A01
.iiki2	in a, (c)
	and $20
	jr nz, iiki2
	ld bc, $1900
	in a, (c)
	ld (hl), a

	pop hl
	;pop de
	pop bc
	pop af
	ei
	reti

; -------------------------------------------


l_dcal:
        jp      (hl)

_wait_sub_cpu:
	ld bc, $1A01
.ii_w1	in a, (c)
	bit 6, a
	jp nz, ii_w1
	ret

	IF !DEFINED_ansicolumns
		 defc DEFINED_ansicolumns = 1
		 defc ansicolumns = 40
	ENDIF

        INCLUDE "crt/classic/crt_runtime_selection.asm"

	INCLUDE	"crt/classic/crt_section.asm"


	SECTION	code_crt_init
	ld	hl,$FDFF
	ld	(exitsp),hl

; X1 stdio support variables
	SECTION	bss_crt
	PUBLIC    _x1_cursor_coords
	PUBLIC    _x1_keyboard_io
_x1_cursor_coords:	defw	0
_x1_keyboard_io:	defw	0









