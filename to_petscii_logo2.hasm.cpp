#include <stdio.h>

#include "hires.h"

//#include <debugf.h>
//#include <hashembler.h>

#include "assets/aeriel_core.h"

unsigned char *petscii = aeriallogo;
char outputfn[] = "logo2.petscii";

void output()
{

	FILE *f = fopen(outputfn, "wb");

	int x,y,i;

	int datalen = 40*25*2+2;
	for (i = 0; i < datalen; i++)
	{
		int arvo = petscii[i];
		fputc(arvo, f);
	}

	fclose(f);
}


int main(int argc, char *argv[])
{
	output();
}