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
value_t rotoang3 = 0x48; // s2

value_t curinst = 0x04a;

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


void effu()
{
	CTX("msdos")
	actualprg.begin(0x4000);
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



	LDAi(0x3C);
	STA(0xDD02);
	LDAi(0x2F);
	STA(0xd018);

	LDAi(0x3B)
	STA(0xd011);

	LDAi(0x07);
	STA(0xd016);

	MOV8i(curinst, 0);


	LDXi(LB(L("topirq")))
	LDYi(HB(L("topirq")))
	JSR(if_setirq1)

#if 1
	LPC("loop-1")
	jump_if_not_time(sync_roto_gonext, L("loop-1"));

#ifndef NO_LOADING
//	JSR(0xC90);

	LDAi(0);
	STA(0xd011);

	LDAi(0x7F)
	STA(0xDD0D);
	LDA(0xDD0D)



	JSR(if_unsetirq1)
	JSR(if_unsetirq2)

	JMP(if_executenext_nowait);
#endif

#endif

LPC("loopa")
	//INC(0xd020);
	
	JMP(L("loopa"))


// IRQ ROUTINES BEGIN

{
LPC("topirq")
	int x,y;

	ADD16zi(rotoang, 0x113);
	ADD16zi(rotoang2, 0x101);
	ADD16zi(rotoang3, 0x0e3);

//	ADD16zi(rotoang, 0x193);
//	ADD16zi(rotoang2, 0x010);

	LDXz(rotoang+1);
	LDYz(rotoang2+1);

	LDAx(L("sinus"))
	CLC();
	ADCy(L("sinusc"))
	STAz(rotoinc1)


	LDYz(rotoang3+1);
	LDAx(L("sinus2"))
	CLC();
	ADCy(L("sinusc"))

	STAz(rotoinc2)



	LDAx(L("sinusb"))
	CLC();
	ADCx(L("sinus2b"))
	STAz(rotopos1)


//	LDAx(L("sinus"))
//	STAz(rotoinc1)

//	LDAi(0);
//	STAz(rotopos1)

	CLI();


	LDXz(curinst)
	INX();
	STXz(curinst)

	//JMP(L("instance%i", 2));


	int i;
	for (i = 0; i < 3; i++)
	{
		LPC("instcmp%i", i);
		if (i < 2)
		{
			CPXi(i);
			BNE(L("instcmp%i", i+1));
		}
		else
		{
			LDAi(0)
			STAz(curinst)
		}
		JMP(L("instance%i", i));
	}
	LPC("instcmp%i", i);
	INC(0xd020);
	RTS();
	int inst = 0;

	for (inst = 0; inst < 3; inst++)
	{
		LPC("instance%i", inst);


//		RTS();

		int zxc = 0;

	for (y = 0; y < 25; y++)
	{

		LDAz(rotopos1)
		ADCz(rotoinc1);
		STAz(rotopos1)
		for (x = 0; x < 40; x++)
		{
			zxc++;
			zxc %= 10;
			if (zxc == 0)
			{
				STAz(rotopos2)
				LDA(0xd012)
				EORz(framecounter)
				ANDi(0x07); // 2
				STA(0xd016); // 4
				LDAz(rotopos2)
			}

			ADCz(rotoinc2);
			int palnum = 0;

			vec2 midv = vec2(x-20.0f,y-12.5f);
			float dist = glm::length(midv);
			float fuger = 2.2f - dist * 0.14f;
			fuger += glm::clamp(glm::linearRand(-0.2f, 0.2f), -0.45f, 0.45f);
			palnum = roundjitter(fuger);
			palnum = glm::clamp(palnum, -1, 2);

			if (palnum >= 0)
			{
				TAX();
				LDYx(L("cta%i",palnum))
			}
			else
				LDYi(0);
			STY(0x800 + y*40 + x)
		}
	}

		LDA(0xd011)
		EORi(0x02);
		ANDi(0x7F);
		STA(0xd011);

	RTS();
	}

}


}



void write_colortab(int *t1, int *t2, int numcol)
{
	int i;
	for (i = 0; i < 128; i++)
	{
		float intensity = (float)i / 127.f;
		float fnum = intensity * (numcol - 1.01f);
		int num1 = intensity * (numcol - 1.01);
		int num2 = num1 + 1;

		int colmix = 0;
		float dither = 0.5f;
		dither = glm::linearRand(0.0f, 1.0f);
		if (glm::fract(fnum) < dither)
			colmix |= t1[num1];
		else
			colmix |= t1[num2];

		dither = 0.5f;
		dither = glm::linearRand(0.0f, 1.0f);
		if (glm::fract(fnum) < dither)
			colmix |= t2[num1] << 4;
		else
			colmix |= t2[num2] << 4;

		B(colmix)
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
	int cl1[] = {0, 0,  0,  0,  0,  0,  0,  0,  0};
	int cl2[] = {0, 0,  6,  11, 2,  11, 6,  0,  0};
	int cl3[] = {0, 6,  14, 12, 10, 12, 14, 6,  0};
	int cl4[] = {6, 14, 3,  7,  1,  7,  3,  14, 6};

	PAGE
	LPC("cta%i", 0);
	write_colortab(cl1, cl2, 9);
	write_colortab(cl1, cl2, 9);
	LPC("cta%i", 1);
	write_colortab(cl2, cl3, 9);
	write_colortab(cl2, cl3, 9);
	LPC("cta%i", 2);
	write_colortab(cl3, cl4, 9);
	write_colortab(cl3, cl4, 9);

	int sinusarr[256];
	int sinus2arr[256];

	int sinusbarr[256];
	int sinus2barr[256];

	int sinuscarr[256];

	LPC("sinus")
	int i;
	for(i = 0; i < 256; i++)
	{
		{
			float fval = 256.f + sin(i * M_PI/128.f) * 6.f;
			fval += sin(3.f * i * M_PI/128.f) * 9.f;

			fval += glm::linearRand(-0.1f, 0.1f);
			//fval = roundjitter(fval);
			int val = (int)fval & 0xFF;
			sinusarr[i] = val;
		}
		{
			float fval = 256.f + cos(i * M_PI/128.f) * 6.f;
			fval += cos(2.f * i * M_PI/128.f) * 9.f;

			fval += glm::linearRand(-0.2f, 0.2f);
			//fval = roundjitter(fval);
			int val = (int)fval & 0xFF;
			sinus2arr[i] = val;
		}

		sinusbarr[i] = (256*256 + sinusarr[i] * -12) & 0xff;
		sinus2barr[i] = (256*256 + sinus2arr[i] * -20) & 0xff;

		{
			float fval = 256.f + cos(i * M_PI/128.f) * 6.f;
			fval += cos(5.f * i * M_PI/128.f) * 9.f;

			fval += glm::linearRand(-0.1f, 0.1f);
			//fval = roundjitter(fval);
			int val = (int)fval & 0xFF;
			sinuscarr[i] = val;
		}

	}

	maketable("sinus", sinusarr, 256);
	maketable("sinus2", sinus2arr, 256);
	maketable("sinusb", sinusbarr, 256);
	maketable("sinus2b", sinus2barr, 256);
	maketable("sinusc", sinuscarr, 256);
}

void genis()
{
	effu();
	colorstabs();
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
	make_prg("rotozoom.prg", 0x4000, segs);
}