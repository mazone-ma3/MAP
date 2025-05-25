;	CRT0 for the Sharp X1
;
;	Karl Von Dyson (for X1s.org)
;
;    $Id: x1_crt0.asm,v 1.17 2016-07-20 05:45:02 stefano Exp $
;

    MODULE x1_crt0

    defc    crt0 = 1
    INCLUDE "zcc_opt.def"


    EXTERN    _main
    EXTERN    _x1_printf

IF(startup = 3)
	IF      !DEFINED_CRT_ORG_CODE
		defc	CRT_ORG_CODE = 0xc000
	ENDIF
	org	CRT_ORG_CODE
    jp	_main
ELSE
    PUBLIC    cleanup 
    PUBLIC    l_dcal

    PUBLIC	_wait_sub_cpu
    EXTERN  asm_x1_keyboard_handler
ENDIF

    defc    CONSOLE_COLUMNS = 40
    defc    CONSOLE_ROWS = 25

    defc	TAR__no_ansifont = 1
    defc    TAR__clib_exit_stack_size = 32
    defc	__CPU_CLOCK = 4000000


IF(startup = 3)
	defc	TAR__register_sp = 0x0000
;    INCLUDE "crt/classic/crt_z80_rsts.asm"
    INCLUDE "crt/classic/crt_rules.inc"
;    INCLUDE	"crt/classic/crt_init_sp.asm"
;    INCLUDE	"crt/classic/crt_init_atexit.asm"
ELSE
IF (!DEFINED_startup | (startup=1))
    INCLUDE "target/x1/classic/allram.asm"
ELSE
    INCLUDE "target/x1/classic/ipl.asm"
ENDIF
ENDIF

IF(startup != 3)
l_dcal:
    jp      (hl)

_wait_sub_cpu:
    ld      bc, $1A01
.ii_w1
    in      a, (c)
    bit     6, a
    jp      nz, ii_w1
    ret

    IF !DEFINED_ansicolumns
         defc DEFINED_ansicolumns = 1
         defc ansicolumns = 40
    ENDIF

ENDIF

    INCLUDE "crt/classic/crt_runtime_selection.asm"

    INCLUDE	"crt/classic/crt_section.asm"



; X1 stdio support variables
    SECTION	bss_crt
    PUBLIC  _x1_cursor_coords
_x1_cursor_coords:	defw	0




