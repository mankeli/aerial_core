#pragma once
#include "glm.h"

#include "lode/lodepng.h"

class hiresdata_t
{
public:
 	std::vector<uint8_t> m_pic; //the raw pixels
 	std::vector<uint8_t> m_col1;
 	std::vector<uint8_t> m_col2;
 	std::vector<uint8_t> m_chars;
 	ivec2 m_size;
 	ivec2 m_bsize;
 	int m_bcount;
	void load(const char *fn)
	{
  		unsigned width, height;

  		//decode
//  		unsigned error = lodepng::decode(m_pic, width, height, fn, LCT_GREY, 8);

		std::vector<unsigned char> png;
		lodepng::State state; //optionally customize this one
		state.decoder.color_convert = false;

		lodepng::load_file(png, fn); //load the image file with given filename
		unsigned error = lodepng::decode(m_pic, width, height, state, png);


  		printf("m_pic size: %lu\n", m_pic.size());
  		printf("LODE error %i, w %i, h%i (%s)\n", error, width, height, lodepng_error_text(error));

#if 0
  		int i;
  		for (i = 0; i < 1000; i++)
  		{
  			float a = i * M_PI / 500.f;
  			float a2 = i * M_PI / 250.f + 4.2f;
  			float a3 = i * M_PI / 125.f + 2.3f;
  			int x = 160 + cos(a) * 50 + cos(a2) * 30 + cos(a3) * 20;
  			int y = 100 + sin(a) * 50 + sin(a2) * 30 + sin(a3) * 20;
  			m_pic[y * 320 + x] = 1;
  		}
#endif

  		m_size = ivec2(width, height);

		resize_buffers();
	}

	void scale_halfwidth()
	{
		int x,y;

		printf("scaling to half width\n");

		std::vector<uint8_t> tmppic;
		int halfwid = m_size.x / 2;

		tmppic.resize(m_size.y * halfwid);

		for (y = 0; y < m_size.y; y++)
		{
			for (x = 0; x < halfwid; x++)
			{
				int thisx = glm::clamp(x * 2, 0, m_size.x - 1);
				tmppic[y * halfwid + x] = m_pic[y * m_size.x + thisx];

#if 0
				if ((x % 6) == 3)
					tmppic[y * halfwid + x] = 1;
#endif
			}
		}

		m_pic = tmppic;
		m_size.x = halfwid;

		resize_buffers();
	}

	void init(ivec2 size)
	{
		m_size = size;
		m_pic.resize(m_size.x * m_size.y);

		resize_buffers();
	}

	void resize_buffers()
	{
		m_bsize = m_size / ivec2(8);

		printf("hires: size: %i,%i, bsize %i,%i\n", m_size.x, m_size.y, m_bsize.x, m_bsize.y);
  		m_bcount = m_bsize.x * m_bsize.y;

  		m_col1.resize(m_bcount);
  		m_col2.resize(m_bcount);
  		m_chars.resize(m_bcount * 8);
	}

	struct col_t
	{
		int idx;
		int cnt;
	};

	static int comparecolor(const void *e1, const void *e2)
	{
		col_t *c1 = (col_t *)e1;
		col_t *c2 = (col_t *)e2;
		return c2->cnt - c1->cnt;
	}

	void findcolor(ivec2 bpos)
	{
		int x,y;

		col_t histogram[256];

		for (x = 0; x < 256; x++)
		{
			histogram[x].idx = x;
			histogram[x].cnt = 0;
		}

		ivec2 bpos_pix = bpos * 8;

		//printf("char %i,%i\n", bpos.x, bpos.y);

		for (y = 0; y < 8; y++)
		{
			for (x = 0; x < 8; x++)
			{
				int idx = m_pic[(bpos_pix.y + y) * m_size.x + (bpos_pix.x + x)];
				histogram[idx].cnt++;
/*
				if (idx)
					printf("#");
				else
					printf(" ");
*/
			}
//			printf("\n");
		}

		qsort(histogram, 256, sizeof(col_t), comparecolor);

		m_col1[bpos.y * m_bsize.x + bpos.x] = histogram[0].idx;
		m_col2[bpos.y * m_bsize.x + bpos.x] = histogram[1].idx;

		if (histogram[1].cnt == 0)
			m_col2[bpos.y * m_bsize.x + bpos.x] = histogram[0].idx;

		//if (histogram[0].cnt != 64 || histogram[1].cnt)
		//	printf("most colors in %i,%i:  %i:%i, %i:%i\n", bpos.x, bpos.y, histogram[0].idx, histogram[0].cnt, histogram[1].idx, histogram[1].cnt);
	}

	void findchar(ivec2 bpos)
	{
		int x,y;

		ivec2 bpos_pix = bpos * 8;
		for (y = 0; y < 8; y++)
		{
			int tmpc = 0;
			for (x = 0; x < 8; x++)
			{
				int idx = m_pic[(bpos_pix.y + y) * m_size.x + (bpos_pix.x + x)];
//				int idx = 0;

				tmpc <<= 1;
				tmpc &= ~1;
				if (idx != m_col1[bpos.y * m_bsize.x + bpos.x])
					tmpc |= 1;
			}
			//tmpc = 0xAA;
			m_chars[(bpos.y * m_bsize.x + bpos.x) * 8 + y] = tmpc;

		}

		//m_chars[(bpos.y * m_bsize.x + bpos.x) * 8 + 0] = 0xAA;
	}

	void tohires()
	{
		int x,y;

		int col = 0;
		for (y = 0; y < m_size.y; y++)
		{
			for (x = 0; x < m_size.x; x++)
			{
				//printf("%i, %i: %i\n", x,y,m_pic[y * m_size.x + x] );
				if (m_pic[y * m_size.x + x] > 0)
					col++;
			}
		}

		printf("there was %i other pixels\n", col);

		printf("bsize %i,%i\n", m_bsize.x, m_bsize.y);

		findcolors();
		findchars();
	}

	void findcolors()
	{
		int x,y;
		for (y = 0; y < m_bsize.y; y++)
		{
			for (x = 0; x < m_bsize.x; x++)
			{
				findcolor(ivec2(x,y));
			}
		}
	}

	void set_monochrome(int c1, int c2)
	{
		int x,y;
		for (y = 0; y < m_bsize.y; y++)
		{
			for (x = 0; x < m_bsize.x; x++)
			{
				m_col1[y * m_bsize.x + x] = c1;
				m_col2[y * m_bsize.x + x] = c2;
			}
		}
	
	}

	void findchars()
	{
		int x,y;
		for (y = 0; y < m_bsize.y; y++)
		{
			for (x = 0; x < m_bsize.x; x++)
			{
				findchar(ivec2(x,y));
			}
		}
	}



void savehiresfile(const char *fn)
{
	int x,y,i;

	FILE *f = fopen(fn, "wb");

	for (y = 0; y < 25; y++)
	{
		for (x = 0; x < 40; x++)
		{
			for (i = 0; i < 8; i++)
			{
				int arvo;
				arvo = m_chars[(y * 40 + x) * 8 + i];
				arvo &= 0xFF;
				fputc(arvo, f);
			}
		}
	}

	for (i = 0; i < 40*25;i++)
	{
		//B(0x20);
		int cval = ((m_col2[i] & 0x0F) << 4) | (m_col1[i] & 0x0f);
		//int cval = rajahdys.m_col1[i] & 0x0f;
		fputc(cval, f);
	}

	fclose(f);

}


};
