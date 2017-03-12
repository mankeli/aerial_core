#include <stdio.h>

#include "dirart.h"

int main(void)
{
	int y,x;
	for (y = 0; y < 15; y++)
	{
		for (x = 0; x < 16; x++)
		{
			putchar(frame0000[y * 40 + x + 2] | 0x80);
		}
		putchar('\n');
	}

	return 1;
}
