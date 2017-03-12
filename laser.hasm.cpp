#include "hires.h"

#include "debugf.h"
#include "hashembler.h"

#include <math.h>

#include "memconfig.h"
#include "lib.h"

hiresdata_t homo;

/*

	CIA TIMING IN THIS PART IS COMPLETELY MESSED UP
	( TIME BETWEEN IRQ:S IS TOO LONG AND THEREFORE THEY MIGHT OVERLAP CONTROLLING IRQ'S)
	EXPECT PROBLEMS !!

*/

//#define BORDER_NMI

using namespace hashembler;

#include "lib.h"

segment_asm_c initprg;

value_t cyclebase = 0x40;
value_t rutnum = 0x3F;
value_t colptr = 0x3D; //s2
value_t fadevar = 0x3C;
value_t paskacounter = 0x3B;

value_t lineptrtab = 0x1A; // s 1E

void write_colortab(int *t1, int *t2, int numcol)
{
	int i;
	for (i = 0; i < 128; i++)
	{
		int i2 = i & 0xFF;
		float intensity = (float)i2 / 127.f;
		float fnum = intensity * (numcol - 1.01f);
		int num1 = intensity * (numcol - 1.01);
		int num2 = num1 + 1;

		int colmix = 0;
		float dither = 0.5f;
		dither = (rand() & 65535) / 65535.f;
		if (glm::fract(fnum) < dither)
			colmix |= t1[num1];
		else
			colmix |= t1[num2];

		dither = (rand() & 65535) / 65535.f;
		if (glm::fract(fnum) < dither)
			colmix |= t2[num1] << 4;
		else
			colmix |= t2[num2] << 4;

		B(colmix)
	}
}

int hextonum(char h)
{
	if (h >= '0' && h <= '9')
		return h - '0';
	else if (h >= 'a' && h <= 'f')
		return h - 'a';

	printf("illegal char in hex data! %02X", h);
	exit(1);
	return 0;
}

void write_colortab2(char *t1, char *t2, int outnum, int outlimit, int numcol)
{
	int i;
	for (i = 0; i < outlimit; i++)
	{
		float intensity = (float)i / (float)(outnum - 1);
		float fnum = intensity * (numcol - 1.01f);
		int num1 = intensity * (numcol - 1.01);
		int num2 = num1 + 1;

		int c11 = hextonum(t1[num1]);
		int c12 = hextonum(t1[num2]);
		int c21 = hextonum(t2[num1]);
		int c22 = hextonum(t2[num2]);

		int colmix = 0;

		float dither = 0.5f;
		dither = (rand() & 65535) / 65535.f;
		if (glm::fract(fnum) < dither)
			colmix |= c11;
		else
			colmix |= c12;

		dither = (rand() & 65535) / 65535.f;
		if (glm::fract(fnum) < dither)
			colmix |= c21 << 4;
		else
			colmix |= c22 << 4;

		B(colmix)
	}

}

//	write_colortab2(ncl1, ncl2, 554, 554, ncllen);


int cl2[256];
int cl3[256];
int cllen = 2;

void initcolors()
{
	JMP(L("actualcopy"))

	int cl0[] = {2, 2,  2,  2,  2,  2,  2,  2, 2};
	int cl1[] = {0, 0,  0,  0,  0,  0,  0,  0, 0};
	int cl2[] = {0, 0,  6,  11, 2,  11, 6,  0, 0};
	int cl3[] = {0, 6,  14, 12, 10, 12, 14, 6, 0};
	int cl4[] = {6, 14, 3,  7,  1,  7,  3,  14, 6};
	int cl5[] = {0, 6,  14, 12, 10, 12, 14, 6, 0};

/*
	int cl1[] = {2, 2,  2,  2,  2,  2,  2,  2, 0};
	int cl2[] = {2, 2,  6,  11, 2,  11, 6,  2, 0};
	int cl3[] = {2, 6,  14, 12, 12, 12, 14, 6, 0};
	int cl4[] = {6, 14, 3,  7,  1,  7,  3,  14, 6};
	int cl5[] = {2, 6,  14, 12, 12, 12, 14, 6, 0};
*/
	//int cl5[] = {0, 3,  14, 12, 10, 12, 14, 3, 0};



//	int cl2[] = {0, 0,  6,  11, 2,  11, 6,  0, 0};
//	int cl3[] = {0, 6,  14, 12, 10, 12, 14, 6, 0};


//	int cl2[] = {0, 0,  3,  15, 7,  15, 3,  0, 0};
//	int cl3[] = {0, 3,  14, 12, 10, 12, 14, 3, 0};

//	int cl2[] = {0, 0,  6,  0xe, 3,  0xe, 6,  0, 0};
//	int cl3[] = {0, 6,  14, 12, 10, 12, 14, 6, 0};

	char *ncl1 = "2246e3d7fcb89904a71d5902b4ace5d11fcb9000";
	char *ncl2 = "298bcf71d3e64066e3dfcb9b48e5caf711fcb902";

	int ncllen = strlen(ncl1);

	LPC("ct");
	write_colortab(cl0, cl0, 9);
	//write_colortab(cl0, cl0, 9);
	LPC("ctrb");
	write_colortab2(ncl1, ncl2, 554, 554, ncllen);
	LPC("ctend")
	write_colortab2(ncl1, ncl2, 554, 128, ncllen);

	//write_colortab(cl2, cl3, 9);
//	write_colortab(cl2, cl3, cllen);

	LPC("actualcopy")
	int i;
	for (i = 0; i < 0x80; i++)
	{
		LDA(L("ct")+i)
		STAz(cyclebase+i)
	}

	LDAi(LB(L("ct")))
	STAz(colptr+0)
	LDAi(HB(L("ct")))
	STAz(colptr+1)

}


void movecolors()
{
	int i;
#if 0
	LDXz(cyclebase+0x7F)
	for (i = 0x7E; i >= 0; i--)
	{
		LDAz(cyclebase+i)
		STAz(cyclebase+i+1)
	}
	STXz(cyclebase);
#endif

	LDAz(colptr+0)
	CLC();
	ADCi(1);
	STAz(colptr+0)
	LDAz(colptr+1)
	ADCi(0);
	STAz(colptr+1)

	CMPi(HB(L("ctend")))
	BNE(L("changecolors"))
	LDAz(colptr+0)
	CMPi(LB(L("ctend")))
	BNE(L("changecolors"))

	LDAi(LB(L("ctrb")))
	STAz(colptr+0)
	LDAi(HB(L("ctrb")))
	STAz(colptr+1)

LPC("changecolors")

	LDYi(0);
	for (i = 0; i < 0x80; i++)
	{
		LDAizy(colptr);
		STAz(cyclebase+0x7F-i)
		INY();
	}
}

void copycolors()
{
	int x,y;
	int i;

	LPC("copycolors")

	LDAi(0)
	STAz(paskacounter)

	LDAz(rutnum);
	CLC();
	ADCi(5);
	STAz(rutnum);

	int lastnum = 0;
	for (i = 0; i < 5; i++)
	{
		LPC("colcmp%i", i);
		int num = ((float)(i+1) / 6.f) * 255;
		printf("CMP %i: %i (d %i)\n", i, num, num-lastnum);
		lastnum = num;
		CMPi(num);
		BCS(L("colcmp%i", i+1))
		JMP(L("copy%i", i));
	}
	LPC("colcmp%i", i);
	//LDAi(1);
	//STAz(rutnum);
	JMP(L("copy%i", i));


	for (i = 0; i < 6; i++)
	{
		float frameang = (float)i * (M_PI/7.f) * 2.f;
		PRINTPOS("copystarts")
		LPC("copy%i", i);

#if 1
		for(y = 10; y < 25; y++)
		{
			value_t rptr = L("fryg%i%i", i, y);
			int ptrpos = (y - 10) * 2;

			LDAi(LB(rptr))
			STAz(lineptrtab + ptrpos + 0)
			LDAi(HB(rptr))
			STAz(lineptrtab + ptrpos + 1)
		}
#endif

//	RTS();

	for(y = 0; y < 25; y++)
	{
		LPC("fryg%i%i", i, y);
		for(x = 0; x < 40; x++)
		{
//			float pos = (float)x / 39.f;

			float xf = (x - 20.f) / 20.f;
			float yf = (y - 12.f) / 20.f;

//			xf += sin(yf * 3.1f) * 0.3f;
//			yf += sin(xf * 3.1f) * 0.3f;

			float pos = sqrt(xf*xf + yf*yf) * 2.f;
			pos += sin(xf * 13.1f) * 0.1f;
			pos += sin(yf * 15.1532f) * 0.1f;

			float ang = atan2(xf, yf);
			//pos += sin(frameang + yf * 15.f) * 0.05f;
			//pos += sin(frameang + xf * 15.f) * 0.05f;

			pos += sin(ang * 6.f + frameang) * 0.08f;

			//int ipos = (int)(200+pos*98);
			int ipos = (int)(pos*50);

			ipos = glm::clamp(ipos, 0, 127);
			ipos = ipos & 0x7F;
/*			if (ipos < 0)
				ipos = 0;
			else if (ipos > 127)
				ipos = 127;*/

			LDAz(cyclebase+ipos)
			STA(0x800+y*40+x)
		}
		if (y >= 9)
		{
//			INC(0xd020);
			RTS();
		}
	}
	RTS();
	PRINTPOS("copyends")
}

}

void irq()
{
	LPC("irqrut");
	PRINTPOS("irqrut");

	LDAi(0x38+3);
	STA(0xd011);

	CLI();


#if 0
	LDXz(ZP("mcc"));
	DEX();
	BEQ(L("domc"))
//	JMP(L("skipmc"))
	LPC("domc");
#endif

//	INC(0xd020);
	movecolors();
//	DEC(0xd020);

	JSR(L("copycolors"))

//		RTS();


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

	value_t first_nmi = 63*14;
	LDAi(LB(first_nmi))
	STA(0xdd04)
	LDAi(HB(first_nmi))
	STA(0xdd05)

	LDAi(0x11);
	STA(0xDD0E);

	LDAi(0x81)
	STA(0xDD0D);
#endif

//	LDXi(20);
//	LPC("skipmc");
//	STXz(ZP("mcc"));

	RTS();

	copycolors();



{
LPC("nmi")
	PHA();
	TXA();
	PHA();
	TYA();
	PHA();
//	LDAz(1)
//	PHA();
//	LDAi(0x35);
//	STAz(1);

#ifdef BORDER_NMI
	LDAi(0x02)
	STA(0xd020);
#endif

	LDXz(paskacounter)

#if 1
	LDAzx(lineptrtab + 0)
	STA(L("jsrmod") + 1)
	LDAzx(lineptrtab + 1)
	STA(L("jsrmod") + 2)

	PRINTPOS("just before jsrmod")

LPC("jsrmod")
	JSR(L("jsrmodend"))

#endif

	LDAz(paskacounter)
	CLC();
	ADCi(2);
	CMPi(0x1E);
	BNE(L("nokillnmi"))

	LDAi(0x00);
	STA(0xDD0E);

	// a paska nyt mutta ei haittaa
LPC("nokillnmi")
	STAz(paskacounter)

//	INC(0xd020);



// VÄLIAIKINEN

//	LDAi(0x7F)
//	STA(0xDD0D);

	LDA(0xDD0D)
#ifdef BORDER_NMI
	LDAi(0xf)
	STA(0xd020);
#endif


//	PLA();
//	STAz(1)
	PLA();
	TAY();
	PLA();
	TAX();
	PLA();

	RTI();

LPC("jsrmodend")
	RTS();

}

}

void putpic(hiresdata_t *pic)
{
	int i;
	for (i = 0; i < 40*25*8; i++)
	{
		int colmix = 0;
		colmix |= pic->m_chars[i];
//		colmix |= pic->bitplanes[i + 1*40*25*8] << 4;
		B(colmix);
//		B(0xAA);
	}
}

#include "jump_if_not_time.h"

void genis()
{
	CTX("main")
	RESERVE(0x00);
	RESERVE(0x01);

	initprg.begin(0x4000);
	set_segment(&initprg);

	LPC("begin");
	PRINTPOS("pc at begin");

	JMP(L("actualbeg"))
LPC("actualbeg")

	MOV16i(framecounter, 0);

	LDAi(0x3C);
	STA(0xDD02);
	LDAi(0x2F);
	STA(0xd018);

	LDAi(0x7F)
	STA(0xDD0D);
	LDA(0xDD0D)


	initcolors();

	LDAi(0x08);
	STA(0xd016);

	//MOV8i(0xd020, 0x02);

	JSR(if_unsetirq1);

	LDXi(LB(L("irqrut")))
	LDYi(HB(L("irqrut")))
	JSR(if_setirq2);



	LPC("loop-1")
	jump_if_not_time(sync_laser_toinenload, L("loop-1"));

//	B(2);


#ifndef NO_LOADING
	JSR(0xC90);
#endif

	LPC("loop-3")
	jump_if_not_time(sync_laser_logo, L("loop-3"));


#ifndef NO_LOADING
	JSR(0xC90);

#endif

//	JSR(if_unsetirq1)
//	JSR(if_unsetirq2)

	LDAi(0x0);
	STA(fadevar);

	LDXi(LB(L("playfade")))
	LDYi(HB(L("playfade")))
	JSR(if_setirq1);


	LPC("loop-2")


	LDA(0xd012);
	EORz(framecounter);
	TAX();
	LDAx(L("randtab"))
	CMPz(fadevar)

	LDAi(0x7F)
	STA(0xDD0D);

	BCC(L("extendruutu"))

	// effu
	LDAi(0x38+3);
	STA(0xd011);
	LDAi(0x2F);
	STA(0xd018);

	JMP(L("end-effu-ruutu"))

LPC("extendruutu")

	// extendruutu
	LDAi(0x1B);
	STA(0xd011);
	LDAi(0x14)
	STA(0xd018);

LPC("end-effu-ruutu")

	LDAi(0x81)
	STA(0xDD0D);
/*
	LDA(0xd011);
	ANDi(0x7);
LPC("waitforonecharline")
	EOR(0xd012);
	BNE(L("waitforonecharline"))
*/
LPC("next_ruutu")

	jump_if_not_time(sync_laser_load_next, L("loop-2"));

#ifndef NO_LOADING
	JSR(if_unsetirq1)
	JSR(if_unsetirq2)
	LDAi(0x7F)
	STA(0xDD0D);
	LDA(0xDD0D)

	// extendruutu
	LDAi(0x1B);
	STA(0xd011);
	LDAi(0x14)
	STA(0xd018);

	LDXi(LB(sync_mohkoexecute))
	LDYi(HB(sync_mohkoexecute))
	JMP(if_executenext);
#endif


	LPC("loop")
	JMP(L("loop"))

	PRINTPOS("pc at code end");

	LPC("randtab")
	{
		int i;
		for (i = 0; i < 256; i++)
		{
			B(rand() & 0xFF);
		}
	}

	//SEG->setpc(0x2000);
	//LPC("pic")
	//putpic(&homo);

	irq();

LPC("playfade")
	// jotain fadevar
	LDAz(fadevar)
	CLC();
	ADCi(1);
	BCC(L("store_fadevar"))
	LDAi(0);

LPC("store_fadevar")
	STAz(fadevar)
	RTS();

}

void maketab(char *rivi, int *ct)
{
	char *riviptr = rivi;
	int count, arvo;

	int pos = 0;

	while(sscanf(riviptr, "%d,%n", &arvo, &count) > 0)
	{
		printf("arvo: %i, count %i\n", arvo, count);
		ct[pos] = arvo;
		pos++;
		riviptr += count;
	}

	cllen = pos;

}

void luevarit(const char *fn)
{
	FILE *f = fopen(fn, "r");
	if (!f)
	{
		printf("cant read colmap\n");
		exit(1);
	}

	char rivi[1024];
	fgets(rivi, 1024, f);
	maketab(rivi, cl2);

	fgets(rivi, 1024, f);
	maketab(rivi, cl3);


	fclose(f);

}


int main(int argc, char **argv)
{

#if 0
	char *colorfn = strdup(argv[1]);
	char *pngfn = strdup(argv[2]);
	char *prgfn = strdup(argv[3]);
#else
	const char *colorfn = "assets/laservarit.txt";
	const char *pngfn = "assets/laserhires.png";
	const char *prgfn = "laser.prg";
#endif

	//luevarit(colorfn);

	homo.load(pngfn);
	homo.tohires();

//	return 1;

	assemble(genis);

	list<segment_c *> segs;
	segs.push_back(&initprg);

//	make_prg("eleneffu.prg", 0x0801, segs);
	make_prg(prgfn, 0x4000, segs);

	return 0;
}
