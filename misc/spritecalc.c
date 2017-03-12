#include <stdio.h>
#include <string.h>

int main(void)
{
	int y;

	const int spritecount = 13;
	const int width = 7;

	const int maxvisi = 5;

	int sprvisi[spritecount];
	int sprvisid[spritecount];
	memset(sprvisi, 0, sizeof(int) * spritecount);
	memset(sprvisid, 0, sizeof(int) * spritecount);
	int sprnum = -1;

	int sprite_linemap[spritecount * 21];
	memset(sprite_linemap, 0, sizeof(int) * spritecount * 21);

	int i;

	for (y = 0; y < 200; y++)
	{
		int barrel_y = y % 21;

		if (y % 16 == 0)
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

}