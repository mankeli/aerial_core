#include "hires.h"

#include <debugf.h>
#include <hashembler.h>

#include <math.h>
#include "lib.h"
#include "memconfig.h"

#include "assets/maapallo_petscii.h"

using namespace hashembler;


segment_asm_c actualprg;

value_t chg = 2;

value_t sprite_temp = 0x8; // takes space of 0x11

value_t NEXTFREE = 0x19;

value_t cutpos_a = 0x1F;
value_t cutpos = 0x20;
value_t tvpos = 0x21;

value_t writer_color = 0x22;
value_t nextwriterframe = 0x23;
value_t writerptr = 0x24; // siz 2
value_t cursor_x = 0x26;
value_t cursor_y = 0x27;

value_t writerspeed = 0x28;
value_t writer_run = 0x29;
value_t writerrand = 0x2A;
value_t coloffs = 0x2B; // siz 2
value_t coloffs2 = 0x2D; // siz 2
value_t loadnext = 0x2F;

void msdoseffu()
{
	CTX("msdos")
	actualprg.begin(0x4000);
	set_segment(&actualprg);

	LPC("begin");
	PRINTPOS("begin");

	JMP(L("paska"))

//	LDAi(0);
//	JSR(0xE000);

	// init shit

	LDAi(HB(L("paska")-1))
	PHA();
	LDAi(LB(L("paska")-1))
	PHA();

	RTS();
LPC("paska")



//	B(2);
//	memcopy(0xd000, 0x800, 0x800);

#if 0
	{
			int i, j;
		for (j = 0; j < 8; j++)
		{
			LDA(0xd000+j);
//			LDAi(0xF0);
			for (i = 0; i < 64; i++)
			{
				STA(0x800 + i * 8 + j)
			}
		}
	}
#endif

	// memcopy(L("maapallo_color"), 0x800, 0x400);

//	B(2);
	MOV8zi(loadnext, 0);

LDAz(musicflag)
BNE(L("skip_sid_init"))


	// SID stuff

	LDAi(0x41);
	STA(0xd416);

	LDAi(0x70);
	STA(0xd417);
	LDAi(0x2F);
	STA(0xd418);


	float pal_freq = 985248;
	float freqmul = powf(256.f, 3.f) / pal_freq;

	value_t freq1 = (value_t)(freqmul * 50.f);
	value_t freq2 = (value_t)(freqmul * 49.1f);

	// voice 2

	LDAi(LB(freq1));
	STA(0xd407);
	LDAi(HB(freq1));
	STA(0xd408);

	LDAi(0xD0);
	STA(0xd40C);
	LDAi(0xFa);
	STA(0xd40D);

	LDAi(0x11);
	STA(0xd40B);


	// voice 3

	LDAi(LB(freq2));
	STA(0xd40E);
	LDAi(HB(freq2));
	STA(0xd40F);

	LDAi(0xF0);
	STA(0xd413);
	LDAi(0xFa);
	STA(0xd414);

	LDAi(0x11);
	STA(0xd412);



	// voice 1 (glitch)

	LDAi(0x02);
	STA(0xd400);
	STA(0xd401);

	LDAi(0x29);
	STA(0xd404);

	LDAi(0x21);
	STA(0xd404);

	LDAi(0x01);
	STA(0xd405);
	LDAi(0x05);
	STA(0xd406);

LPC("skip_sid_init")

	LDAi(0x35);
	STAz(0x01);


	LDAi(0xDE)
	STA(0xFFF0);
	LDAi(0xAD)
	STA(0xFFF1);
	LDAi(0xFA)
	STA(0xFFF2);
	LDAi(0xCE)
	STA(0xFFF3);

	LDAi(0x00)
	STA(0xd011)

	LDXi(0);

LPC("ff")
	LPC("wait1")
	LDA(0xd011);
	BPL(L("wait1"))
	LPC("wait2")
	LDA(0xd011);
	BMI(L("wait2"))

	LDAx(L("flashtab"))
	CMPi(0xFF)
	BEQ(L("stopflashing"));
	printf("%04X FFF\n", L("stopflashing"));
	STA(0xd020);
	INX();

	JMP(L("ff"))

LPC("flashtab")
	int i;
	for (i = 0; i < 0x10; i++)
	{
		int r = rand() & 0x03;
		int c;
		if (i < r)
			c = 1;
		else
			c = 0;

		B(c);
	}
	B(0xFF);

LPC("stopflashing")

//	B(2);

	LDAi(0x12)
	STA(0xd018);
	//LDAi(0x97);
	//STA(0xdd00);
	LDAi(0x3c);
	STA(0xdd02);


#if 0
	LPC("moi");
	INC(0xd020);
	JMP(L("moi"))
#endif

LDAz(musicflag)
BNE(L("skip_sid2"))
	// filt (writer)

	LDAi(0x71);
	STA(0xd417);


	// voice 1 (writer)

	LDAi(0x20);
	STA(0xd400);
	STA(0xd401);

	LDAi(0x20);
	STA(0xd404);

	LDAi(0x01);
	STA(0xd405);
	LDAi(0x05);
	STA(0xd406);

LPC("skip_sid2")


	LDAi(0x0)
	STA(0xd020);
	LDAi(0x5)
	STA(0xd021);
	LDAi(0xD)
	STA(0xd022);
	LDAi(0x1)
	STA(0xd023);
	LDAi(0xe)
	STA(0xd024);

	LDAi(1)
	STAz(cursor_x)
	STAz(cursor_y)


	LDAi(30)
	STAz(nextwriterframe)

	LDAi(LB(L("text")))
	STAz(writerptr + 0)
	LDAi(HB(L("text")))
	STAz(writerptr + 1)

	LDAi(0);
	STAz(writer_color);


	int sprpos = 0x3C0;
	LDAi(0x55);
	LDXi(63);
LPC("spritedumloop1");
	DEX();
	STAx(sprpos)
	DEX();
	STAx(sprpos)
	DEX();
	STAx(sprpos)
	DEX();
	DEX();
	DEX();
	BPL(L("spritedumloop1"))

	LDAi(0xAA);
	LDXi(60);
LPC("spritedumloop2");
	DEX();
	STAx(sprpos)
	DEX();
	STAx(sprpos)
	DEX();
	STAx(sprpos)
	DEX();
	DEX();
	DEX();
	BPL(L("spritedumloop2"))

	// SOME SHIT IS WRITTEN TO 0x400++ BUT JAXA FIXAA

	LDAi(0x20+0x80);
	LDXi(0);
	LPC("normalizeloop")
	for (i = 0; i < 4; i++)
	{
		STAx(0x400 + i * 0x100)
	}
	INX();
	BNE(L("normalizeloop"))
	JSR(L("clear_colormem"))



	// spraits!

	LDAi(0xFF);
	STA(0xd015);
	STA(0xd01d);

	// double height
	LDAi(0xFF);
	STA(0xd017);

	// priority
	LDAi(0xFF);
	STA(0xd01b);


	LDAi(0x09);
	for (i = 0; i < 8; i++)
		STA(0xd027+i);

	// positions set in irq

	LDAi(sprpos/64)
	for (i = 0; i < 8; i++)
		STA(0x400+0x3f8+i)

	//MOV16i(0x0314, L("irqrut_kernal"));
	//MOV16i(0x0316, L("irqrut_kernal"));
	//MOV16i(0xFFFE, L("irqrut_direct"));
//	B(2);
	//CLI();

	LDAi(0x7B)
	STA(0xd011)

//	B(2);


	LDXi(LB(L("irqrut")))
	LDYi(HB(L("irqrut")))
	JSR(if_setirq2);


	LDAi(1)
	STAz(writer_run)
	
	LDAi(0x00)
	STAz(cutpos_a)

LPC("beforejumi")

	LDAi(0x07)
	STA(0xd016);
	//STA(0xd020);

	LDAi(0x3B)
	STA(0xd011)
	LDAi(0x28)
	STA(0xd018);

	LDAi(0xFF)
	STA(0xd000)
	STA(0xd002)
	STA(0xd004)
	STA(0xd006)
	STA(0xd008)
	STA(0xd00A)
	STA(0xd00C)
	STA(0xd00E)
	STA(0xd010)

	LDAz(loadnext)
	BEQ(L("no_loadnext"))
	JMP(L("nextpart"))
LPC("no_loadnext")


LPC("jumi")



	LDAz(writer_run)
	BNE(L("no_writer_runs"))
	JMP(L("writer_run_func"))
LPC("no_writer_runs")


	LDA(0xd012);
	ADCi(0x04);
	CMPz(cutpos);
	//CMPi(0x80);
	BCC(L("beforejumi"))

LPC("beforejumi2")

	LDAi(0x14)
	STA(0xd018);
	LDAi(0x7B)
	STA(0xd011)
	LDAi(0xFF)
	STA(0xd016)

	LDA(0xd012);
	CMPz(cutpos);
	//CMPi(0x80);
	BCC(L("beforejumi2"))

//#define SHOWTRANS


#ifdef SHOWTRANS
	LDAi(2);
	STA(0xd020);
#endif
#if 1
	//LDAx(L("rand"))
	//ANDi(1);

	for (i = 0; i < 8; i++)
	{
//		LDA(0xd000+i*2);
		LDAz(sprite_temp + i * 2)
		STA(0xd000+i*2);
	}
#else
	LDAx(L("rand"))
	ANDi(7);
	ASLa();
	TAX();
	LDAzx(sprite_temp);
	STAx(0xd000);
#endif



#if 1	
	LDAz(sprite_temp + 0x10)
	STA(0xd010);
#endif


#if 1
LPC("endblack")
	LDA(0xd012);
	SBCi(0x08);
	CMPz(cutpos);
	BCC(L("endblack"))

#endif

#ifdef SHOWTRANS
	LDAi(0);
	STA(0xd020);
#endif

	LDAi(0x1B)
	STA(0xd011)
	LDAi(0x14)
	STA(0xd018);

#if 1
	LDA(0xd012);
	ANDi(0x3F);
	ADCz(tvpos);
	EORz(framecounter);
	TAX();
#endif


LPC("fuckd016")
	LDAx(L("rand"))
	ANDi(0x01);
	ADCi(0x05);
	STA(0xd016);
	INX();

	LDA(0xd012);
	CMPz(cutpos);
	//CMPi(0x80);
	BCS(L("fuckd016"))

	JMP(L("jumi"))



	LPC("writer_run_func")
	LDAz(writer_run)
	ORAi(0x01);
	STAz(writer_run)
	//INC(0xd020);
#if 1
	{

	#define SCR_CARRIAGE (0x70)
	#define SCR_SPEED1 (0x71)
	#define SCR_SPEED2 (0x72)
	#define SCR_SPEED3 (0x73)
	#define SCR_WAIT (0x74)
	#define SCR_LOADNEXT (0x75)
	#define SCR_RUNNEXT (0x76)
	#define SCR_SETCOL1 (0x78)
	#define SCR_SETCURSOR (0x79)
	#define SCR_SETCOL3 (0x7A)
	#define SCR_SETCOL4 (0x7B)
	#define SCR_SETCOL5 (0x7C)


		LDYi(0);
		LDAizy(writerptr)

		CMPi(SCR_CARRIAGE)				BNE(L("wr_1"))
		JSR(L("carriage"))
		LDAi(2)
		STAz(cursor_x)

		JMP(L("writer_inc_ptr"))

		LPC("wr_1") CMPi(SCR_SPEED1) 	BNE(L("wr_2"))
		LDAi(6);
		STAz(writerspeed)
		LDAi(1);
		STAz(writerrand)
		JMP(L("writer_inc_ptr"))

		LPC("wr_2") CMPi(SCR_SPEED2) 	BNE(L("wr_3"))
		LDAi(1);
		STAz(writerspeed)
		LDAi(0);
		STAz(writerrand)
		JMP(L("writer_inc_ptr"))

		LPC("wr_3") CMPi(SCR_SPEED3) 	BNE(L("wr_4"))
		LDAi(12);
		STAz(writerspeed)
		LDAi(0x0F);
		STAz(writerrand)
		JMP(L("writer_inc_ptr"))

		LPC("wr_4") CMPi(SCR_WAIT) 		BNE(L("wr_5"))
		JMP(L("writer_inc_ptr"))

		LPC("wr_5") CMPi(SCR_LOADNEXT) 	BNE(L("wr_6"))
		LDAi(0xFF);
		STAz(writer_run)
//		JSR(L("load_nextpart"))
		JMP(L("writer_inc_ptr"))

		LPC("wr_6") CMPi(SCR_RUNNEXT) 	BNE(L("wr_7"))
		//JSR(L("nextpart"))
		JMP(L("writer_inc_ptr"))

		LPC("wr_7") CMPi(SCR_SETCOL1) 	BNE(L("wr_8"))
		LDAi(0);
		STAz(writer_color);
		JMP(L("writer_inc_ptr"))

		LPC("wr_8") CMPi(SCR_SETCURSOR) 	BNE(L("wr_9"))

		ADD16zi(writerptr, 1);

		LDYi(0);
		LDAizy(writerptr)
		STAz(cursor_x);
		JMP(L("writer_inc_ptr"))

		LPC("wr_9") CMPi(SCR_SETCOL3) 	BNE(L("wr_10"))
		LDAi(0xD);
		STAz(writer_color);
		JMP(L("writer_inc_ptr"))

		LPC("wr_10") CMPi(SCR_SETCOL4) 	BNE(L("wr_11"))
		LDAi(0x7);
		STAz(writer_color);
		JMP(L("writer_inc_ptr"))

		LPC("wr_11") CMPi(SCR_SETCOL5) 	BNE(L("wr_12"))
		LDAi(1);
		STAz(writer_color);
		JMP(L("writer_inc_ptr"))

		LPC("wr_12")

		LDXz(cursor_x)
		//LDAx(0x400 + 24 * 40)
		//ANDi(0xC0);
		LDAizy(writerptr)
LPC("wrpos1")
		STAx(0x400 + 1 * 40)
		LDAz(writer_color)
LPC("wrpos2")
		STAx(0xd800 + 1 * 40)

		INX();
		STXz(cursor_x)

LDAz(musicflag)
BNE(L("skip_sid3"))

		LDAi(0x20);
		STA(0xd404);

		LDAi(0x21);
		STA(0xd404);
LPC("skip_sid3")

		LPC("writer_inc_ptr")

		ADD16zi(writerptr, 1);

#if 1
		LDXz(framecounter + 0)
		LDAx(L("rand"))
		ANDz(writerrand)
		ADCz(framecounter + 0)
		ADCz(writerspeed);
		STAz(nextwriterframe)
#else
		LDAz(framecounter + 0)
		ADCz(writerspeed);
		STAz(nextwriterframe)
#endif
	}
#endif
	//DEC(0xd020);
	JMP(L("jumi"))

LPC("rand")
	for (i = 0; i < 0x100; i++)
	{
		int b = rand() & 0xFF;
		B(b);
	}



/* IRQ RUOTINE */



LPC("irqrut");

	//INC(0xd020)

	LDAz(tvpos)
	CLC();
	ADCi(0x05);
	STAz(tvpos);
	for (i = 0; i < 8; i++)
	{
		STA(0xd001+i*2);
	}
	LDAz(framecounter)
	ADCz(tvpos);
	TAX();
	LDAx(L("rand"))
	ANDi(0x03);
	TAX();

	uint8_t sprite9bit = 0;
	for (i = 0; i < 8; i++)
	{
		int xp = i*48+23;
		if (xp >= 256)
			sprite9bit |= 1 << i;
		TXA();
		ADCi(xp)
		STA(sprite_temp+i*2);
	}
	LDAi(sprite9bit);
	STA(sprite_temp + 0x10);

	JMP(L("calc_cutpos"))

LPC("sinus")
	for (i = 0; i < 0x100; i++)
	{
		float ang = i * (M_PI/0x30);
		float ang2 = i * (M_PI/0x20);
		float mulli = (float)i/(float)0xFF;
		int pos = 0x20 + (0x110-0x20) * mulli;
		pos += (sin(ang) * 0x20 + sin(ang2) * 0x0D) * (1.f - powf(mulli, 2.f));
		
		if (pos < 0)
			pos = 0;
		else if (pos > 0xff)
			pos = 0xFF;

		B(pos)
	}

LPC("calc_cutpos")

//	;

	//LDXz(framecounter)
	//LDAx(L("sinus"))
	//LDAi(0x30)
	//STAz(cutpos)

	LDAz(writer_run)
	ANDi(0x80);
	CMPi(0x80);
	BNE(L("noquityet"))

	LDAz(cutpos_a)
	CLC();
	ADCi(0x01);
	CMPi(0xf0);
	BEQ(L("end_cutpos"))
	STAz(cutpos_a)
	JMP(L("noquityet"))
//	LDAi(0x80)
//	STA(0xd40B);
//	LDAi(0x81)
//	STA(0xd40B);
LPC("end_cutpos")

// vittu tää kyllä pitäis tulla writeristä eikä suoraan tästä :/
	//B(2);
	//JMP(L("nextpart"))
	INCz(loadnext)

LPC("noquityet")
	LDXz(cutpos_a)
	LDAx(L("sinus"))
	STAz(cutpos)
//	STA(0xd408);

	//LDAi(0x80);
	//STAz(cutpos)


#if 1
	LDAz(framecounter + 0)
	CMPz(nextwriterframe)
	BEQ(L("part_writer"))
	JMP(L("no_part_writer"))
	LPC("part_writer");
	LDAz(writer_run)
	ANDi(0xFE);
	STAz(writer_run)

#endif

	LPC("no_part_writer")

	//DEC(0xd020);

	RTS();

	LPC("load_nextpart")


	LPC("nextpart")

	JSR(if_unsetirq2)

LDAz(musicflag)
BNE(L("skip_sid4"))

	LDAi(0x10);
	STA(0xd412);
	STA(0xd40b);

LPC("skip_sid4")



//	SEI();
/*	LDAi(HB(0x4000-1))
	PHA();
	LDAi(LB(0x4000-1))
	PHA();

	JMP(0xC90);
	CLI();*/
	JMP(if_executenext_nowait);

		//JMP(actual_start)

		RTS();

	LPC("clear_colormem")
		LDXi(0);
		TXA();
		LPC("clearcolmemloop")
		STAx(0xd800);
		STAx(0xd900);
		STAx(0xdA00);
		STAx(0xdB00);
		INX();
		BNE(L("clearcolmemloop"))
		RTS();



	LPC("carriage")
	{
		int i;

		LDAz(cursor_y);
		CMPi(24);
		BEQ(L("movescreenup"))
		CLC();
		ADCi(0x01);
		STAz(cursor_y);

		ADD16i(L("wrpos1")+1, 40);
		ADD16i(L("wrpos2")+1, 40);


		RTS();

	LPC("movescreenup")
		for (i = 1; i < 25; i++)
		{
			value_t c1 = 0x400+(i-1)*40;
			value_t c2 = 0x400+i*40;
			LDAi(LB(c2))
			STAz(coloffs)
			LDAi(HB(c2))
			STAz(coloffs + 1)
			LDAi(LB(c1))
			STAz(coloffs2)
			LDAi(HB(c1))
			STAz(coloffs2 + 1)
			JSR(L("movecol"))

			c1 = 0xD800+(i-1)*40;
			c2 = 0xd800+i*40;
			LDAi(LB(c2))
			STAz(coloffs)
			LDAi(HB(c2))
			STAz(coloffs + 1)
			LDAi(LB(c1))
			STAz(coloffs2)
			LDAi(HB(c1))
			STAz(coloffs2 + 1)
			JSR(L("movecol"))

		}
		LDAi(0x80+0x20)
		LDXi(40)
		LPC("clearheadrowloop")
		DEX();
		STAx(0x400 + 24*40)
		BNE(L("clearheadrowloop"))

		LDAi(0)
		LDXi(40)
		LPC("clearheadrowloop2")
		DEX();
		STAx(0xD800 + 24*40)
		BNE(L("clearheadrowloop2"))

		RTS();
	}

	LPC("movecol")
//	;
	LDYi(0);
	for (i = 0; i < 40; i++)
	{
		LDAizy(coloffs);
		STAizy(coloffs2);

		INY();
	}
	RTS();



	// \b fast text
	// \f super flow
// 0x70 \n carriage
// 0x71 \a writerspeed = 6
// 0x72 \b writerspeed = 1
// 0x73 \f writerspeed = ??
// 0x74 \t unused
// 0x75 \r load next part
// 0x76 \v run next part



	LPC("text")
	const char *text = "\f\t\t\t\t\t\t\t\a\nAERIAL CORE BBS\b\t\t\n\n"
	"LOGIN: \fJELTZ, \tV.\t\t\n"
	"\bPASSWORD: \f****\a*\f**\n"
	"\b\t\t\n\n"
	"LOGIN ACCEPTED.\n"
	"\a\t\t\t\t\t\n\nDOWNLOADING DATA...\n\t\t\n\b";
//	"\v";

	for (i = 0; i < strlen(text); i++)
	{
		if (text[i] == '\n')
			B(SCR_CARRIAGE)
		else if (text[i] == '\a')
			B(SCR_SPEED1)
		else if (text[i] == '\b')
			B(SCR_SPEED2)
		else if (text[i] == '\f')
			B(SCR_SPEED3)
		else if (text[i] == '\t')
			B(SCR_WAIT)
		else if (text[i] == '\r')
			B(SCR_LOADNEXT)
		else if (text[i] == '\v')
			B(SCR_RUNNEXT)
		else if (text[i] == '\\')
			B('/')
		else
			B((text[i] & 0x3F) | 0x80)
	}
	int x,y;
	int ccol = 0;
	for (y = 0; y < 25; y++)
	{
		int buf[40];
		int last_x = 0;
		int first_x = 0;
		for (x = 0; x < 40; x++)
		{
			buf[x] = maapallo_petscii[2 + y*40 + x];
			int col = maapallo_petscii[2 + 40*25 + y*40 + x];

			if (buf[x] != 160)
				last_x = x;

			if (col == 0xB)
				buf[x] = 0x20;

			if (buf[x] != 160 && first_x == 0)
			{
				first_x = x;
			}

		}
		//last_x = 39;

		if (first_x > 0)
		{
			B(SCR_SETCURSOR)
			B(first_x)
		}

		for (x = first_x; x <= last_x; x++)
		{
			int col = maapallo_petscii[2 + 40*25 + y*40 + x];
			if (ccol != col)
			{
				if (col == 0)
					B(SCR_SETCOL1)
//				else if (col == 0xB)
//					B(SCR_SETCOL5)
				else if (col == 0xC)
					B(SCR_SETCOL3)
				else if (col == 0xF)
					B(SCR_SETCOL4)
				else if (col == 0x1)
					B(SCR_SETCOL5)

				ccol = col;
			}
			B(buf[x]);
		}
		if (y < 24)
			B(SCR_CARRIAGE);
	}
	B(SCR_SPEED3)
	B(SCR_WAIT)
	B(SCR_WAIT)
	B(SCR_WAIT)
	B(SCR_WAIT)
	B(SCR_WAIT)
	B(SCR_WAIT)
	B(SCR_WAIT)
	B(SCR_WAIT)
	B(SCR_WAIT)
	B(SCR_WAIT)
	B(SCR_WAIT)
	B(SCR_LOADNEXT)

	B(0);

/*	PAGE;

	maapallo_color();
*/


}

void genis()
{
	msdoseffu();
}

int main()
{
	assemble(genis);

	list<segment_c *> segs;
	segs.push_back(&actualprg);
	make_prg("msdos.prg", 0x4000, segs);
}