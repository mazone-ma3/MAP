/* サブCPU止めてVRAMアクセス 2000-5FFFに1プレーン16K全展開する */
/* $6AFFに有効=1～・無効=0を書いておく */
//#pragma exec_once

unsigned char *mmr;
unsigned char *vram;
unsigned char *mem;
unsigned char *msr;
unsigned char *msr_sv;

#define PAGE 0x02

void sub_disable(void)
{
asm{
SUBHLT:
	lda	$FD05
	bmi	SUBHLT
	orcc	#$50
	lda	#$80
	sta	$FD05
	lda	$FD05
	bpl	*-3
}
}

void sub_enable(void)
{
asm{
	ldb	$FC80
	orb	#$80
	stb	$FC80

	clr	$FD05
	andcc	#$AF
}
}

void bank1_on()
{
	mmr[2] = 0x1d;
	vram[0x410] = 0;
	vram[0x430] = 0x60;
}

void bank1_off(void)
{
	mmr[2] = 0x1d;
	vram[0x410] = 0;
	vram[0x430] = 0x00;
}

void vram_b_on(void)
{
	mmr[2] = 0x10;
	mmr[3] = 0x11;
	mmr[4] = 0x12;
	mmr[5] = 0x13;
}

void vram_r_on(void)
{
	mmr[PAGE] = 0x14;
	mmr[PAGE+1] = 0x15;
	mmr[PAGE+2] = 0x16;
	mmr[PAGE+3] = 0x17;
}

void vram_g_on(void)
{
	mmr[PAGE] = 0x18;
	mmr[PAGE+1] = 0x19;
	mmr[PAGE+2] = 0x1a;
	mmr[PAGE+3] = 0x1b;
}

void vram_off(void)
{
	mmr[2] = 0x32;
	mmr[3] = 0x33;
	mmr[4] = 0x34;
	mmr[5] = 0x35;
}

void save_msr(void)
{
	*msr_sv = *msr;
	*msr |= 0x80;
}

void load_msr(void)
{
	*msr = *msr_sv;
}

int main(void){
	mmr = (unsigned char *)0xFD80;
	vram = (unsigned char *)0x2000;
	mem = (unsigned char *)0x6AFF;
	msr = (unsigned char *)0xFD93;
	msr_sv = (unsigned char *)0x6AFE;

	if(*mem == 0){
		bank1_off();
		vram_off();
		load_msr();
		sub_enable();
	}else{
		if(!(*mem & 0x80)){
			sub_disable();
			save_msr();
		}
		if(*mem & 0x40)
			bank1_off();
		else
			bank1_on();
		switch(*mem & 0x3f){
			case 1:
				vram_b_on();
				break;
			case 2:
				vram_r_on();
				break;
			case 3:
				vram_g_on();
				break;
		}
	}
	return 0;
}
