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
segment_asm_c spriteprg;

hiresdata_t pallo;
hiresdata_t kallo;


ivec2 lightsize = ivec2(22,22);
//#define SEELIGHT

float getlight(int x,int y)
{
	x %= lightsize.x;
	y %= lightsize.y;
//	int cv = valo[y * lightsize.x + x] - '0';
//	return (float)cv / 3.f;
	vec2 lp = vec2(x,y) - vec2(lightsize) / 2.f;
	float lv = 1.f - glm::clamp(-0.0f + glm::length(lp)/16.0f, 0.0f, 1.f);
	
	//lv = pow(lv, 0.8f);
	//lv += glm::linearRand(0.01f, 0.6f);
	//lv *= 1.6f;
	lv = glm::clamp(lv, 0.f, 1.f);
	lv = powf(lv, 2.4f);
	//lv = 1.0f;
	return lv;
}

float roundjitter(float in)
{
	float dither = 0.5f;
	dither = glm::linearRand(0.f, 1.f);
	if (glm::fract(in) < dither)
		return floor(in);
	else
		return ceil(in);

}

int lighttab_count = 12;

int lighttab[2048];
int lighttab_size;
int lighttab_offsets[6];

typedef struct
{
	uint8_t sinusl[256];
	uint8_t sinush[256];
	uint8_t sinusyc[256];
	uint8_t sinusxc[256];
	uint8_t sinuse1x[256];
	uint8_t sinuse2x[256];
	uint8_t sinusey[256];
} sinustab_t;

void putsinustab(sinustab_t *tab, int num)
{
	int i;
LPC("sinusl%i", num)
	for (i = 0; i < 256; i++)
	{
		B(tab->sinusl[i]);
	}
LPC("sinush%i", num)
	for (i = 0; i < 256; i++)
	{
		B(tab->sinush[i]);
	}
LPC("sinusyc%i", num)
	for (i = 0; i < 256; i++)
	{
		B(tab->sinusyc[i]);
	}
LPC("sinusxc%i", num)
	for (i = 0; i < 256; i++)
	{
		B(tab->sinusxc[i]);
	}

LPC("sinuse1x%i", num)
	for (i = 0; i < 256; i++)
	{
		B(tab->sinuse1x[i]);
	}
LPC("sinuse2x%i", num)
	for (i = 0; i < 256; i++)
	{
		B(tab->sinuse2x[i]);
	}
LPC("sinusey%i", num)
	for (i = 0; i < 256; i++)
	{
		B(tab->sinusey[i]);
	}

}


void put_to_sinustab(sinustab_t *ste, int i, vec2 pos)
{
	int xd = pos.x;
	int yd = pos.y;
//		if (xd < 0)
//			xd = 0;
		if (xd > (39-lightsize.x))
			xd = 39-lightsize.x;

		vec2 sprsize = vec2(12.f,10.5f);
		vec2 eye1 = vec2(0x88,0x94) + sprsize;
		vec2 eye2 = vec2(0xe3,0x94) + sprsize;

		vec2 lp_f = pos * 8.f + vec2(110.f, 140.f);
		eye1 = glm::mix(eye1, lp_f, 0.04f);
		eye2 = glm::mix(eye2, lp_f, 0.04f);

		float ya = i * (M_PI / 128.f);
		eye1 += vec2(sin(ya*9.f), cos(ya*8.f)) * 0.5f;
		eye2 += vec2(sin(ya*8.f), cos(ya*9.f)) * 0.5f;
		eye1 += glm::linearRand(vec2(-1.f), vec2(1.f)) * 0.5f;
		eye2 += glm::linearRand(vec2(-1.f), vec2(1.f)) * 0.5f;
		ste->sinuse1x[i] = eye1.x - sprsize.x;
		ste->sinuse2x[i] = eye2.x - sprsize.x;
		ste->sinusey[i] = (eye1.y + eye2.y) * 0.5f - sprsize.y;

		value_t offval = yd * 40 + xd;
		ste->sinusl[i] = LB(offval);
		ste->sinush[i] = HB(offval);

		ste->sinusyc[i] = (yd+256) & 0xFF;
		ste->sinusxc[i] = 0;

		//xd = glm::clamp(xd, -lightsize.x, 40);
		if (xd < 0)
			ste->sinusxc[i] = glm::clamp(-xd, 0, lightsize.x);
}


void playmusic()
{
	INC(0xd020);
	JSR(0x1003);
	DEC(0xd020);
}

#include "jump_if_not_time.h"

	//value_t gfxmemout = 0x4000 + 0x1c00;
	value_t gfxmemout = 0x0000 + 0x800;
	value_t gfxmemin  = 0x4000 + 0x1800;


value_t writeptr = 0x10; // s 2
value_t readptr = 0x12; // s 2
value_t ang  = 0x14; // s2
value_t angv  = 0x16; // s2
value_t yforclip = 0x18;
value_t xforclip = 0x19;

value_t adrcalctemp = 0x1A; // s2

value_t lightflag = 0x1C;
value_t fadepos = 0x1D;

value_t spriteang = 0x1E;

value_t sinusmode = 0x1F;
value_t nextsinusmode = 0x20;
value_t sinuse1x = 0x21;
value_t sinuse2x = 0x22;
value_t sinusey = 0x23;

void msdoseffu()
{
	CTX("valo")

	gfxmemin = L("gfxmemin");

	printf("gfxmemin: %04X\n", gfxmemin);



	actualprg.begin(0x4000);
	set_segment(&actualprg);

	LPC("begin");
	PRINTPOS("begin");

	JMP(L("actualbeg"))
LPC("actualbeg")
	JSR(if_unsetirq1);
	JSR(if_unsetirq2)

	JSR(if_startmusic)

	MOV16i(framecounter, 0);

// vois käyttää samaa vic bankkia ku eka effu?

	LDAi(0);
	STA(0xd015);

	LDAi(0x3C);
	STA(0xDD02);
	LDAi(0x2F);
	STA(0xd018);


	LDAi(0x3B)
	STA(0xd011);

	LDAi(0x07);
	STA(0xd016);

	//MOV16zi(angv, (0x10000/2)+0xA0);

	MOV8zi(lightflag, 0);


	LDXi(LB(L("irqrut")))
	LDYi(HB(L("irqrut")))
	JSR(if_setirq1);

	JSR(if_unsetirq2)


LPC("endless")


/*
	LDAz(lightflag)
	ANDi(0x02)
	BEQ(L("endless"))

	LDAz(lightflag)
	ANDi(0x01)
	BEQ(L("endless"))

	JSR(L("dolight"))

	LDAz(lightflag)
	ANDi(~0x01)
	STAz(lightflag)
*/

//	JMP(L("endless"))

	/*
	here's an idea:
	irq loppuun olisi mahdollista säätää "return address" eli tässä tapauksessa vaikkapa
	suoraan tuo L("dolight")  ja sitten irq rutiini menisi ensin tekemään tuon, ja sitten
	dolightin RTS palauttaisi takaisin päärutiiniin.. eli esimerkiksi loaderiin
	*/

	// jotain fiksumpaa myös tähän ?? ei jaksa ladata näin tyhmästi vaan pitäis olla
	// joku MANAGERI
	// cmp16i ??
	jump_if_not_time(sync_kalloload, L("endless"));

#if 1
	LDAi(0x0)
	STA(0xd015)
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
}
#endif
	

#ifndef NO_LOADING
	JSR(0xC90);
#endif

LPC("endless2")

	//LDAi(0)
	//STA(0xd020);

//	JMP(L("endless2"))
	jump_if_not_time(sync_eyegravity, L("endless2"));


	LDAi(0x7F);
	STAz(fadepos)
	LDXi(LB(L("dofade")))
	LDYi(HB(L("dofade")))
	JSR(if_setirq2);


	JMP(effu_pumpum);

//	JSR(if_unsetirq1)
//	JSR(if_unsetirq2)
//	//B(2);
//	JSR(0xC90);
//	JMP(0xFFF0);


	LPC("irqrut");

#if 0
	LDAi(LB(gfxmemout))
	STAz(writeptr+0)
	LDAi(HB(gfxmemout))
	STAz(writeptr+1)
#endif

	LDX(0xdc00)
	TXA();
	ANDi(0x4)
	BNE(L("testleft"))
	ADD16zi(angv, 0x0030);
	JMP(L("nojoy"))
	LPC("testleft")

	TXA();
	ANDi(0x8)
	BNE(L("nojoy"))
	ADD16zi(angv, 0xFFcF);


LPC("nojoy")

{
//	TXA();
	int i;
	for (i = 0; ;i++)
	{
		LPC("irqswitch%i", i);
		value_t next = L("irqswitch%i",i+1);
		switch(i)
		{
			case 0:
				jump_if_not_time(sync_lightappear2, next);
				JSR(L("initlight"))
				LDAi(1);
				STAz(sinusmode)
				LDAi(4);
				STAz(nextsinusmode)

				break;
			case 1:
				jump_if_not_time(sync_lightchangea, next);
				LDAi(4);
				STAz(nextsinusmode)
				break;
			case 2:
				jump_if_not_time(sync_lightrecedea, next);
				LDAi(3);
				STAz(nextsinusmode)
				break;
			case 3:
				//jump_if_not_time(sync_eyegravity, next);
				//JSR(effu_pumpum);
				break;
			case 4:
				jump_if_not_time(sync_lightrecedeb, next);
				LDAi(3);
				STAz(nextsinusmode)
				break;
			case 5:
				jump_if_not_time(sync_lightappear, next);
				LDAi(1);
				STAz(sinusmode)
				LDAi(2);
				STAz(nextsinusmode)
				JSR(L("initlight"))
			case 6:
				jump_if_not_time(sync_sprites, next);
				JSR(L("initsprites"))
			case 7:
				jump_if_not_time(sync_sprites2, next);
				JSR(L("initsprites2"))
			case 8:
				jump_if_not_time(sync_fade, next);
				// fade effu disabled
				LDAi(0x7F);
				STAz(fadepos)
				LDXi(LB(L("dofade")))
				LDYi(HB(L("dofade")))
				JSR(if_setirq2);
				break;
			case 9:
				goto irqswitchloop_end;
				break;
		}

		//JMP(L("returnfromirq"))
	}
	irqswitchloop_end:;

LPC("returnfromirq")
	LDAz(lightflag)
	ORAi(0x01);
	STAz(lightflag)


	RTS();
}

 // SPRITE EFFU

value_t spriteangoffset = 0x05;

{
	int i;
	int sproffslen = spriteangoffset * 8;
	int sprstop = 0xFF - sproffslen * 2;
	int spritenums[256];
	LPC("spritesinus")
	for(i = 0; i < 0x100; i++)
	{
		int i2 = i - sproffslen + 256;
		i2 &= 0xFF;
		i2 = glm::clamp(i2, 0, sprstop);

		float v = 0.f;
		v += (float)i2 / (float)sprstop;
		float nv = v;

		float piscl = (M_PI / sprstop) * 2.f;;
		//v += sin(1.f * i * M_PI / 128.f);
		v += sin(3.f * i2 * piscl) * 0.3f;
		v += sin(4.f * i2 * piscl) * 0.1f;

		v *= 0x30;
		int xd = 0xA0 + v - 11;
		B(xd)


/*		nv *= 2.f;
		if (nv > 1.0f)
			nv = 2.0f - nv;
		nv = pow(nv, 2.f);
*/
		int byt = roundjitter(nv * 21.f);
		byt = glm::clamp(byt-1, -1, 20);

		if (byt == 20)
			byt = -1;
		// -1, 19, 18..
		//byt = 19;
		byt += 0x300/64;
		spritenums[i] = byt;

	}

	LPC("spritenums")
	for(i = 0; i < 0x100; i++)
	{
		B(spritenums[i]);
	}

	LPC("initsprites2")

	LDAi(0x88);
	STA(0xd000);
	LDAi(0x94);
	STA(0xd001);
	LDAi(0xe0);
	STA(0xd002);
	LDAi(0x94);
	STA(0xd003);

	LDAi(0x300/64+0);
	STA(0x800+0x3f8+0)
	STA(0x800+0x3f8+1)

	LDAi(0x03);
	STA(0xd015);



	LDXi(LB(L("dospritemove2")))
	LDYi(HB(L("dospritemove2")))
	JSR(if_setirq2);

	RTS();

LPC("dospritemove2")
	JSR(L("dolight"))

	LDX(0x800+0x3f8+0)
	INX();
	CPXi(7+0x300/64)
	BEQ(L("just_set_eyeposses"))
	STX(0x800+0x3f8+0)
	STX(0x800+0x3f8+1)
LPC("just_set_eyeposses")

	LDAz(sinuse1x)
	STA(0xd000);
	LDAz(sinuse2x)
	STA(0xd002);
	LDAz(sinusey)
	STA(0xd001);
	STA(0xd003);
	RTS();

	LPC("initsprites")
{
	int i;
	// spraits!
	LDAi(0xD0)
	LDYi(0x02);
	for(i = 0; i < 0x8; i++)
	{
		//LDXi(i * 24 + 0x40)
		//23 + i*48
		int xoffs = (320-8*24) / 2 + 23;
		LDXi(i * 24 + xoffs)
		STX(0xd000+i*2+0)
		STA(0xd000+i*2+1)
		STY(0xd027+i)
	}

	LDAi(0x00)
	STA(0xd010)

	LDAi(0x300/64-1);
	LDXi(7)
LPC("sprblanking")
	STAx(0x800+0x3f8)
	DEX();
	BPL(L("sprblanking"))

	LDAi(0xFF);
	STA(0xd015);



	// double height
	LDAi(0x00);
	STA(0xd01d);
	STA(0xd017);

	// priority
	LDAi(0x00);
	STA(0xd01b);
 	LDAi(0);
	STAz(spriteang)

	LDXi(LB(L("dospritemove")))
	LDYi(HB(L("dospritemove")))
	JSR(if_setirq2);

}
	RTS();

	LPC("dospritemove")

	CLI();

	LDAz(spriteang)
	CLC();
	ADCi(0x01);
	STAz(spriteang)
	BNE(L("nosprstop"))
//	B(2);
	// LDAi(0);
	STA(0xd015);
	JSR(if_unsetirq2);
	RTS();

LPC("nosprstop");

	for(i = 0; i < 0x8; i++)
	{
		CLC();
		ADCi(spriteangoffset);
		TAX();
		LDYx(L("spritesinus"))
		STY(0xd000+i*2+1)

		LDYx(L("spritenums"))
		//LDYi( + 5)
		//LDYi(0x300/64+0)
		STY(0x800+0x3f8+i)

	}

	RTS();
}


{
	int i;

	LPC("roundjittertab")
	for(i = 0; i < 0x7f; i++)
	{
		float v = (float)i/(float)0x7F - 0.03f;
		v = glm::clamp(v, 0.f, 1.f);
		int iv = roundjitter(v * 2.f);
		//printf("%i: %.2f\n", i, v);
		B(iv);
	}

	LPC("ltptrs_1")
	for(i = 0; i < lighttab_count; i++)
	{
		value_t memmy = L("lighttab") + lighttab_offsets[i];
		B(LB(memmy));
	}
	LPC("ltptrs_2")
	for(i = 0; i < lighttab_count; i++)
	{
		value_t memmy = L("lighttab") + lighttab_offsets[i];
		B(HB(memmy));
	}

}

#if 1

LPC("dofade")
	CLI();


	LDAz(fadepos)
	SEC();
	SBCi(3);
	STAz(fadepos)
	/*LSRa();
	LSRa();
	LSRa();
	LSRa();
	ANDi(0x07);
	TAX();*/
	TAX();

	BPL(L("justsetcolor"))
	JSR(if_unsetirq2);
	RTS();
LPC("justsetcolor")

	LDAx(L("roundjittertab"))
	TAX();

	LDAx(L("ltptrs_1"))
	STAz(writeptr)
	LDAx(L("ltptrs_2"))
	STAz(writeptr+1)

{
	LDXi(0x0F);
LPC("maploop")
PRINTPOS("maploop")


	int i,x,y;

	int r1[25];
	int r2[25];

	for(y = 0; y < 25; y++)
	{
		int first = 40, last = -1;
		for(x = 0; x < 40; x++)
		{
			int idx = y * 40 + x;
			int cval;
			cval = ((pallo.m_col2[idx] & 0x0F) << 4) | (pallo.m_col1[idx] & 0x0f);


			if (cval && x < first)
				first = x;
			else if (cval && x > last)
				last = x;
		}
		r1[y] = first;
		r2[y] = last;
	}

	for(y = 0; y < 25; y++)
	{
		int o = r1[y];

		while(o < r2[y])
		{
			LDAx(gfxmemin + y*40+o)
			TAY();
			LDAizy(writeptr)
			STAx(gfxmemout + y*40+o)
			o += 0x10;
		}

	}
	
PRINTPOS("maploop-cont")

	DEX();
	BMI(L("outthiscycle"))

	JMP(L("maploop"))
}
LPC("outthiscycle")
RTS();
#endif

// LIGHT EFFU

LPC("initlight")
	LDXi(LB(L("dolight")))
	LDYi(HB(L("dolight")))
	JSR(if_setirq2);

	MOV16zi(angv, 0x220);
	MOV16zi(ang, 0x0);

	RTS();

// ACTAUL

LPC("sinuslinks")
B(0);
B(2);
B(2);
B(0);
B(5);
B(6);
B(2);

LPC("dolight")
	CLI();


	ADD16z(ang, angv);

	BCC(L("nosm2yet"))
	LDAz(nextsinusmode);
	STAz(sinusmode)

	CMPi(0);
	BNE(L("getnextsinusmode"))
		JSR(if_unsetirq2);
		RTS();

LPC("getnextsinusmode")
	TAX();
	LDAx(L("sinuslinks"))
	STAz(nextsinusmode);

	LPC("nosm2yet")


	LDXz(ang+1)

	LDAz(sinusmode)

	int num;
	for (num = 1; num <= 6; num++)
	{
		LPC("getsinus%i", num);

		CMPi(num)
		BNE(L("getsinus%i", num + 1))

		PRINTPOS("getsinus");
		LDAx(L("sinusl%i", num))
		STAz(writeptr+0)
		STAz(readptr+0)
		LDAx(L("sinush%i", num))
		STAz(writeptr+1)
		STAz(readptr+1)
		LDAx(L("sinusyc%i", num))
		STAz(yforclip)
		LDAx(L("sinusxc%i", num))
		STAz(xforclip)

		LDAx(L("sinuse1x%i", num))
		STAz(sinuse1x)
		LDAx(L("sinuse2x%i", num))
		STAz(sinuse2x)
		LDAx(L("sinusey%i", num))
		STAz(sinusey)

		JMP(L("sinusgot"))
	}
	LPC("getsinus%i", num);
	LPC("sinusgot")

	ADD16zi(writeptr, gfxmemout);
	ADD16zi(readptr, gfxmemin);

	LDYi(0);

	int x,y,i;

	int yregemu = 0;

	for (y = 0; y < lightsize.y; y++)
	{
#ifdef BORDERTIME
		INC(0xd020);
#endif

		LDAz(yforclip)
		BPL(L("testlow%i", y))
		JMP(L("linedone%i", y))
LPC("testlow%i", y)
		CMPi(25);
		BCC(L("realline%i", y))
		//DEC(0xd020);
		JMP(L("drawroutine_end"))
LPC("realline%i", y);

		yregemu = 0;
//		LDYi(0);
		LDYz(xforclip);
//		JMP(L("drawpixel%i,%i", 0, y));

		TYA();
		STAz(adrcalctemp+0);
		ASLa();
		ASLa();
		ASLa();
		CLC();
		ADCz(adrcalctemp+0)
		// carry should be 0
		ADCi(LB(L("drawpixel%i,%i", 0, y)));
		STAz(adrcalctemp+0)
		LDAi(0)
		ADCi(HB(L("drawpixel%i,%i", 0, y)));
		STAz(adrcalctemp+1)

		JMPi(adrcalctemp);

		for (x = 0; x < lightsize.x; x++)
		{
		//B(0x20);
			LPC("drawpixel%i,%i", x, y);
			float lv = getlight(x,y);

#if 1
			//int lvi = lv * lighttab_count;
			//int lvi = roundjitter(lv * (lighttab_count-3) + 3);
			//int lvi = roundjitter(lv * (lighttab_count-0) + 0);
			//int lvi = floor(lv * lighttab_count);
			int lvi = floor(lv * (lighttab_count-2) + 2);

			//lvi = lighttab_count - 1;

			lvi = glm::clamp(lvi, 0, lighttab_count - 1);

#ifndef SEELIGHT
			LDAizy(readptr)
#else
			LDAi(0x11);
#endif
			TAX();
			// optimization: if lvi == 0, LDAi(0) here!
			LDAx(L("lighttab") + lighttab_offsets[lvi])
#else
			int c1,c2;
			c1 = mapcolor(lv, mapping, 6);
			c2 = mapcolor(lv, mapping, 6);
			int cval = (c1 << 4) | c2;
			LDAi(cval);
#endif
			STAizy(writeptr)
			INY();
			yregemu++;
		}

		for (x = 0; x < lightsize.x; x++)
		{
			value_t ad, adprev, de = 0xFFFF;
			ad = L("drawpixel%i,%i", x, y);
			if (x >= 1)
			{
				adprev = L("drawpixel%i,%i", x - 1, y);
				de = ad - adprev;
			}

			//printf("address of %i,%i: %04X, delta %04X\n", x, y, ad, de);

		}


		LPC("linedone%i", y);


		int stride = 40 - lightsize.x;
		yregemu += stride;

		LDAz(writeptr+0)
		CLC();
		ADCi(LB(yregemu))
		STAz(writeptr+0)
		LDAz(writeptr+1)
		ADCi(HB(yregemu))
		STAz(writeptr+1)

		LDAz(readptr+0)
		CLC();
		ADCi(LB(yregemu))
		STAz(readptr+0)
		LDAz(readptr+1)
		ADCi(HB(yregemu))
		STAz(readptr+1)

		INCz(yforclip)
	}

LPC("drawroutine_end")

	//INC(0xd016)
	#if 0
	LDAizy(writeptr)
	CLC();
	ADCi(1);
	STAizy(writeptr)
	#endif

	RTS();


sinustab_t st1;
sinustab_t st2;
sinustab_t st3;

sinustab_t st4;
sinustab_t st5;
sinustab_t st6;

vec2 co = vec2(15.f, 12.f) - vec2(lightsize) / 2.f;


	for (i = 0; i < 256; i++)
	{
		float v1 = sin(1.f * i * M_PI / 128.f);
		float v2 = cos(1.f * i * M_PI / 128.f);

		//v1 += cos(5.f * i * M_PI / 128.f + 3.2f) * 0.1f;
		//v2 += sin(3.f * i * M_PI / 128.f + 2.92f) * 0.1f;


		float av1 = 1.4432f;
		float av2 = 1.12612f;
		float v3a = sin(av1 *  (i + 256.f*0) * M_PI / 128.f);
		float v3b = sin(av1 *  (i + 256.f*1) * M_PI / 128.f);
		float v3c = sin(av1 *  (i + 256.f*2) * M_PI / 128.f);
		float v4a = cos(av2 * (i + 256.f*0) * M_PI / 128.f);
		float v4b = cos(av2 * (i + 256.f*1) * M_PI / 128.f);
		float v4c = cos(av2 * (i + 256.f*2) * M_PI / 128.f);

		vec2 lp2 = vec2(v1,v2);
		vec2 lp4 = vec2(v3a,v4a);
		vec2 lp5 = vec2(v3b,v4b);
		vec2 lp6 = vec2(v3c,v4c);

		lp2 *= vec2(13.f, 12.f);
		lp4 *= vec2(13.f, 12.f);
		lp5 *= vec2(13.f, 12.f);
		lp6 *= vec2(13.f, 12.f);


		//v1 = roundjitter(v1);
		//v2 = roundjitter(v2);


		float fb = (float)i/(float)0xFF;

		//vec2 lp1 = glm::mix(vec2(0.f, 19.f), lp, fb);
		vec2 lp1 = lp2 + vec2(0.f, 15.f) * (1.f - fb);
		//vec2 lp3 = lp2 + vec2(0.f, 15.f) * fb;
		vec2 lp3 = glm::mix(lp2, vec2(15.f, -22.f), fb);

		put_to_sinustab(&st1, i, lp1+co);
		put_to_sinustab(&st2, i, lp2+co);
		put_to_sinustab(&st3, i, lp3+co);

		lp4 = glm::mix(lp2, lp4, fb);
		lp6 = glm::mix(lp6, lp2, fb);

		put_to_sinustab(&st4, i, lp4+co);
		put_to_sinustab(&st5, i, lp5+co);
		put_to_sinustab(&st6, i, lp6+co);

	}

	PRINTPOS("end of code")


/*

	SEG->setpc(	);
*/

	//PAGE

	SEG->setpc(0x6000);
	PRINTPOS("gfxmemin is here?")

LPC("gfxmemin")

	for (y = 0; y < 25; y++)
	{
		for (x = 0; x < 40; x++)
		{

			int idx = y * 40 + x;

			int cval = ((pallo.m_col2[idx] & 0x0F) << 4) | (pallo.m_col1[idx] & 0x0f);
			//cval = 11;
			B(cval);

		}
	}

	PAGE;

	PRINTPOS("some tables")


	putsinustab(&st1, 1);
	putsinustab(&st2, 2);
	putsinustab(&st3, 3);
	putsinustab(&st4, 4);
	putsinustab(&st5, 5);
	putsinustab(&st6, 6);
LPC("lighttab")
	for (i = 0; i < lighttab_size; i++)
		{
			B(lighttab[i]);
		}
	PRINTPOS("end of tables")


/*
LPC("muna")
	INC(0xd020);
	JMP(L("muna"))
*/
}


void ekat()
{
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

	int lighttab_used[2048];
	memset(lighttab_used, 0, 2048*sizeof(int));

	int lastused = 0;

	int mapoffs = 0;
	for (i = 0; i < lighttab_count; i++)
	{
		mapoffs = 0;
		trytabmakingagain:;

		for (j = 0; j < 256; j++)
		{
			int ltpos = j + mapoffs;

			if (colcombs[j] > 0)
			{
				if (lighttab_used[ltpos])
				{
					mapoffs++;
					goto trytabmakingagain;
				}
			}
		}

		lighttab_offsets[i] = mapoffs;
		printf("ok, putting mapping %i to offs %i\n", i, mapoffs);

		for (j = 0; j < 256; j++)
		{
			int ltpos = j + mapoffs;

			if (colcombs[j] > 0)
			{
				int c1o = (j >> 4) & 0x0F;
				int c2o = (j >> 0) & 0x0F;

				//float lv = (float)i / (float)(lighttab_count - 1);
				//float c1f = grey_to_bright[c1o];
				//float c2f = grey_to_bright[c2o];

				int grey_to_num[] =
				{
					0,3,9,9,
					9,9,9,9,
					9,9,9,1,
					2,9,9,2,
				};
/*
että kallossa noi perusvärit on 6, 6, e (eikä 0, 0, b kuten nyt) ja se
valopallo olisi sitten tuollainen lämmin punaisen sävyinen (eli skaala
olis tämmönen : 4, 2, a, 7, 1 - toi 4 mukaan sen takia, että se on
*/
				int c1n, c2n;
				c1n = grey_to_num[c1o];
				c2n = grey_to_num[c2o];


				int c1,c2;

				int palette[] =  {0x0, 0x0, 0x0, 0x0, 0x6, 0xe, 0x4, 0x2, 0xa, 0x7, 0x1};
				int palette2[] = {0x0, 0x0, 0x0, 0x0, 0x6, 0x4, 0x2, 0xe, 0xa, 0x7, 0x1};
				//int palette[] = {0x0, 0x6, 0x6, 0xe, 0x4, 0x2, 0xa, 0x7, 0x1};

				int palsize = sizeof(palette) / sizeof(palette[0]);

				c1n += i;
				c2n += i;
				c1n = glm::clamp(c1n, 0, palsize - 1);
				c2n = glm::clamp(c2n, 0, palsize - 1);

				if (i >= 3)
				{
					c1 = palette2[c1n];
					c2 = palette2[c2n];
				}
				else
				{
					c1 = palette[c1n];
					c2 = palette[c2n];
				}

				if (c1o == 0)
					c1 = 0;
				if (c2o == 0)
					c2 = 0;

				

				/*if (c1n == 9)
					c1 = 2;
				if (c2n == 9)
					c2 = 2;
*/
//				c1 = c1n;
//				c2 = c2n;


				int cval = (c1 << 4) | c2;

				lighttab[ltpos] = cval;
				lighttab_used[ltpos] = 1;
				lastused = glm::max(lastused, ltpos);
			}
		}
	}

	lighttab_size = lastused + 1;
	printf("lighttab size %i\n", lighttab_size);

#if 1
	int fillbyte = 0x00;
	for (i = 0; i < lighttab_size; i++)
	{
		if (lighttab_used[i])
			fillbyte = lighttab[i];
		else
			lighttab[i] = fillbyte;
	}
#endif

	pallo.savehiresfile("pallo.hires");
	kallo.savehiresfile("kallo.hires");


	hiresdata_t pallo_blue = pallo;

	for (i = 0; i < 40*25;i++)
	{
		int c1o = pallo.m_col1[i];
		int c2o = pallo.m_col2[i];

				int grey_to_num[] =
				{
					0,3,0,0,
					0,0,0,0,
					0,0,0,1,
					2,0,0,2,
				};

				int c1n, c2n;
				c1n = grey_to_num[c1o];
				c2n = grey_to_num[c2o];


				int c1,c2;

				int palette[] = {0x0, 0x0, 0x6, 0xe};

				c1 = palette[c1n];
				c2 = palette[c2n];
				//int cval = (c1 << 4) | c2;
		pallo_blue.m_col1[i] = c1;
		pallo_blue.m_col2[i] = c2;
	}

	pallo_blue.savehiresfile("pallo_blue.hires");

}

void outputsprite(char *pic)
{
	int x,y,x2;
	for (y = 0; y < 21; y++)
	{
		for (x = 0; x < 3; x++)
		{
			int ddo = y * 24 + x * 8;

			int byt = 0;
			for (x2 = 0; x2 < 8; x2++)
			{
				byt <<= 1;
				if (pic[ddo+x2])
					byt |= 1;
			}

			B(byt);

		}
	}
	B(0xFF);
}

void spritecrap()
{
	CTX("paska")
	spriteprg.begin(0x2C0);
	set_segment(&spriteprg);

	char pic[24*21];
	int x,y,i;


	for (x = 0; x < 64; x++)
	{
		B(0);
	}


	LPC("sprite1");
	for (i = 0; i < 20; i++)
	{
				float ad = i / 20.0f;

	for (x = 0; x < 24; x++)
	{
		for (y = 0; y < 21; y++)
		{
			vec2 midv = vec2(x-12.f, y-10.f);
			//float v = glm::length(midv) * 0.5f;
			float v = 1.0;
			int a;
			for (a = 0; a < 5; a++)
			{
				float ra = (a+ad*6.f) * (M_PI/5.f) * 2.f;
				v = glm::max(glm::dot(midv,vec2(cos(ra), sin(ra))), v);
			}

			//v *= 1.0f;
			v /= glm::min(1.f, i / 20.f) * 2.0f;

			int c;

			if (v < 5.f)
				c = 1;
			else
				c = 0;

			pic[y * 24 + x] = c;
		}
	}


		char curpic[24*21];
		memcpy(curpic, pic, 24*21);

		float edge = glm::max(0.f, ad*2.0f-1.0f);
		edge = pow(edge, 5.f) * 1.2f;
		//edge = 0.f;
		for (x = 0; x < 24*21; x++)
		{
			if (glm::linearRand(0.f, 1.f) <= edge)
				curpic[x] = 0;
		}

		outputsprite(curpic);
	}

}

void genis()
{
	msdoseffu();

	spritecrap();
}


int main()
{
	srand(time(NULL));
	ekat();
	assemble(genis);

	list<segment_c *> segs;
	segs.push_back(&actualprg);
	make_prg("valo.prg", 0x4000, segs);

	segs.clear();
	segs.push_back(&spriteprg);
	make_prg("valosprites.prg", 0x2C0, segs);

#ifdef TESTROUNDJITTER
	int i;
	for (i = 0; i < 40; i++)
	{
		float v = (i - 20) / 6.f;
		float v2 = roundjitter(v);
		float vf = glm::fract(v);
		//printf("%i: %.2f = %.2f (%.2f)\n", i, v, v2, vf);
	}
#endif
}