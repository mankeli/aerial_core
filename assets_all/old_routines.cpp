

int mapcolor(float intensity, int *t1, int numcol)
{
	float fnum = intensity * (numcol - 1.01f);
	int num1 = floor(intensity * (numcol - 1.01));

	if (num1 > (numcol - 2))
		num1 = numcol - 2;

	int num2 = num1 + 1;

	int colmix = 0;
	float dither = 0.5f;
	//dither = glm::linearRand(0.3f, 0.7f);
	if (glm::fract(fnum) < dither)
		colmix = t1[num1];
	else
		colmix = t1[num2];

	return colmix;
}
