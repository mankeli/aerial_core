#include "hires.h"

#include <debugf.h>
#include <hashembler.h>

#include <math.h>
#include "lib.h"
#include "memconfig.h"

using namespace hashembler;

segment_asm_c actualprg;

//#define BORDER_NMI

value_t which_irq = 0x40;
value_t eorbyte = 0x41;

int sprytab[13];

#include "jump_if_not_time.h"

void effu()
{
	CTX("msdos")
	actualprg.begin(0x8000);
	set_segment(&actualprg);

	LPC("begin");
	PRINTPOS("begin");

	JMP(L("paska"))

LPC("paska")

	JSR(if_unsetirq1)
	JSR(if_unsetirq2)

	// stop cia nmi
	LDAi(0x7F)
	STA(0xDD0D);
	LDA(0xDD0D)


int i;

	JSR(if_startmusic)

	MOV16i(framecounter, 0);

	LDAi(0)
	STA(0xd011);


	JSR(L("setupmohko"))

	LPC("loop-2")
//	JMP(L("loop-2"))
	jump_if_not_time(sync_mohko_logo, L("loop-2"));

#ifndef NO_LOADING

	LDAi(0)
	STA(0xd011);

	LDAi(0x7F)
	STA(0xDD0D);
	LDA(0xDD0D)
	JSR(if_unsetirq1)
	JSR(if_unsetirq2)

	JSR(0xC90)
#endif

	JSR(L("setuplogo"))

	LPC("loop-3")
	jump_if_not_time(sync_mohko_backtogfx, L("loop-3"));

	LDAi(0x7F)
	STA(0xDD0D);
	LDA(0xDD0D)
	JSR(if_unsetirq1)
	JSR(if_unsetirq2)
	JSR(L("setupmohko"))


	LPC("loop-4")
	jump_if_not_time(sync_mohko_gonext, L("loop-4"));

	LDAi(0x7F)
	STA(0xDD0D);
	LDA(0xDD0D)
	JSR(if_unsetirq1)
	JSR(if_unsetirq2)
	JSR(L("setuplogo"))


//	LPC("loop-1")
//	jump_if_not_time(sync_mohko_gonext, L("loop-1"));

#ifndef NO_LOADING

	// NOTE: next part is not using the memory nmi points to!!
	// LDAi(0x7F)
	// STA(0xDD0D);
	// LDA(0xDD0D)

	JSR(if_unsetirq1)
	JSR(if_unsetirq2)

	JMP(if_executenext_nowait);
#endif


LPC("loopa")
	//INC(0xd020);
	
	JMP(L("loopa"))


LPC("setupmohko")
	LDAi(0x0)
	STA(0xd020)


	LDAi(0x3D)
	STA(0xdd02);
	LDAi(0x7F)
	STA(0xd018);



	LDAi(0x08)
	STA(0xd016)

	LDXi(0x0);
	for (i = 0; i < 7; i++)
	{
		LDAi(24 + i*48)
		STA(0xd000 + i*2)
		STX(0xd027+i)
	}
	LDAi(0x60);
	STA(0xd010);

	LDAi(0x7f)
	
	STA(0xd015);

	LDAi(0xFF)
//	STA(0xd017); // dh
	STA(0xd01D); // dw

	LDAi(0x00);
	STA(0xd017); // dh
	STA(0xd01b); //prio


	INC(0xd021);

	LDXi(LB(L("topirq")))
	LDYi(HB(L("topirq")))
	JSR(if_setirq2)

	LDXi(LB(L("colorshit")))
	LDYi(HB(L("colorshit")))
	JSR(if_setirq1)

	LDAi(0x3B)
	STA(0xd011)


	RTS();


LPC("setuplogo")

	LDAi(0);
	STA(0xd020);
	STA(0xd021);

	LDAi(0x3C);
	STA(0xDD02);

	LDAi(0x1B);
	STA(0xd011);
	LDAi(0x14)
	STA(0xd018);


	LDAi(0x00);
	STA(0xDD0E);


// NMI for logo
	LDAi(LB(L("logonmi")))
	STA(0xfffa)
	LDAi(HB(L("logonmi")))
	STA(0xfffb)

	value_t first_nmi = 63*10;
	LDAi(LB(first_nmi))
	STA(0xdd04)
	LDAi(HB(first_nmi))
	STA(0xdd05)

	// start counter, irq
	LDAi(0x11);
	STA(0xDD0E);

	LDAi(0x81)
	STA(0xDD0D);
	LDA(0xDD0D);
	RTS();

// IRQ ROUTINES BEGIN

{
LPC("topirq")
	//DEC(0XD020);
	LDAi(0x7F)
	STA(0xDD0D);

	LDAi(LB(L("nmi")))
	STA(0xfffa)
	LDAi(HB(L("nmi")))
	STA(0xfffb)

	LDAi(0x0);
	STA(0xDD0E);
	STA(0xDD0F);

	value_t first_nmi = 63*73;
	LDAi(LB(first_nmi))
	STA(0xdd04)
	LDAi(HB(first_nmi))
	STA(0xdd05)


	LDAi(0x1C);
LPC("wait_for_line")
	CMP(0xd012);
	BNE(L("wait_for_line"))

	// start counter, irq
	LDAi(0x19);
	STA(0xDD0E);

	LDA(0xDD0D)
	LDAi(0x81)
	STA(0xDD0D);

#ifdef BORDER_NMI
	INC(0xd020);
	DEC(0xd020);
#endif

//	JSR(L("colorshit"))

	RTS();
}

{
	LPC("logonmi")
	PHA();

	//INC(0xd020);
	LDA(0xd41C);
	LSRa();
	ANDi(0x03);
	STAz(eorbyte)
	LDA(0xd016);
	EORz(eorbyte)
	STA(0xd016);
	//DEC(0xd020);

	LDA(0xDD0D)

	PLA();
	RTI();
}

{
LPC("nmi")
	PHA();
	TXA();
	PHA();
	TYA();
	PHA();
#ifdef BORDER_NMI
//	LDAi(0x02)
//	STA(0xd020);
	INC(0xd020);
#endif
	value_t first_nmi = 63*16;

	// cia timer begin
	LDAi(LB(first_nmi))
	STA(0xdd04)
	LDAi(HB(first_nmi))
	STA(0xdd05)

	LDAi(0x11);
	STA(0xDD0E);

	LDAi(LB(L("nmi2")))
	STA(0xfffa)
	LDAi(HB(L("nmi2")))
	STA(0xfffb)

	LDAi(0x81)
	STA(0xDD0D);
	LDA(0xDD0D)

	LDAi(50)
	for (i = 0; i < 7; i++)
	{
		STA(0xd001 + i * 2)
	}

	for (i = 0; i < 7; i++)
	{
		int sn = (0x00) / 64 + i;
		LDAi(sn)
		STA(0x5C00+0x3f8 + i)
	}

	LDXi(0x01);
	STXz(which_irq)


#ifdef BORDER_NMI
//	LDAi(0xf)
//	STA(0xd020);
	DEC(0xd020);
#endif


	PLA();
	TAY();
	PLA();
	TAX();
	PLA();

	RTI();
}

// linet 64, 80 jne

{
LPC("nmi2")
	PHA();
	TXA();
	PHA();
//	TYA();
//	PHA();


#ifdef BORDER_NMI
//	LDAi(0x01)
//	STA(0xd020);
	INC(0xd020);

#endif
	LDXz(which_irq)

//	B(2);

	for (i = 0; i < 7; i++)
	{
		LDAx(L("sprptrtab%i", i))
		STA(0x5C00+0x3f8 + i)
	}

	LDAx(L("sprytab"))
	for (i = 0; i < 7; i++)
	{
		STA(0xd001 + i * 2)
	}



	LDA(0xDD0D)

	INX();
	STXz(which_irq)
	CPXi(13);
	BNE(L("endnmi"))

	LDAi(0x7F)
	STA(0xDD0D);

LPC("endnmi")



#ifdef BORDER_NMI
//	LDAi(0xf)
//	STA(0xd020);
	DEC(0xd020);

#endif

//	PLA();
//	TAY();
	PLA();
	TAX();
	PLA();


	RTI();
}

/*
	- selvitä eka scanline missä NMI tapahtuu
    - aja rutiini joka ajaa sprpointterivaihtoja 16 rivin välein
    - merkkaa mahdolliset kohdat missä sprite on
*/

// sprites:
// 0 16 32 48 64

LPC("sprytab")
	for (i = 0; i < 13; i++)
	{
		int sprite_y = sprytab[i];
		B(sprite_y)
	}

	int sn = (0x00) / 64;

	int j;

	for (i = 0; i < 7; i++)
	{
		LPC("sprptrtab%i", i)
		for (j = 0; j < 13; j++)
		{
			int ptr = sn + j * 7 + i;
//			ptr = sn+7 + (j&1) ? 8 : 0;
			B(ptr)
		}
	}


}



hiresdata_t mhires;


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

int colcombs[256];

void puthiresdata()
{
	CTX("msdos")

	int x,y,i;

{
LPC("colorshit")
	CLI();

#if 0
	LDA(0xd016);
	CLC();
	ANDi(0x07);
	ADCi(0x04);
	ORAi(0x08);
	STA(0xd016);
#endif

	LDAz(framecounter)
	ANDi(0x7F);
	TAX();
	for (y = 0; y < 25; y++)
	{
	//	#define D016JITTER
		#ifdef D016JITTER
		LDA(0xd016);
		CLC();
		ANDi(0x07);
		ADCi(0x01);
		ORAi(0x08);
		STA(0xd016);
		#endif


		//#define SPRITEJITTER
		#ifdef SPRITEJITTER
		for (x = 0; x < 7; x++)
		{
			LDA(0xd000 + x*2)
			EORi(0x01);
			STA(0xd000 + x*2)
		}
		#endif

		#ifdef SPRITECOLORFUCK
		//LDA(0xd027 + x)
		LDA(0xd012);
		ANDi(0x01);
		BEQ(L("nosprcolchg%i",y))

		LDAi(0x0b);
		LPC("nosprcolchg%i",y)

		//EORi(0x0b);
		for (x = 0; x < 7; x++)
		{
			STA(0xd027 + x)
		}
		#endif


		for (x = 0; x < 40; x++)
		{
			//INC(0x5C00+y*40+x)
			int chi = y*40+x;
			int colcomb = ((mhires.m_col2[chi] & 0x0F) << 4) | (mhires.m_col1[chi] & 0x0f);

			vec2 midv = vec2(x-20.0f,y-8.0f);
			float dist = glm::length(midv);
			int offs = (int)dist & 0x7F;

			float ang = atan2(midv.x, midv.y) / M_PI;

			float val = 0.f;
			val += ang * 128.f * 3;
			val += dist * 2.f;

			val += sin(x * 0.4f) * 5.f;
			val += sin(y * 0.3f) * 5.f;

			offs = (int)val & 0x7f;


			LDAx(L("coltab%i", colcomb)+offs)
//			LDAi(0x11);
			STA(0x5C00+y*40+x)
		}
	}
	RTS();
}



//	int cl1[] = {11, 11,  11,  11};
//	int cl2[] = {0x6, 0xb, 0x2, 0xb};
//	int cl3[] = {0xe, 0xc, 0xa, 0xc};
//	int cl4[] = {0xc, 0xf, 0x7, 0xf};

	// b 3 e 1
	// 6 4 e 3 d 1
//	int cl1[] = {0xb, 0xc, 0xf, 0x1};
//	int cl2[] = {0x6, 0x4, 0xe, 0x3};
//	int cl3[] = {0x4, 0xe, 0x3, 0xd};
//	int cl4[] = {0xe, 0x3, 0xd, 0x1};

//	int cl1[] = {0xb, 0xc, 0xf, 0x1, 0xf, 0xc};

int clU[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

int cl1[] = {0xb, 0x2, 0xb, 0x6, 0xb, 0x4};
int cl2[] = {0xe, 0xa, 0x5, 0xe, 0xc, 0xa};
int cl3[] = {0x3, 0xf, 0xd, 0x3, 0x7, 0x3};
int cl4[] = {0x1, 0x7, 0x1, 0x7, 0x1, 0xd};

	int *maptab[] =
	{
                clU, cl4, clU, cl3,
                clU, clU, clU, clU,
                clU, clU, clU, cl1,
                clU, clU, cl2, clU,
   	};



	for (i = 0; i < 256; i++)
	{
		if (colcombs[i] > 0)
		{
			int c1o = (i >> 4) & 0x0F;
			int c2o = (i >> 0) & 0x0F;

			int *cl_a = maptab[c1o];
			int *cl_b = maptab[c2o];

			LPC("coltab%i", i);
			write_colortab(cl_b, cl_a, 6);
			write_colortab(cl_b, cl_a, 6);
		}
	}
}


void genis()
{
	effu();
	puthiresdata();
}

int *create_sprite_mappings(int spritecount, int firstspriteline, int firstnmiline)
{
	int y;

	int offs = firstnmiline - firstspriteline;


	const int maxvisi = 5;

	int sprvisi[spritecount];
	int sprvisid[spritecount];
	memset(sprvisi, 0, sizeof(int) * spritecount);
	memset(sprvisid, 0, sizeof(int) * spritecount);
	int sprnum = -1;

	int *sprite_linemap = new int[sizeof(int) * spritecount * 21];
	memset(sprite_linemap, 0, sizeof(int) * spritecount * 21);

	sprnum = 0;
	sprvisi[0] = maxvisi;

	int i;

	for (y = 0; y < 200; y++)
	{
		int barrel_y = y % 21;

		if (y % 16 == offs)
		{
			if (sprnum >= 0)
				sprvisid[sprnum] = -1;
			sprnum++;

			sprvisi[sprnum] = maxvisi;
		}

		for (i = 0; i < spritecount; i++)
		{
			sprvisi[i] += sprvisid[i];
			if (sprvisi[i] < 0)
			{
				sprvisi[i] = 0;
				sprvisid[i] = 0;
			}
		}


		printf("y: %02X, %02X: ", y, barrel_y);

		for (i = 0; i < spritecount; i++)
		{
			if (sprvisi[i] > 0)
				printf("%X", i);
			else
				printf(" ");

			if (sprvisi[i] > 0)
			{
				int lmo = barrel_y * spritecount + i;

				if (sprite_linemap[lmo])
					printf("!");
				else
					printf("+");

				sprite_linemap[lmo] = y;
			}
			else
				printf(" ");

			printf(" ");

		}
		printf("\n");
		if (barrel_y == 20)
		{
			printf("---------\n");
		}


	}

	printf("\n\n");
	for (i = 0; i < spritecount; i++)
	{
		printf("sprite %i:\n", i);
		for (y = 0; y < 21; y++)
		{
			printf("%02X: %02X\n", y, sprite_linemap[y * spritecount + i]);
		}

	}

	return sprite_linemap;
}

void ekat()
{
	int i,y,x;
	int set;

int firstnmiline = 62;
int firstspriteline = 50;
int sprites_per_col = 13;

sprytab[0] = firstspriteline;

	for (i = 0; i < sprites_per_col; i++)
	{
		int nmi_y = firstnmiline + i * 16;
		int gfx_y = (nmi_y - firstspriteline);
		int sprite_set = (nmi_y - firstspriteline) / 21 + 1;
		int sprite_set_y = sprite_set * 21 + firstspriteline;
		int line_in_sprite = (nmi_y - firstspriteline) % 21;
		printf("sprite_y tab %i: nmi %i, gfx_y %i, sprset %i, sprset_y %i, spriteline %i\n", i, nmi_y, gfx_y, sprite_set, sprite_set_y, line_in_sprite);

		sprytab[i + 1] = sprite_set_y;
	}

int *sprlinemap = create_sprite_mappings(sprites_per_col, firstspriteline, firstnmiline);

	hiresdata_t spr;
	spr.load("assets/mohko_sprites.png");
	spr.scale_halfwidth();
	spr.set_monochrome(1, 0);
	spr.findchars();

#if 0
	for (x = 0; x < spr.m_bcount*8; x++)
	{
		spr.m_chars[x] = 0xAA;
	}
#endif

	FILE *f = fopen("mohkosprites.bin", "wb");

	for (set = 0; set < sprites_per_col; set++)
	{
		for (i = 0; i < 7; i++)
		{
			for (y = 0; y < 21; y++)
			{
				//int co = (y+0) * 20 + i * 3;
				//int ys = y + set * 21;

				int ys = sprlinemap[y * sprites_per_col + set];

//				int ryd = ys/16;
//				int dd = ryd*21;

//				dd = 0xAA;


				int yb = ys/8;
				int ybo = ys&7;
				int co = yb*spr.m_bsize.x*8 + (i*3)*8 + ybo;

				//spr.m_chars[co + 8] ^= 0xFF;

#if 1
				fputc(spr.m_chars[co + 0], f);
				fputc(spr.m_chars[co + 8], f);
				fputc(spr.m_chars[co + 16], f);
#else
				fputc(dd, f);
				fputc(dd, f);
				fputc(dd, f);
#endif
			}
			fputc(0, f);
		}
	}

	printf("end of mohkosprites: %04X\n", ftell(f));

	fclose(f);

	mhires.load("assets/mohko_hires.png");
	mhires.tohires();
	mhires.savehiresfile("mohko.hires");

	for (i = 0; i < 256; i++)
	{
		colcombs[i] = 0;
	}

	for (i = 0; i < mhires.m_bcount; i++)
	{
		int cval = ((mhires.m_col2[i] & 0x0F) << 4) | (mhires.m_col1[i] & 0x0f);
		colcombs[cval]++;
	}


}

int main()
{
	ekat();
	assemble(genis);

	list<segment_c *> segs;
	segs.push_back(&actualprg);
	make_prg("spriteoverlay.prg", 0x8000, segs);
}