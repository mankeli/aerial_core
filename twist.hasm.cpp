#include "hires.h"

#include <debugf.h>
#include <hashembler.h>

#include <math.h>
#include "lib.h"
#include "memconfig.h"


using namespace hashembler;

segment_asm_c actualprg;

//#define BORDER_NMI

value_t rotopos1 = 0x40;
value_t rotopos2 = 0x41;

value_t rotoinc1 = 0x42;
value_t rotoinc2 = 0x43;
value_t rotoang = 0x44; // s2
value_t rotoang2 = 0x46; // s2

value_t curinst = 0x048;

value_t colptr = 0x4A; // s2

value_t curcol = 0x4C;

value_t st1 = 0x4D; // s2
value_t st2 = 0x4F; // s2
value_t st1v = 0x51; // s2
value_t st2v = 0x53; // s2

	value_t twister_vel = 0x55; // s2
	value_t twister_dist = 0x57; // s2
	value_t twister_target = 0x59; // s2
	value_t twister_temp = 0x5B; // s2
	value_t twister_pos = 0x5D; // s2

	value_t beginoffs = 0x5F; // s2
	
	value_t eorbyte = 0x61;


value_t colbuffer = 0x80; // size 40!!!

#include "jump_if_not_time.h"

float roundjitter(float in)
{
	float dither = 0.5f;
	dither = glm::linearRand(0.f, 1.f);
	if (glm::fract(in) < dither)
		return floor(in);
	else
		return ceil(in);
}

#include "assets/twister.h"
int graffanpituus = 36; // v4
unsigned char *twistergraffa = twister_v4;
//#include "assets/twister_v5.c"

//#include "assets/twister_v6.c"
//int graffanpituus = 32; // v6
//unsigned char *twistergraffa = twister_v6;


void div16_fast_z(value_t addr)
{
	CMPi(0x80);
	ROR(addr + 1);
	ROR(addr + 0);
}

void sub16_z(value_t outaddr, value_t addr1, value_t addr2)
{
	LDA(addr1 + 0);
	SEC();
	SBC(addr2 + 0);
	STA(outaddr + 0);
	LDA(addr1 + 1);
	SBC(addr2 + 1);
	STA(outaddr + 1);
}

void add16_z(value_t outaddr, value_t addr1, value_t addr2)
{
	LDA(addr1 + 0);
	CLC();
	ADC(addr2 + 0);
	STA(outaddr + 0);
	LDA(addr1 + 1);
	ADC(addr2 + 1);
	STA(outaddr + 1);
}

void effu()
{
	CTX("msdos")
	actualprg.begin(0xC200);
	set_segment(&actualprg);

	LPC("begin");
	PRINTPOS("begin");

	JMP(L("paska"))
	JMP(L("paska2"))

LPC("paska")

	JSR(if_unsetirq1)
	JSR(if_unsetirq2)
	MOV16i(framecounter, 0);


int i;

//	LDAi(0);
//	JSR(if_startmusic)

LPC("paska2")

	LDAi(0x0)
	STA(0xd020)

	LDAi(0x0);
	STA(0xd011);


	LDAi(twistergraffa[1]);
	STA(0xd021);

	LDAi(0);
	LDXi(40);
LPC("reset_colbuffer")
	DEX();
	STAx(colbuffer)
	BNE(L("reset_colbuffer"))


{
	int i;
	LDAi(0x0);
	LDXi(0);
	//LDXz(paskapunaiter);
	LPC("normalizeloop1")
	for (i = 0; i < 4; i++)
	{
		STAx(0x400 + i * 0x100)
		STAx(0xD800 + i * 0x100)
	}
	INX();
	BNE(L("normalizeloop1"))
}

	LDAi(0)
	STAz(curcol)

	LDAi(0x3C);
	STA(0xDD02);

	LDAi(0x14)
	STA(0xd018);

	LDAi(0x07);
	STA(0xd016);

	MOV8i(curinst, 0);



	JSR(L("inittwist"))


	LDXi(LB(L("topirq")))
	LDYi(HB(L("topirq")))
	JSR(if_setirq1)


#if 1
	// cia shite
	LDAi(0x7F)
	STA(0xDD0D);
	LDA(0xDD0D)

	LDAi(LB(L("nmi")))
	STA(0xfffa)
	LDAi(HB(L("nmi")))
	STA(0xfffb)

	LDAi(0x0);
	STA(0xDD0E);
	STA(0xDD0F);

	value_t first_nmi = 63*60;
	LDAi(LB(first_nmi))
	STA(0xdd04)
	LDAi(HB(first_nmi))
	STA(0xdd05)

	LDAi(0x11);
	STA(0xDD0E);

	LDAi(0x81)
	STA(0xDD0D);
#endif


	LDAi(0x1B);
	STA(0xd011);


	JSR(0xC90);

	LDAi(0x0);
	STA(0xDD0E);

	LDAi(0x7F)
	STA(0xDD0D);
	LDA(0xDD0D)

#if 1
	LPC("loop-1")

	JSR(L("colupdate"))
	JSR(L("colupdate"))
	JSR(L("colupdate"))
	JSR(L("colupdate"))
	JSR(L("colupdate"))
	JSR(L("colupdate"))
	jump_if_not_time(sync_twist_gonext, L("loop-1"));



#ifndef NO_LOADING

	LDAi(0x00)
	STA(0xd011);

	JSR(if_unsetirq1)
	JSR(if_unsetirq2)

	//JMP(if_executenext_nowait);

	JMP(0xFFF0);

#endif
#endif

LPC("loopa")
	JSR(L("colupdate"))

	
	JMP(L("loopa"))


{
LPC("nmi")
	PHA();
	TXA();
	PHA();
	TYA();
	PHA();

#ifdef BORDER_NMI
	INC(0xd020);
#endif

	JSR(L("colupdate"))

	LDA(0xDD0D)

#ifdef BORDER_NMI
	DEC(0xd020);
#endif

	PLA();
	TAY();
	PLA();
	TAX();
	PLA();

	RTI();

}



// IRQ ROUTINES BEGIN

{
LPC("inittwist")
	MOV16zi(st1v, 0xFD92);
	MOV16zi(st2v, 0x3F0);

	LDAi(0);
	STAz(twister_pos + 0)
	STAz(twister_pos + 1)
	STAz(twister_vel + 0)
	STAz(twister_vel + 1)
	STAz(twister_dist + 0)
	STAz(twister_dist + 1)

	MOV16zi(twister_target, 0x033);
//	STAz(twister_target + 0)
//	STAz(twister_target + 1)

	RTS();

LPC("topirq")

{
	int i;

	int times[] = {100, 200, 300, 400, 500, 600};
	int speeds[] = {0x009, 0x0094, 0x0999, 0x500, 0xF655, 0xFC92};

	for (i = 0; i < 6; i++)
	{
		LPC("tgt%i", i);
		jump_if_not_time(times[i], L("tgt%i", i + 1));
		MOV16zi(twister_target, speeds[i]);
		JMP(L("targetdecided"))
	}
	LPC("tgt%i", i);
}

LPC("targetdecided")

//	MOV16zi(st1, 0);

	MOV16z(twister_temp, twister_vel);
	LDAz(twister_temp + 1)
	div16_fast_z(twister_temp);
	div16_fast_z(twister_temp);
	div16_fast_z(twister_temp);
	div16_fast_z(twister_temp);
	sub16_z(twister_vel, twister_vel, twister_temp);

	// integrate position
	add16_z(twister_pos, twister_pos, twister_vel);

	// calculate distance to target
	sub16_z(twister_dist, twister_target, twister_pos);
	LDAz(twister_dist + 1)
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);

	// add to velocity
	add16_z(twister_vel, twister_vel, twister_dist);

	MOV16z(st2v, twister_pos);

/*


	mov16_z(twister_temp, twister_vel);
	LDAz(twister_temp + 1)
	div16_fast_z(twister_temp);
	div16_fast_z(twister_temp);
	div16_fast_z(twister_temp);
	div16_fast_z(twister_temp);
//	div16_fast_z(twister_temp);
//	div16_fast_z(twister_temp);
	sub16_z(twister_vel, twister_vel, twister_temp);

	// integrate position
	add16_z(posaddr, posaddr, twister_vel);

	//MOV16i(twister_vel, 0);

	// calculate distance to target
	sub16_z(twister_dist, twister_target, posaddr);
	LDAz(twister_dist + 1)
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);

	// add to velocity
	add16_z(twister_vel, twister_vel, twister_dist);


*/





	ADD16z(st1, st1v);

#if 1
	MOV16z(st2, st1);

	LDYi(20);
	LPC("teff1-a")
		ADD16z(st2, st2v);

		//LDXz(st1 + 1)
		TAX();
		LDAx(L("colmodtab"))
		STAy(colbuffer)
		INY();
		CPYi(40)
		BNE(L("teff1-a"))

	MOV16z(st2, st1);

	LDYi(19);
	LPC("teff1-b")
		//LDXz(st1 + 1)
		TAX();
		LDAx(L("colmodtab"))
		STAy(colbuffer)

//		SUB16z(st2, st2, st2v);
		ADD16z(st2, st2, st2v);
		DEY();
		//CPYi(40)
		BPL(L("teff1-b"))


#endif

#if 0
	MOV16z(st2, st1);

		LDYi(20);
	LPC("teff2-a")
		ADD16z(st2, st2v);

		LDXz(st2 + 1)
		//TAX();
		LDAx(L("colmodtab"))
		STAy(colbuffer)
		INY();
		CPYi(30)
		BNE(L("teff2-a"))


	LPC("teff2-b")
		SUB16z(st2, st2, st2v);

		LDXz(st2 + 1)
		//TAX();
		LDAx(L("colmodtab"))
		STAy(colbuffer)
		INY();
		CPYi(40)
		BNE(L("teff2-b"))


	MOV16z(st2, st1);

	LDYi(20);
	LPC("teff2-c")

		LDXz(st2 + 1)
		//TAX();
		LDAx(L("colmodtab"))
		STAy(colbuffer)

//		SUB16z(st2, st2, st2v);
		ADD16z(st2, st2, st2v);
		DEY();
		CPYi(10)
		BNE(L("teff2-c"))


	LPC("teff2-d")

		LDXz(st2 + 1)
		//TAX();
		LDAx(L("colmodtab"))
		STAy(colbuffer)

		SUB16z(st2, st2, st2v);
		//ADD16z(st2, st2, st2v);
		DEY();
		BPL(L("teff2-d"))


#endif

#if 0
	MOV16z(st2, st1);

		LDYi(00);
	LPC("teff3-a")
		ADD16z(st2, st2v);

		//LDXz(st1 + 1)
		TAX();
		LDAx(L("colmodtab"))
		STAy(colbuffer)
		INY();
		CPYi(10)
		BNE(L("teff3-a"))


	LPC("teff3-b")
		SUB16z(st2, st2, st2v);
		ADD16zi(st2, st2, 0x900);

		//LDXz(st1 + 1)
		TAX();
		LDAx(L("colmodtab"))
		STAy(colbuffer)
		INY();
		CPYi(30)
		BNE(L("teff3-b"))

	LPC("teff3-c")
		ADD16z(st2, st2v);

		//LDXz(st1 + 1)
		TAX();
		LDAx(L("colmodtab"))
		STAy(colbuffer)
		INY();
		CPYi(40)
		BNE(L("teff3-c"))

#endif


#if 0
	for (i = 0; i < 40; i++)
	{
		LDAz(colbuffer+i)
		CLC();
		ADCi(1);
		TAX();
		LDAx(L("colmodtab"))
		STAz(colbuffer+i)
	}
#endif

	RTS();


LPC("colmodtab")
for (i = 0; i < 256; i++)
{
	float ass = (float)i/(float)0x100;
	ass *= graffanpituus;
	int assi = floor(ass);
	assi = glm::clamp(assi, 0, graffanpituus - 1);
	B(assi);
}

}

LPC("colupdate")

PRINTPOS("colupdate")

//	INC(0xd020);
	//LDA(0xd016);
	//EORi(0x04)
	//STA(0xd016);

	LDA(0xd41B);
	LSRa();
	ANDi(0x03);
	STAz(eorbyte)
	LDA(0xd016);
	EORz(eorbyte)
	STA(0xd016);

		
	LDXz(curcol)
	LDAx(L("colorder"))
	TAX();
	STXz(curcol)


	LDAzx(colbuffer)
	TAY();
//	TAY();

//	LDAi(LB(L("graffabuf")))
	LDAy(L("graffabufs_l"))
	STAz(L("colptr"))
//	LDAi(HB(L("graffabuf")))
	LDAy(L("graffabufs_h"))
	STAz(L("colptr")+1)

	LDYi(0);
	for (i = 0; i < 25; i++)
	{
		LDAizy(L("colptr"))
		STAx(0x400 + i * 40)
		INY();
	}

	for (i = 0; i < 25; i++)
	{
		LDAizy(L("colptr"))
		STAx(0xd800 + i * 40)
		INY();
	}
	RTS();

#if 0
int colorder[256];
for (i = 0; i < 256; i++)
{
	colorder[i] = i % 40;
}
for (i = 0; i < 256; i++)
{
	int victim = rand() & 0xFF;

	int tmp = colorder[i];
	colorder[i] = colorder[victim];
	colorder[victim] = tmp;
}

PAGE;
LPC("colorder")
for (i = 0; i < 256; i++)
{
	B(colorder[i]);
}
#endif

	{
		static int randomtab[40];
		int x,y, i;

		if (g_pass == 0)
		{
			for (x = 0; x < 40; x++)
			{
				randomtab[x] = -1;
			}
			int freetab[40];
			int freecount = 0;
			x = 0;
			while(1)
			{
				freecount = 0;
				for (i = 0; i < 40; i++)
				{
					if (randomtab[i] == -1)
					{
						freetab[freecount] = i;
						freecount++;
					}
				}


				int nextfree;
				if (freecount == 1)
				{
					randomtab[x] = 0;
					break;
				}
				else
				{
					int nextfree = freetab[rand() % freecount];
					randomtab[x] = nextfree;
					x = nextfree;
				}

			}

		}

		#if 0
		for (x = 0; x < 40; x++)
		{
			randomtab[x] = (x + 1) % 40;
		}
		#endif

		// test if table is valid

		int cur = randomtab[0];
		//printf("cur %i: %i\n", 0, cur);
		for (x = 1; x < 40; x++)
		{
			cur = randomtab[cur];
			//printf("cur %i: %i\n", x, cur);
		}

		if (cur != 0)
			exit(1);

		LPC("colorder")
		for (x = 0; x < 40; x++)
		{
			// printf("%i ", values[x]);
			B(randomtab[x]);
		}
	}



{
LPC("graffabuf")
//twistergraffa


int x,i;

for (x = 0; x < 40; x++)
{
	LPC("graffabuf%i", x);
	for (i = 0; i < 25; i++)
	{
		B(twistergraffa[2 + i * 40 + x]);
	}
	for (i = 0; i < 25; i++)
	{
		B(twistergraffa[2 + i * 40 + x + 40*25]);
	}
}


LPC("graffabufs_l")
for (x = 0; x < 40; x++)
{
	B(LB(L("graffabuf%i", x)))
}

LPC("graffabufs_h")
for (x = 0; x < 40; x++)
{
	B(HB(L("graffabuf%i", x)))
}
}


}


void maketable(const char *label, int ar[], int num)
{
	LPC(label);
	int i;
	for (i = 0; i < num; i++)
	{
		B(ar[i]);
	}
}

void colorstabs()
{
	CTX("msdos")

}

void genis()
{
	effu();
//	colorstabs();
}

void ekat()
{
}

int main()
{
	ekat();
	assemble(genis);

	list<segment_c *> segs;
	segs.push_back(&actualprg);
	make_prg("twist.prg", 0xC200, segs);
}