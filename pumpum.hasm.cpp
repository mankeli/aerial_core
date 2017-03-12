#include "hires.h"

#include <debugf.h>
#include <hashembler.h>

#include <math.h>
#include "lib.h"
#include "memconfig.h"
#include "glm.h"


//#define BORDERTIME

using namespace hashembler;

segment_asm_c actualprg;

float roundjitter(float in)
{
	float dither = 0.5f;
	dither = glm::linearRand(0.f, 1.f);
	if (glm::fract(in) < dither)
		return floor(in);
	else
		return ceil(in);

}


typedef struct
{
	uint8_t sinusex[256];
	uint8_t sinusey[256];
} sinustab_t;

void putsinustab(sinustab_t *tab, int num)
{
	int i;

LPC("sinusex%i", num)
	for (i = 0; i < 256; i++)
	{
		B(tab->sinusex[i]);
	}
LPC("sinusey%i", num)
	for (i = 0; i < 256; i++)
	{
		B(tab->sinusey[i]);
	}

}


void put_to_sinustab(sinustab_t *ste, int i, vec2 pos)
{
	ste->sinusex[i] = pos.x;
	ste->sinusey[i] = pos.y;

}

#include "jump_if_not_time.h"

value_t spriteeka = 0x300/64+5;
value_t spriteperus = 0x300/64+7;
value_t spriteend = 0x300/64+20;


value_t ang  = 0x14; // s2
value_t angv  = 0x16; // s2

value_t sinusmode = 0x40;
value_t nextsinusmode = 0x41;
value_t sinusmode2 = 0x42;
value_t nextsinusmode2 = 0x43;
value_t sinusex = 0x44;
value_t sinusey = 0x45;

value_t cmpval1 = 0x46;
value_t cmpval2 = 0x47;

value_t spritecrazyflag = 0x48;

void msdoseffu()
{
	CTX("valo")

	actualprg.begin(effu_pumpum);
	set_segment(&actualprg);

	LPC("begin");
	PRINTPOS("begin");

	JMP(L("actualbeg"))
LPC("actualbeg")

/*
// vois käyttää samaa vic bankkia ku eka effu?

	LDAi(0x3C);
	STA(0xDD02);
	LDAi(0x2F);
	STA(0xd018);

	LDAi(0x3B)
	STA(0xd011);

	LDAi(0x07);
	STA(0xd016);

*/	

	MOV16zi(angv, 0x200);
	MOV16zi(ang, 0x0);

	LDAi(1);
	STAz(sinusmode)
	LDAi(2);
	STAz(nextsinusmode)
	LDAi(7);
	STAz(sinusmode2)
	LDAi(8);
	STAz(nextsinusmode2)

	LDXi(LB(L("irqrut")))
	LDYi(HB(L("irqrut")))
	JSR(if_setirq1);

//	JSR(if_unsetirq1);


	LDAi(0x88);
	STA(0xd000);
	LDAi(0x94);
	STA(0xd001);
	LDAi(0xe0);
	STA(0xd002);
	LDAi(0x94);
	STA(0xd003);

	JSR(L("setspriteposses"))


//RTS();

	LDAi(spriteperus);
	STA(0x800+0x3f8+0)
	STA(0x800+0x3f8+1)

	LDAi(0x03);
	STA(0xd015);

#ifndef NO_LOADING
	JSR(0xC90);
#endif

	LDYi(0x02);
	STY(0xd027+0)
	STY(0xd027+1)
	LDAi(0);
	STA(0xd020);



{
	int i;
	LDAi(0x0);
	LDXi(0);
	//LDXz(paskapunaiter);
	LPC("normalizeloop1")
	for (i = 0; i < 4; i++)
	{
		STAx(0x800 + i * 0x100)
	}
	INX();
	BNE(L("normalizeloop1"))

	LDAi(spriteperus);
	STA(0x800+0x3f8+0)
	STA(0x800+0x3f8+1)

}


LPC("endless2")

//LPC("endless")
//	JMP(L("endless"))
	jump_if_not_time(sync_pumpumgraffaload, L("endless2"));

	LDAi(0x08);
	STA(0xd016);

#ifndef NO_LOADING
	JSR(0xC90);
#endif

LPC("waitingforcorrectsprite")
	LDX(0x800+0x3f8+0);
	CPXi(spriteend)
	BEQ(L("okbeginpumpum"))
	JMP(L("waitingforcorrectsprite"))

LPC("okbeginpumpum")

	LDAi(0)
	STA(0xd015);

	LDAi(0x0);
	STAz(cmpval1)
	LDAi(0x0);
	STAz(cmpval2)

	JSR(if_unsetirq1)

	LDXi(LB(L("irqrut1")))
	LDYi(HB(L("irqrut1")))
	JSR(if_setirq2);


LPC("endless3")

#if 1

#endif

	jump_if_not_time(sync_laserload, L("endless3"));

	JSR(if_unsetirq1)
	JSR(if_unsetirq2)

{
	int i;
	LDAi(0x22);
	LDXi(0);
	LPC("normalizeloop-tored")
	for (i = 0; i < 4; i++)
	{
		STAx(0x800 + i * 0x100)
	}
	INX();
	BNE(L("normalizeloop-tored"))
}

	//B(2);

LPC("endless4")

#ifndef NO_LOADING
	LDXi(LB(sync_laserexecute))
	LDYi(HB(sync_laserexecute))
	JMP(if_executenext);
#endif

	JMP(L("endless4"))




	LPC("irqrut2");


//	INC(0xd020);

	//LDA(sinusmode)
	//STA(0xd020);

	LDXi(LB(L("irqrut1")))
	LDYi(HB(L("irqrut1")))
	JSR(if_setirq2);

	JSR(L("dolight"))

	CLI();

{
	int i;
	LDAi(0x2);
	LDXi(0);
	//LDXz(paskapunaiter);
	LPC("normalizeloop")
	for (i = 0; i < 4; i++)
	{
		LDYx(0xCC00 + i * 0x100)
		LDAy(0x200)
		//TYA();
		STAx(0x800 + i * 0x100)
		LPC("pah%i", i);
	}
	INX();
	BNE(L("normalizeloop"))
}
	RTS();


LPC("irqrut1")

	LDXi(LB(L("irqrut2")))
	LDYi(HB(L("irqrut2")))
	JSR(if_setirq2);

	LDAz(cmpval1)
	CLC();
	ADCi(0x01);
	ANDi(0x0F);
	STAz(cmpval1)

	CMPi(0x00);
	BNE(L("nozoomstopyet"))

	//LDAi(2);
	//STA(0xd020);

	JSR(if_unsetirq2);

	//LDAi(0x00)
	//STA(0xd011);
	RTS();

LPC("nozoomstopyet")

	LDAz(cmpval2)
	CLC();
	ADCi(0x10);
	ANDi(0xF0);
	STAz(cmpval2)

	JSR(L("dolight"))

	CLI();

LDXi(0);
LPC("coltabismakeloop1")
	TXA();
	ANDi(0x0F);
	CMPz(cmpval1);
	LDAi(0x00);
	BCS(L("coltabismakeloop1_store"))
	LDAi(0x02);
LPC("coltabismakeloop1_store")
	STAx(0x200);

	TXA();
	ANDi(0xF0);
	CMPz(cmpval2);
	LDAx(0x200);
	BCS(L("coltabismakeloop2_store"))
	ORAi(0x20);
LPC("coltabismakeloop2_store")
	STAx(0x200);


	INX();
	BNE(L("coltabismakeloop1"))

RTS();

	LPC("irqrut");
	JMP(L("dolight"));


#if 0
LPC("coltabis")
{
	int i;
	for (i = 0; i < 256; i++)
	{
		int e1 = (i >> 4) & 0x0F;
		int e2 = (i >> 0) & 0x0F;
		int asd = 0;
		if (e1 <= 0x0A)
			asd |= 0x20;
		if (e2 <= 0x0A)
			asd |= 0x02;
		B(asd)
	}
}
#endif
#if 0
{

	int i;
	for (i = 0; ;i++)
	{
		LPC("irqswitch%i", i);
		value_t next = L("irqswitch%i",i+1);
		switch(i)
		{
			case 0:
				jump_if_not_time(sync_eyegravity, next);
				JSR(L("initlight"))
				LDAi(1);
				STAz(sinusmode)
				LDAi(4);
				STAz(nextsinusmode)

				break;
			case 1:
				goto irqswitchloop_end;
				break;
		}

		//JMP(L("returnfromirq"))
	}
	irqswitchloop_end:;

LPC("returnfromirq")
	RTS();
}
#endif

LPC("sinuslinks")
B(0); // 0

B(2); // 1
B(3); // 2
B(4); // 3
B(5); // 4
B(6); // 5
B(6); // 6

B(8); // 7
B(9); // 8
B(10); // 9
B(11);
B(12);
B(12);


LPC("loadsinus")
	int num;
	for (num = 1; num <= 12; num++)
	{
		LPC("getsinus%i", num);

		CMPi(num)
		BNE(L("getsinus%i", num + 1))

		PRINTPOS("getsinus");
		LDAx(L("sinusex%i", num))
		STAz(sinusex)
		LDAx(L("sinusey%i", num))
		STAz(sinusey)

		JMP(L("sinusgot"))
	}
	LPC("getsinus%i", num);
	LPC("sinusgot")
	RTS();


LPC("dolight")
	CLI();


	ADD16z(ang, angv);

	BCC(L("nosm2yet"))
	LDAz(nextsinusmode);
	STAz(sinusmode)

	CMPi(0);
	BNE(L("getnextsinusmode"))
		JSR(if_unsetirq1);
		RTS();
	LPC("getnextsinusmode")
	TAX();

	LDAx(L("sinuslinks"))
	STAz(nextsinusmode);

	LDAz(nextsinusmode2);
	STAz(sinusmode2)
	TAX();
	LDAx(L("sinuslinks"))
	STAz(nextsinusmode2);

	LPC("nosm2yet")

// tätä kutsutaan myös erillään
LPC("setspriteposses")
	LDXz(ang+1)

	LDAz(sinusmode)
	JSR(L("loadsinus"));
	LDAz(sinusex)
	STA(0xd000);
	LDAz(sinusey)
	STA(0xd001);


	LDXz(ang+1)
	LDAz(sinusmode2)
	JSR(L("loadsinus"));
	LDAz(sinusex)
	STA(0xd002);
	LDAz(sinusey)
	STA(0xd003);

	jump_if_not_time(sync_pumpum_crazysprite, L("nocrazyspriteenable"));
	LDAi(1)
	STAz(spritecrazyflag)
LPC("nocrazyspriteenable")

// animate crazy sprites, or don't animate
	LDX(0x800+0x3f8+0);

	LDAz(spritecrazyflag)
	BEQ(L("storesprnum"))

	INX();
	CPXi(spriteend+1)
	BNE(L("storesprnum"))
	LDXi(spriteeka)
LPC("storesprnum")
	STX(0x800+0x3f8+0)
	STX(0x800+0x3f8+1)


	RTS();





sinustab_t st[12];

vec2 co = vec2(15.f, 12.f);

int i,j;

{
	vec2 lp1 = vec2(0x8C, 0x8D);
	vec2 lp2 = vec2(0xe4, 0x8d);

	vec2 lp1v = vec2(0.f);
	vec2 lp2v = vec2(0.f);

	//vec2 lpt = vec2(0x80,0x40);
	vec2 lpt = vec2(23+160-10,30+100+12);

	vec2 lp1hist[256];
	vec2 lp2hist[256];

	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 256; j++)
		{
			float fb1 = (float)j/(float)0xFF;
			float fb = (float)j/(float)0xFF + (float)i;

			lp1v += (lpt - lp1) * 0.003f;
			lp2v += (lpt - lp2) * 0.003f;

			vec2 lp1_o = lp1;
			vec2 lp2_o = lp2;

			lp1 += lp1v;
			lp2 += lp2v;

			vec2 pv = lp2 - lp1;

			if (fb < 3.0f)
			{
				float meno = glm::length(pv) - 10.f;
				if (meno < 0.f)
				{
					vec2 pd = glm::normalize(pv);
					meno = 1.f;
					float meno2 = fabs(meno) * 5.0f;
					lp1 -= pd * meno2;
					lp2 += pd * meno2;
				}

				lp1 += glm::linearRand(vec2(-1.f), vec2(1.f)) * 0.1f;
				lp2 += glm::linearRand(vec2(-1.f), vec2(1.f)) * 0.1f;
			}

			if (i < 2)
			{
				lp1 += glm::linearRand(vec2(-1.f), vec2(1.f)) * 0.5f;
				lp2 += glm::linearRand(vec2(-1.f), vec2(1.f)) * 0.5f;
			}
			else if (i == 3)
			{
				lp1 += glm::linearRand(vec2(-1.f), vec2(1.f)) * 0.7f * (1.f - fb1);
				lp2 += glm::linearRand(vec2(-1.f), vec2(1.f)) * 0.7f * (1.f - fb1);
			}

			lp1.y = glm::clamp(lp1.y, 30.f, 180.f);
			lp2.y = glm::clamp(lp2.y, 30.f, 180.f);

			float kullidampen = glm::clamp(fb / 0.3f, 0.f, 1.f);
			lp1 = glm::mix(lp1_o, lp1, kullidampen);
			lp2 = glm::mix(lp2_o, lp2, kullidampen);


			lp1v = lp1 - lp1_o;
			lp2v = lp2 - lp2_o;

			lp1v *= 0.97f;
			lp2v *= 0.97f;

			float ya = fb;
			vec2 jit = vec2(0.f);
			//jit += vec2(sin(ya*9.f), cos(ya*8.f)) * 0.5f;


			if (i == 5)
			{
				lp1 = glm::mix(lp1, lp1hist[j], fb1);
				lp2 = glm::mix(lp2, lp2hist[j], fb1);
			}

			//lp2 = lp1 + vec2(30.f, 10.f);
			put_to_sinustab(&st[i], j, lp1);
			put_to_sinustab(&st[i+6], j, lp2);

			lp1hist[j] = lp1;
			lp2hist[j] = lp2;
		}
	}
}

	PRINTPOS("end of code")

	PAGE;

	for (i = 0; i < 12; i++)
	{
		putsinustab(&st[i], i + 1);
	}

	PRINTPOS("some tables end")

}




void ekat()
{
	hiresdata_t rajahdys;
	int x,y,i;

	rajahdys.init(ivec2(320,200));
	for (y = 0; y < 200; y++)
	{
		for (x = 0; x < 320; x++)
		{
				vec2 midv = vec2(x,y) - vec2(160.f, 100.f);
				//float dist = 1000.f / glm::length(midv);
				//float dist = glm::length(midv) * 0.084f;
				//dist = roundjitter(dist);

				float dfc = glm::length(midv) * 0.00f;

				float v = 1.0;
				int a;
				for (a = 0; a < 5; a++)
				{
					float ra = (a+dfc) * (M_PI/5.f) * 2.f;
					v = glm::max(glm::dot(midv,vec2(cos(ra), sin(ra))), v);
				}

			//v *= 1.0f;
			//v /= glm::min(1.f, i / 20.f) * 2.0f;
				float dist = v * 0.082f / 0x0F;
				dist = 16.f - 16.f / (1.f+dist);
				//dist = powf(dist, 2.f);
				dist = roundjitter(dist);

				dist = glm::clamp(dist, 0.f, 15.f);


				int arvo = (int)dist & 0x0F;
				//arvo = 0xDE;
			rajahdys.m_pic[y * 320 + x] = arvo;
		}
	}
	rajahdys.tohires();



	FILE *f = fopen("pumpum.hires", "wb");

	for (y = 0; y < 25; y++)
	{
		for (x = 0; x < 40; x++)
		{
			for (i = 0; i < 8; i++)
			{
				int arvo;
				arvo = rajahdys.m_chars[(y * 40 + x) * 8 + i];
				arvo &= 0xFF;
				fputc(arvo, f);
			}
		}
	}

	for (i = 0; i < 40*25;i++)
	{
		//B(0x20);
		int cval = ((rajahdys.m_col2[i] & 0x0F) << 4) | (rajahdys.m_col1[i] & 0x0f);
		//int cval = rajahdys.m_col1[i] & 0x0f;
		fputc(cval, f);
	}

	fclose(f);

	#if 0
	pallo.load("assets/pallo.png");
	pallo.tohires();

	kallo.load("assets/kallo.png");
	kallo.tohires();


	int colcombs[256];
	memset(colcombs, 0, sizeof(int)*256);

	int i,j;
	for (i = 0; i < 40*25;i++)
	{
		//B(0x20);
		int cval;
		cval = ((pallo.m_col2[i] & 0x0F) << 4) | (pallo.m_col1[i] & 0x0f);
		colcombs[cval]++;

		cval = ((kallo.m_col2[i] & 0x0F) << 4) | (kallo.m_col1[i] & 0x0f);
		colcombs[cval]++;
	}

	colcombs[0x11]++;

	for (i = 0; i < 256; i++)
	{
		if (colcombs[i] > 0)
		{
			printf("comb %02X: %i instances\n", i, colcombs[i]);
		}
	}
#endif
}

void genis()
{
	msdoseffu();
}


int main()
{
	srand(time(NULL));

	ekat();
	assemble(genis);

	list<segment_c *> segs;
	segs.push_back(&actualprg);
	make_prg("pumpum.prg", effu_pumpum, segs);
}