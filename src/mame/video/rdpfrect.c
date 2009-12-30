#if defined(MAGICDITHER)
	static void fill_rectangle_16bit_c1_dm(RECTANGLE *rect)
#elif defined(BAYERDITHER)
	static void fill_rectangle_16bit_c1_db(RECTANGLE *rect)
#else
	static void fill_rectangle_16bit_c1_dn(RECTANGLE *rect)
#endif
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT8* hb = &hidden_bits[fb_address >> 1];

	int index, i, j;
	int x1 = rect->xh / 4;
	int x2 = rect->xl / 4;
	int y1 = rect->yh / 4;
	int y2 = rect->yl / 4;
	int clipx1, clipx2, clipy1, clipy2;
	UINT16 fill_color1, fill_color2;
	int fill_cvg1;
	int fill_cvg2;

	if (x2 <= x1)
	{
		x2=x1+1; // SCARS (E)
	}
	if (y2 == y1)
	{
		y2=y1+1; // Goldeneye
	}


	fill_color1 = (fill_color >> 16) & 0xffff;
	fill_color2 = (fill_color >>  0) & 0xffff;
	fill_cvg1 = (fill_color1 & 1) ? 8 : 1;
	fill_cvg2 = (fill_color2 & 1) ? 8 : 1;

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	// clip
	if (x1 < clipx1)
	{
		x1 = clipx1;
	}
	if (y1 < clipy1)
	{
		y1 = clipy1;
	}
	if (x2 >= clipx2)
	{
		x2 = clipx2-1;
	}
	if (y2 >= clipy2)
	{
		y2 = clipy2-1;
	}

	shade_color.c = 0;	// Needed by Command & Conquer menus

#if defined(MAGIC_DITHER)
	for (j = y1; j <= y2; j++)
	{
		COLOR c;
		int dith = 0;
		index = j * fb_width;
		for (i = x1; i <= x2; i++)
		{
			curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;
			COLOR_COMBINER1(c);
			dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
			BLENDER1_16_DITH(&fb[(index + i) ^ WORD_ADDR_XOR], &hb[(index + i) ^ BYTE_ADDR_XOR], c, dith);
		}
	}
#elif defined(BAYER_DITHER)
	for (j = y1; j <= y2; j++)
	{
		COLOR c;
		int dith = 0;
		index = j * fb_width;
		for (i = x1; i <= x2; i++)
		{
			curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;
			COLOR_COMBINER1(c);
			dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
			BLENDER1_16_DITH(&fb[(index + i) ^ WORD_ADDR_XOR], &hb[(index + i) ^ BYTE_ADDR_XOR], c, dith);
		}
	}
#else
	for (j = y1; j <= y2; j++)
	{
		COLOR c;
		index = j * fb_width;
		for (i = x1; i <= x2; i++)
		{
			curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;
			COLOR_COMBINER1(c);
			BLENDER1_16_NDITH(&fb[(index + i) ^ WORD_ADDR_XOR], &hb[(index + i) ^ BYTE_ADDR_XOR], c);
		}
	}
#endif
}

#if defined(MAGICDITHER)
	static void fill_rectangle_16bit_c2_dm(RECTANGLE *rect)
#elif defined(BAYERDITHER)
	static void fill_rectangle_16bit_c2_db(RECTANGLE *rect)
#else
	static void fill_rectangle_16bit_c2_dn(RECTANGLE *rect)
#endif
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT8* hb = &hidden_bits[fb_address >> 1];

	int index, i, j;
	int x1 = rect->xh / 4;
	int x2 = rect->xl / 4;
	int y1 = rect->yh / 4;
	int y2 = rect->yl / 4;
	int clipx1, clipx2, clipy1, clipy2;
	UINT16 fill_color1, fill_color2;
	int fill_cvg1;
	int fill_cvg2;

	if (x2 <= x1)
	{
		x2=x1+1; // SCARS (E)
	}
	if (y2 == y1)
	{
		y2=y1+1; // Goldeneye
	}


	fill_color1 = (fill_color >> 16) & 0xffff;
	fill_color2 = (fill_color >>  0) & 0xffff;
	fill_cvg1 = (fill_color1 & 1) ? 8 : 1;
	fill_cvg2 = (fill_color2 & 1) ? 8 : 1;

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	// clip
	if (x1 < clipx1)
	{
		x1 = clipx1;
	}
	if (y1 < clipy1)
	{
		y1 = clipy1;
	}
	if (x2 >= clipx2)
	{
		x2 = clipx2-1;
	}
	if (y2 >= clipy2)
	{
		y2 = clipy2-1;
	}

	shade_color.c = 0;	// Needed by Command & Conquer menus

#if defined(MAGIC_DITHER)
	for (j=y1; j <= y2; j++)
	{
		COLOR c1, c2;
		int dith = 0;
		index = j * fb_width;
		for (i=x1; i <= x2; i++)
		{
			curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;
			COLOR_COMBINER2_C0(c1);
			COLOR_COMBINER2_C1(c2);
			dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
			BLENDER2_16_DITH(&fb[(index + i) ^ WORD_ADDR_XOR],  &hb[(index + i) ^ BYTE_ADDR_XOR], c1, c2, dith);
		}
	}
#elif defined(BAYER_DITHER)
	for (j=y1; j <= y2; j++)
	{
		COLOR c1, c2;
		int dith = 0;
		index = j * fb_width;
		for (i=x1; i <= x2; i++)
		{
			curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;
			COLOR_COMBINER2_C0(c1);
			COLOR_COMBINER2_C1(c2);
			dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
			BLENDER2_16_DITH(&fb[(index + i) ^ WORD_ADDR_XOR],  &hb[(index + i) ^ BYTE_ADDR_XOR], c1, c2, dith);
		}
	}
#else
	for (j=y1; j <= y2; j++)
	{
		COLOR c1, c2;
		index = j * fb_width;
		for (i=x1; i <= x2; i++)
		{
			curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;
			COLOR_COMBINER2_C0(c1);
			COLOR_COMBINER2_C1(c2);
			BLENDER2_16_NDITH(&fb[(index + i) ^ WORD_ADDR_XOR],  &hb[(index + i) ^ BYTE_ADDR_XOR], c1, c2);
		}
	}
#endif
}

#if defined(MAGICDITHER)
static void fill_rectangle_16bit_cc(RECTANGLE *rect)
{
	fatalerror("fill_rectangle_16bit: cycle type copy");
}
#endif

#if defined(MAGICDITHER)
static void fill_rectangle_16bit_cf(RECTANGLE *rect)
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT8* hb = &hidden_bits[fb_address >> 1];

	int index, i, j;
	int x1 = rect->xh / 4;
	int x2 = rect->xl / 4;
	int y1 = rect->yh / 4;
	int y2 = rect->yl / 4;
	int clipx1, clipx2, clipy1, clipy2;
	UINT16 fill_color1, fill_color2;
	int fill_cvg1;
	int fill_cvg2;

	if (x2 <= x1)
	{
		x2=x1+1; // SCARS (E)
	}
	if (y2 == y1)
	{
		y2=y1+1; // Goldeneye
	}


	fill_color1 = (fill_color >> 16) & 0xffff;
	fill_color2 = (fill_color >>  0) & 0xffff;
	fill_cvg1 = (fill_color1 & 1) ? 8 : 1;
	fill_cvg2 = (fill_color2 & 1) ? 8 : 1;

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	// clip
	if (x1 < clipx1)
	{
		x1 = clipx1;
	}
	if (y1 < clipy1)
	{
		y1 = clipy1;
	}
	if (x2 >= clipx2)
	{
		x2 = clipx2-1;
	}
	if (y2 >= clipy2)
	{
		y2 = clipy2-1;
	}

	shade_color.c = 0;	// Needed by Command & Conquer menus

	fill_cvg1 = (fill_color1 & 1) ? 3 : 0;
	fill_cvg2 = (fill_color2 & 1) ? 3 : 0;

	if(x1 & 1)
	{
		for (j=y1; j <= y2; j++)
		{
			index = j * fb_width;
			for (i=x1; i <= x2; i += 2)
			{
				int curpixel = index + i;
				fb[curpixel ^ WORD_ADDR_XOR] = fill_color2;
				hb[curpixel ^ BYTE_ADDR_XOR] = fill_cvg2;
			}
		}
		for (j=y1; j <= y2; j++)
		{
			index = j * fb_width;
			for (i=x1+1; i <= x2; i += 2)
			{
				int curpixel = index + i;
				fb[curpixel ^ WORD_ADDR_XOR] = fill_color1;
				hb[curpixel ^ BYTE_ADDR_XOR] = fill_cvg1;
			}
		}
	}
	else
	{
		for (j=y1; j <= y2; j++)
		{
			index = j * fb_width;
			for (i=x1; i <= x2; i += 2)
			{
				int curpixel = index + i;
				fb[curpixel ^ WORD_ADDR_XOR] = fill_color1;
				hb[curpixel ^ BYTE_ADDR_XOR] = fill_cvg1;
			}
		}
		for (j=y1; j <= y2; j++)
		{
			index = j * fb_width;
			for (i=x1+1; i <= x2; i += 2)
			{
				int curpixel = index + i;
				fb[curpixel ^ WORD_ADDR_XOR] = fill_color2;
				hb[curpixel ^ BYTE_ADDR_XOR] = fill_cvg2;
			}
		}
	}
}
#endif
