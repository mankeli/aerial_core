#include <stdio.h>
#include <stdint.h>

#include "hires.h"

//#include <debugf.h>
//#include <hashembler.h>

void maapallo(char *inputfn, char *outputfn)
{

	hiresdata_t pallo;
	pallo.load(inputfn);
	//pallo.load("assets/kallo.png");
	pallo.tohires();


	FILE *f = fopen(outputfn, "wb");

	int x,y,i;
	for (y = 0; y < 25; y++)
	{
		for (x = 0; x < 40; x++)
		{
			for (i = 0; i < 8; i++)
			{
				vec2 midv = vec2((x-20)*8,(y-12)*8+i);
				float dist = 1000.f / glm::length(midv);
				int arvo = dist;
				//arvo = 0xDE;
				arvo = pallo.m_chars[(y * 40 + x) * 8 + i];
				arvo &= 0xFF;
				fputc(arvo, f);
			}
		}
	}

	for (i = 0; i < 40*25;i++)
	{
		//B(0x20);
		int cval = ((pallo.m_col2[i] & 0x0F) << 4) | (pallo.m_col1[i] & 0x0f);
		fputc(cval, f);
	}

	fclose(f);
}


int main(int argc, char *argv[])
{
	maapallo(argv[1], argv[2]);
}