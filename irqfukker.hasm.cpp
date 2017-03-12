#include "hires.h"

#include <debugf.h>
#include <hashembler.h>

#include <math.h>
#include "lib.h"
#include "memconfig.h"
#include "glm.h"

using namespace hashembler;
segment_asm_c actualprg;

//#define BORDERTIME

void playmusic()
{
#ifdef BORDERTIME
	INC(0xd020);
#endif
	JSR(0x1003);
#ifdef BORDERTIME
	DEC(0xd020);
#endif
}

void genis()
{
	CTX("irqf")

	actualprg.begin(irqfuk_begin);
	set_segment(&actualprg);

	LPC("begin");
	PRINTPOS("begin");
	JMP(L("start_irq"))
	JMP(L("if_setirq1"))
	JMP(L("if_unsetirq1"))
	JMP(L("if_setirq2"))
	JMP(L("if_unsetirq2"))
	JMP(L("if_startmusic"))
	JMP(L("if_executenext"))
	JMP(L("if_executenext_nowait"))

LPC("if_executenext")
	STXz(executewait);
	STYz(executewait+1);
	/*SEI();
	LDAi(HB(0xF000-1))
	PHA();
	LDAi(LB(0xF000-1))
	PHA();
	CLI();
	JMP(0xC90);*/
	JSR(0xC90);


LPC("executewaiter")
	LDAz(framecounter+0)
	CMPz(executewait+0)
	BNE(L("executewaiter"))
	LDAz(framecounter+1)
	CMPz(executewait+1)
	BNE(L("executewaiter"))


	JMP(0xFFF0);

LPC("if_executenext_nowait")
	JSR(0xC90);
	JMP(0xFFF0);


LPC("if_startmusic")
{
	LDAz(musicflag)
	BEQ(L("really_start_music"))
	RTS();
LPC("really_start_music")


	LDXi(0);
LPC("musiccopyloop")
	int i;
	for (i = 0; i < 0xD; i++)
	{
		LDAx(0xE000 + i * 0x100)
		STAx(0x1000 + i * 0x100);
	}
	INX();
	BNE(L("musiccopyloop"))

	LDAi(0);
	JSR(0x1000);
	INCz(musicflag)
	RTS();
}

LPC("if_setirq1")
	PHP();
	SEI();
	LDAi(0x20);
	STA(L("irqmod1"))
	STX(L("irqmod1")+1)
	STY(L("irqmod1")+2)
	PLP();
	RTS();

LPC("if_unsetirq1")
	PHP();
	SEI();
	LDAi(0xEA);
	STA(L("irqmod1"))
	STA(L("irqmod1")+1)
	STA(L("irqmod1")+2)
	PLP()
	RTS();


LPC("if_setirq2")
	PHP();
	SEI();
	LDAi(0x20);
	STA(L("irqmod2"))
	STX(L("irqmod2")+1)
	STY(L("irqmod2")+2)
	PLP()
	RTS();

LPC("if_unsetirq2")
	PHP();
	SEI();
	LDAi(0xEA);
	STA(L("irqmod2"))
	STA(L("irqmod2")+1)
	STA(L("irqmod2")+2)
	PLP()
	RTS();

LPC("start_irq")

	JSR(0xC90);

	LDAi(0x35);
	STAz(0x01);

	SEI();
	LDAi(0x7f);
	STA(0xdc0d);
	STA(0xdd0d);
	LDA(0xdc0d);
	LDA(0xdd0d);


	MOV16i(framecounter, 0);

	MOV8zi(musicflag, 0);
	MOV8i(0xD01A, 0x01);
	MOV8i(0xD012, 0x20);
	MOV8i(0xD011, 0x18);
	LDAi(LB(L("irqrut")))
	STA(0xFFFE);
	LDAi(HB(L("irqrut")))
	STA(0xFFFF);
	CLI();

	JMP(0xfff0)
	//JSR(if_startmusic)

//LPC("infinite")
//	JMP(L("infinite"))

	//RTS();


	LPC("irqrut")
#ifdef BORDERTIME
	INC(0xd020);
#endif
	PHA();
	TXA();
	PHA();
	TYA();
	PHA();
	LDAz(1)
	PHA();
	LDAi(0x35);
	STAz(1);

	LSR(0xd019);

	LDAz(musicflag);
	BEQ(L("irqjmpmod"))
	playmusic();
LPC("irqjmpmod")
	JMP(L("irqrut1"))

	LPC("irqrut1")
	MOV8i(0xD012, 0xff);
	MOV16i(L("irqjmpmod") + 1, L("irqrut2"));
LPC("irqmod1")
	NOP();
	NOP();
	NOP();
	JMP(L("irqend"));


	LPC("irqrut2")

	ADD16zi(framecounter, 1);

	MOV8i(0xD012, 0x20);
	MOV16i(L("irqjmpmod") + 1, L("irqrut1"));
LPC("irqmod2")
	NOP();
	NOP();
	NOP();
//JMP(L("irqend"));

LPC("irqend")
	PLA();
	STAz(1);
	PLA();
	TAY();
	PLA();
	TAX();
	PLA();
#ifdef BORDERTIME
	DEC(0xd020);
#endif
	RTI();

/*
		stx	irqmod+1
		sty	irqmod+2
		sei
		lda	#<irq
		sta	$fffe
		lda	#>irq
		sta	$ffff
		cli
		rts
irq
		pha
		txa
		pha
		tya
		pha
		lda	1
		pha
		lda	#$35
		sta	1
irqmod
		jsr	0
		lsr	$d019

		pla
		sta	1
		pla
		tay
		pla
		tax
		pla
		rti
*/


}


int main()
{
	srand(time(NULL));

	assemble(genis);

	list<segment_c *> segs;
	segs.push_back(&actualprg);
	make_prg("irqfukker.prg", irqfuk_begin, segs);
}