/***************************************************************************

    Atari Jaguar object processor

****************************************************************************/

#ifndef INCLUDE_OBJECT_PROCESSOR
#error jagobj.c is not directly compilable!
#endif


#define LOG_OBJECTS			0


static UINT16 scanline[360];
static UINT16 *clutbase;
static UINT8 *blend_y, *blend_cc;



/*************************************
 *
 *  Object processor init
 *
 *************************************/

void jagobj_init(void)
{
	int i;

	/* allocate memory for tables */
	blend_y = auto_malloc(256 * 256);
	blend_cc = auto_malloc(256 * 256);

	/* fill tables */
	for (i = 0; i < 256 * 256; i++)
	{
		int y = (i >> 8) & 0xff;
		int dy = (INT8)i;
		int c1 = (i >> 8) & 0x0f;
		int dc1 = (INT8)(i << 4) >> 4;
		int c2 = (i >> 12) & 0x0f;
		int dc2 = (INT8)(i & 0xf0) >> 4;

		y += dy;
		if (y < 0) y = 0;
		else if (y > 0xff) y = 0xff;
		blend_y[i] = y;

		c1 += dc1;
		if (c1 < 0) c1 = 0;
		else if (c1 > 0x0f) c1 = 0x0f;
		c2 += dc2;
		if (c2 < 0) c2 = 0;
		else if (c2 > 0x0f) c2 = 0x0f;
		blend_cc[i] = (c2 << 4) | c1;
	}
}



/*************************************
 *
 *  Blending function
 *
 *************************************/

#define BLEND(dst, src)		\
	(dst) = (blend_cc[((dst) & 0xff00) | (((src) >> 8) & 0xff)] << 8) | blend_y[(((dst) & 0xff) << 8) | ((src) & 0xff)];



/*************************************
 *
 *  4bpp bitmap renderers
 *
 *************************************/

INLINE void bitmap_4_draw(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT8 flags, INT32 dxpos)
{
	if (firstpix & 7)
	{
		UINT32 pixsrc = src[firstpix / 8];
		while (firstpix & 7)
		{
			int pix = (pixsrc >> (4 * (~firstpix & 7))) & 0x0f;
			if ((!(flags & 4) || pix) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE(pix)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE(pix)]);
			}
			xpos += dxpos;
			firstpix++;
		}
	}

	firstpix /= 8;
	iwidth /= 8;
	iwidth -= firstpix;

	while (iwidth-- > 0)
	{
		UINT32 pix = src[firstpix++];
		if (!(flags & 4) || pix)
		{
			if ((!(flags & 4) || (pix & 0xf0000000)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE(pix >> 28)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE(pix >> 28)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x0f000000)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 24) & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 24) & 0x0f)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x00f00000)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 20) & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 20) & 0x0f)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x000f0000)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 16) & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 16) & 0x0f)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x0000f000)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 12) & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 12) & 0x0f)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x00000f00)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 8) & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 8) & 0x0f)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x000000f0)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 4) & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 4) & 0x0f)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x0000000f)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE(pix & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE(pix & 0x0f)]);
			}
			xpos += dxpos;
		}
		else
			xpos += 8 * dxpos;
	}
}

static void bitmap_4_0(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_4_draw(firstpix, iwidth, src, xpos, 0, 1);
}

static void bitmap_4_1(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_4_draw(firstpix, iwidth, src, xpos, 1, -1);
}

static void bitmap_4_2(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_4_draw(firstpix, iwidth, src, xpos, 2, 1);
}

static void bitmap_4_3(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_4_draw(firstpix, iwidth, src, xpos, 3, -1);
}

static void bitmap_4_4(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_4_draw(firstpix, iwidth, src, xpos, 4, 1);
}

static void bitmap_4_5(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_4_draw(firstpix, iwidth, src, xpos, 5, -1);
}

static void bitmap_4_6(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_4_draw(firstpix, iwidth, src, xpos, 6, 1);
}

static void bitmap_4_7(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_4_draw(firstpix, iwidth, src, xpos, 7, -1);
}

static void (*bitmap4[8])(INT32, INT32, UINT32 *, INT32) =
{
	bitmap_4_0,
	bitmap_4_1,
	bitmap_4_2,
	bitmap_4_3,
	bitmap_4_4,
	bitmap_4_5,
	bitmap_4_6,
	bitmap_4_7
};



/*************************************
 *
 *  8bpp bitmap renderers
 *
 *************************************/

INLINE void bitmap_8_draw(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT8 flags, INT32 dxpos)
{
	if (firstpix & 3)
	{
		UINT32 pixsrc = src[firstpix / 4];
		while (firstpix & 3)
		{
			UINT8 pix = pixsrc >> (8 * (~firstpix & 3));
			if ((!(flags & 4) || pix) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE(pix)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE(pix)]);
			}
			xpos += dxpos;
			firstpix++;
		}
	}

	firstpix /= 4;
	iwidth /= 4;
	iwidth -= firstpix;

	while (iwidth-- > 0)
	{
		UINT32 pix = src[firstpix++];
		if (!(flags & 4) || pix)
		{
			if ((!(flags & 4) || (pix & 0xff000000)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE(pix >> 24)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE(pix >> 24)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x00ff0000)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 16) & 0xff)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 16) & 0xff)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x0000ff00)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 8) & 0xff)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 8) & 0xff)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x000000ff)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE(pix & 0xff)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE(pix & 0xff)]);
			}
			xpos += dxpos;
		}
		else
			xpos += 4 * dxpos;
	}
}

static void bitmap_8_0(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_8_draw(firstpix, iwidth, src, xpos, 0, 1);
}

static void bitmap_8_1(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_8_draw(firstpix, iwidth, src, xpos, 1, -1);
}

static void bitmap_8_2(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_8_draw(firstpix, iwidth, src, xpos, 2, 1);
}

static void bitmap_8_3(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_8_draw(firstpix, iwidth, src, xpos, 3, -1);
}

static void bitmap_8_4(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_8_draw(firstpix, iwidth, src, xpos, 4, 1);
}

static void bitmap_8_5(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_8_draw(firstpix, iwidth, src, xpos, 5, -1);
}

static void bitmap_8_6(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_8_draw(firstpix, iwidth, src, xpos, 6, 1);
}

static void bitmap_8_7(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_8_draw(firstpix, iwidth, src, xpos, 7, -1);
}

static void (*bitmap8[8])(INT32, INT32, UINT32 *, INT32) =
{
	bitmap_8_0,
	bitmap_8_1,
	bitmap_8_2,
	bitmap_8_3,
	bitmap_8_4,
	bitmap_8_5,
	bitmap_8_6,
	bitmap_8_7
};



/*************************************
 *
 *  16bpp bitmap renderers
 *
 *************************************/

INLINE void bitmap_16_draw(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT8 flags, INT32 dxpos)
{
	if (firstpix & 1)
	{
		UINT16 pix = src[firstpix / 2];
		if ((!(flags & 4) || pix) && (UINT32)xpos < 360)
		{
			if (!(flags & 2))
				scanline[xpos] = pix;
			else
				BLEND(scanline[xpos], pix);
		}
		xpos += dxpos;
	}

	firstpix /= 2;
	iwidth /= 2;
	iwidth -= firstpix;

	while (iwidth-- > 0)
	{
		UINT32 pix = src[firstpix++];
		if (!(flags & 4) || pix)
		{
			if ((!(flags & 4) || (pix >> 16)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = pix >> 16;
				else
					BLEND(scanline[xpos], pix >> 16);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0xffff)) && (UINT32)xpos < 360)
			{
				if (!(flags & 2))
					scanline[xpos] = pix;
				else
					BLEND(scanline[xpos], pix);
			}
			xpos += dxpos;
		}
		else
			xpos += 2 * dxpos;
	}
}

static void bitmap_16_0(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_16_draw(firstpix, iwidth, src, xpos, 0, 1);
}

static void bitmap_16_1(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_16_draw(firstpix, iwidth, src, xpos, 1, -1);
}

static void bitmap_16_2(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_16_draw(firstpix, iwidth, src, xpos, 2, 1);
}

static void bitmap_16_3(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_16_draw(firstpix, iwidth, src, xpos, 3, -1);
}

static void bitmap_16_4(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_16_draw(firstpix, iwidth, src, xpos, 4, 1);
}

static void bitmap_16_5(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_16_draw(firstpix, iwidth, src, xpos, 5, -1);
}

static void bitmap_16_6(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_16_draw(firstpix, iwidth, src, xpos, 6, 1);
}

static void bitmap_16_7(INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos)
{
	bitmap_16_draw(firstpix, iwidth, src, xpos, 7, -1);
}

static void (*bitmap16[8])(INT32, INT32, UINT32 *, INT32) =
{
	bitmap_16_0,
	bitmap_16_1,
	bitmap_16_2,
	bitmap_16_3,
	bitmap_16_4,
	bitmap_16_5,
	bitmap_16_6,
	bitmap_16_7
};



INLINE UINT8 lookup_pixel(const UINT32 *src, int i, int pitch, int depth)
{
	int ppl			= 32 / depth;
	UINT32 data	= src[((i & ppl) / ppl) + ((i / (ppl*2)) * 2 * pitch)];
	UINT8 pix		= (data >> ((~i & (ppl-1)) * depth)) & ((1 << depth) - 1);
	return pix;
}



/*************************************
 *
 *  Standard bitmap processor
 *
 *************************************/

static UINT32 *process_bitmap(UINT32 *objdata, int vc, int logit)
{
	/* extract minimal data */
	UINT32 upper = objdata[0];
	UINT32 lower = objdata[1];
	UINT32 ypos = (lower >> 3) & 0x7ff;
	UINT32 height = (lower >> 14) & 0x3ff;
	UINT32 link = (lower >> 24) | ((upper & 0x7ff) << 8);
	UINT32 data = (upper >> 11);
	UINT32 *src = (UINT32 *)get_jaguar_memory(data << 3);

	if (logit)
	{
		/* second phrase */
		UINT32 upper2 = objdata[2];
		UINT32 lower2 = objdata[3];

		/* extract data */
		INT32 xpos = (INT32)(lower2 << 20) >> 20;
		UINT8 depth = 1 << ((lower2 >> 12) & 7);
		UINT8 pitch = (lower2 >> 15) & 7;
		UINT32 dwidth = (lower2 >> 18) & 0x3ff;
		INT32 iwidth = (lower2 >> 28) | ((upper2 & 0x3f) << 4);
		UINT8 _index = (upper2 >> 6) & 0x3f;
		UINT8 flags = (upper2 >> 13) & 0x0f;
		UINT8 firstpix = (upper2 >> 17) & 0x3f;

		logerror("        ypos=%X height=%X link=%06X data=%06X\n", ypos, height, link << 3, data << 3);
		logerror("        xpos=%X depth=%X pitch=%X dwidth=%X iwidth=%X index=%X flags=%X firstpix=%X\n", xpos, depth, pitch, dwidth, iwidth, _index, flags, firstpix);
	}

	/* only render if valid */
	if (vc >= ypos && height > 0 && src)
	{
		/* second phrase */
		UINT32 upper2 = objdata[2];
		UINT32 lower2 = objdata[3];

		/* extract data */
		INT32 xpos = (INT32)(lower2 << 20) >> 20;
		UINT8 depthlog = (lower2 >> 12) & 7;
		UINT8 pitch = (lower2 >> 15) & 7;
		UINT32 dwidth = (lower2 >> 18) & 0x3ff;
		UINT32 iwidth = ((lower2 >> 28) | ((upper2 & 0x3f) << 4)) << (6 - depthlog);
		UINT8 _index = (upper2 >> 5) & 0xfe;
		UINT8 flags = (upper2 >> 13) & 0x0f;
		UINT8 firstpix = ((upper2 >> 17) & 0x1f) >> depthlog;
		int i, dxpos = (flags & 1) ? -1 : 1;

		/* preadjust for firstpix */
		xpos += firstpix * dxpos;

		/* switch off the depth */
		switch (depthlog)
		{
			/* 1bpp case */
			case 0:
			{
				UINT16 *clut = (UINT16 *)jaguar_gpu_clut;
				clut += _index & 0xfe;

				/* non-blending */
				if (!(flags & 2))
				{
					for (i = firstpix; i < iwidth; i++)
					{
						UINT8 pix = lookup_pixel(src, i, pitch, 1);

						if (xpos >= 0 && xpos < 360 && (pix || !(flags & 4)))
							scanline[xpos] = clut[BYTE_XOR_BE(pix)];
						xpos += dxpos;
					}
				}

				/* blending */
				else
				{
					for (i = firstpix; i < iwidth; i++)
					{
						UINT8 pix = lookup_pixel(src, i, pitch, 1);

						if (xpos >= 0 && xpos < 360 && (pix || !(flags & 4)))
							BLEND(scanline[xpos], clut[BYTE_XOR_BE(pix)]);
						xpos += dxpos;
					}
				}
				break;
			}

			/* 2bpp case */
			case 1:
			{
				UINT16 *clut = (UINT16 *)jaguar_gpu_clut;
				clut += _index & 0xfc;

				/* non-blending */
				if (!(flags & 2))
				{
					for (i = firstpix; i < iwidth; i++)
					{
						UINT8 pix = lookup_pixel(src, i, pitch, 2);

						if (xpos >= 0 && xpos < 360 && (pix || !(flags & 4)))
							scanline[xpos] = clut[BYTE_XOR_BE(pix)];
						xpos += dxpos;
					}
				}

				/* blending */
				else
				{
					for (i = firstpix; i < iwidth; i++)
					{
						UINT8 pix = lookup_pixel(src, i, pitch, 2);

						if (xpos >= 0 && xpos < 360 && (pix || !(flags & 4)))
							BLEND(scanline[xpos], clut[BYTE_XOR_BE(pix)]);
						xpos += dxpos;
					}
				}
				break;
			}

			/* 4bpp case */
			case 2:
				/* only handle pitch=1 for now */
				if (pitch != 1)
					logerror("Unhandled pitch = %d\n", pitch);

				clutbase = (UINT16 *)jaguar_gpu_clut + (_index & 0xf0);
				(*bitmap4[flags & 7])(firstpix, iwidth, src, xpos);
				break;

			/* 8bpp case */
			case 3:
				/* only handle pitch=1 for now */
				if (pitch != 1)
					logerror("Unhandled pitch = %d\n", pitch);

				clutbase = (UINT16 *)jaguar_gpu_clut;
				(*bitmap8[flags & 7])(firstpix, iwidth, src, xpos);
				break;

			/* 16bpp case */
			case 4:
				/* only handle pitch=1 for now */
				if (pitch != 1)
					logerror("Unhandled pitch = %d\n", pitch);

				(*bitmap16[flags & 7])(firstpix, iwidth, src, xpos);
				break;

			default:
				fprintf(stderr, "Unhandled bitmap source depth = %d\n", depthlog);
				break;
		}

		/* decrement the height and add to the source data offset */
		objdata[0] = upper + (dwidth << 11);
		objdata[1] = lower - (1 << 14);
	}

	return (UINT32 *)get_jaguar_memory(link << 3);
}



/*************************************
 *
 *  Scaled bitmap object processor
 *
 *************************************/

static UINT32 *process_scaled_bitmap(UINT32 *objdata, int vc, int logit)
{
	/* extract data */
	UINT32 upper = objdata[0];
	UINT32 lower = objdata[1];
	UINT32 ypos = (lower >> 3) & 0x7ff;
	UINT32 height = (lower >> 14) & 0x3ff;
	UINT32 link = (lower >> 24) | ((upper & 0x7ff) << 8);
	UINT32 data = (upper >> 11);
	UINT32 *src = (UINT32 *)get_jaguar_memory(data << 3);

	/* third phrase */
	UINT32 lower3 = objdata[5];
	INT32 remainder = (lower3 >> 16) & 0xff;

	if (logit)
	{
		/* second phrase */
		UINT32 upper2 = objdata[2];
		UINT32 lower2 = objdata[3];

		/* extract data */
		INT32 xpos = (INT32)(lower2 << 20) >> 20;
		UINT8 depth = 1 << ((lower2 >> 12) & 7);
		UINT8 pitch = (lower2 >> 15) & 7;
		UINT32 dwidth = (lower2 >> 18) & 0x3ff;
		INT32 iwidth = (lower2 >> 28) | ((upper2 & 0x3f) << 4);
		UINT8 _index = (upper2 >> 6) & 0x3f;
		UINT8 flags = (upper2 >> 13) & 0x0f;
		UINT8 firstpix = (upper2 >> 17) & 0x3f;

		INT32 hscale = lower3 & 0xff;
		INT32 vscale = (lower3 >> 8) & 0xff;

		logerror("        ypos=%X height=%X link=%06X data=%06X\n", ypos, height, link << 3, data << 3);
		logerror("        xpos=%X depth=%X pitch=%X dwidth=%X iwidth=%X index=%X flags=%X firstpix=%X\n", xpos, depth, pitch, dwidth, iwidth, _index, flags, firstpix);
		logerror("        hscale=%X vscale=%X remainder=%X\n", hscale, vscale, remainder);
	}

	/* only render if valid */
	if (vc >= ypos && (height > 0 || remainder > 0) && src)
	{
		/* second phrase */
		UINT32 upper2 = objdata[2];
		UINT32 lower2 = objdata[3];

		/* extract data */
		INT32 xpos = (INT32)(lower2 << 20) >> 20;
		UINT8 depthlog = (lower2 >> 12) & 7;
		UINT8 pitch = (lower2 >> 15) & 7;
		UINT32 dwidth = (lower2 >> 18) & 0x3ff;
		INT32 iwidth = ((lower2 >> 28) | ((upper2 & 0x3f) << 4)) << (6 - depthlog);
		UINT8 _index = (upper2 >> 5) & 0xfe;
		UINT8 flags = (upper2 >> 13) & 0x0f;
		UINT8 firstpix = ((upper2 >> 17) & 0x1f) >> depthlog;

		INT32 hscale = lower3 & 0xff;
		INT32 vscale = (lower3 >> 8) & 0xff;
		INT32 xleft = hscale;
		int dxpos = (flags & 1) ? -1 : 1;
		int xpix = firstpix, yinc;

		/* only handle pitch=0 for now */
		if (pitch != 1)
			logerror("Unhandled pitch = %d\n", pitch);
		if (flags & 2)
		{
			mame_printf_debug("Unhandled blend mode in scaled bitmap case\n");
			logerror("Unhandled blend mode in scaled bitmap case\n");
		}

		/* preadjust for firstpix */
		xpos += firstpix * dxpos;

		/* ignore hscale = 0 */
		if (hscale != 0)
		{
			/* switch off the depth */
			switch (depthlog)
			{
				case 2:
				{
					UINT16 *clut = (UINT16 *)jaguar_gpu_clut;
					clut += _index & 0xf0;

					/* render in phrases */
					while (xpix < iwidth)
					{
						UINT16 pix = (src[xpix / 8] >> ((~xpix & 7) * 4)) & 0x0f;

						while (xleft > 0)
						{
							if (xpos >= 0 && xpos < 360 && (pix || !(flags & 4)))
								scanline[xpos] = clut[BYTE_XOR_BE(pix)];
							xpos += dxpos;
							xleft -= 0x20;
						}
						while (xleft <= 0)
							xleft += hscale, xpix++;
					}
					break;
				}

				case 3:
				{
					UINT16 *clut = (UINT16 *)jaguar_gpu_clut;

					/* render in phrases */
					while (xpix < iwidth)
					{
						UINT16 pix = (src[xpix / 4] >> ((~xpix & 3) * 8)) & 0xff;

						while (xleft > 0)
						{
							if (xpos >= 0 && xpos < 360 && (pix || !(flags & 4)))
								scanline[xpos] = clut[BYTE_XOR_BE(pix)];
							xpos += dxpos;
							xleft -= 0x20;
						}
						while (xleft <= 0)
							xleft += hscale, xpix++;
					}
					break;
				}

				case 4:
					while (xpix < iwidth)
					{
						UINT16 pix = src[xpix / 2] >> ((~xpix & 1) * 16);

						while (xleft > 0)
						{
							if (xpos >= 0 && xpos < 360 && (pix || !(flags & 4)))
								scanline[xpos] = pix;
							xpos += dxpos;
							xleft -= 0x20;
						}
						while (xleft <= 0)
							xleft += hscale, xpix++;
					}
					break;

				default:
					fprintf(stderr, "Unhandled scaled bitmap source depth = %d\n", depthlog);
					break;
			}
		}

		/* handle Y scale */
		remainder -= 0x20;
		yinc = 0;
		while (remainder <= 0 && vscale != 0)
			remainder += vscale, yinc++;
		if (yinc > height)
			yinc = height, remainder = 0;

		/* decrement the height and add to the source data offset */
		objdata[0] = upper + yinc * (dwidth << 11);
		objdata[1] = lower - yinc * (1 << 14);
		objdata[5] = (lower3 & ~0xff0000) | ((remainder & 0xff) << 16);
	}

	return (UINT32 *)get_jaguar_memory(link << 3);
}



/*************************************
 *
 *  Brach object processor
 *
 *************************************/

static UINT32 *process_branch(UINT32 *objdata, int vc, int logit)
{
	UINT32 upper = objdata[0];
	UINT32 lower = objdata[1];
	UINT32 ypos = (lower >> 3) & 0x7ff;
	UINT32 cc = (lower >> 14) & 7;
	UINT32 link = (lower >> 24) | ((upper & 0x7ff) << 8);
	int taken = 0;

#ifndef MESS
	if ((ypos & 1) && ypos != 0x7ff)
		fprintf(stderr, "        branch cc=%d ypos=%X link=%06X - \n", cc, ypos, link << 3);
#endif

	switch (cc)
	{
		/* 0: branch if ypos == vc or ypos == 0x7ff */
		case 0:
			if (logit) logerror("        branch if %X == vc or %X == 0x7ff to %06X\n", ypos, ypos, link << 3);
			taken = (ypos == vc) || (ypos == 0x7ff);
			break;

		/* 1: branch if ypos > vc */
		case 1:
			if (logit) logerror("        branch if %X > vc to %06X\n", ypos, link << 3);
			taken = (ypos > vc);
			break;

		/* 2: branch if ypos < vc */
		case 2:
			if (logit) logerror("        branch if %X < vc to %06X\n", ypos, link << 3);
			taken = (ypos < vc);
			break;

		/* 3: branch if object processor flag is set */
		case 3:
			if (logit) logerror("        branch if object flag set to %06X\n", link << 3);
			fprintf(stderr, "Unhandled branch!\n");
			link = 0, taken = 1;
			break;

		/* 4: branch on second half of display line */
		case 4:
			if (logit) logerror("        branch if second half of line to %06X\n", link << 3);
			taken = (vc & 1);
			break;

		default:
			fprintf(stderr, "Invalid branch!\n");
			link = 0, taken = 1;
			break;
	}

	/* handle the branch */
	return taken ? (UINT32 *)get_jaguar_memory(link << 3) : (objdata + 2);
}



/*************************************
 *
 *  Process object list
 *
 *************************************/

static void process_object_list(mame_bitmap *bitmap, const rectangle *cliprect)
{
	UINT32 *objdata;
	int y, x, pass;
	int logit;

	/* loop over all scanlines */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		/* erase the scanline first */
		for (x = 0; x < 336; x++)
			scanline[x] = gpu_regs[BG];

#if LOG_OBJECTS
		logit = (y == cliprect->min_y);
#else
		logit = 0;
#endif

		/* two passes per scanline */
		for (pass = 0; pass < 1; pass++)
		{
			int vc = y * 2 + pass + gpu_regs[VBE];
			int done = 0, count = 0;

			/* fetch the object pointer */
			objdata = (UINT32 *)get_jaguar_memory((gpu_regs[OLP_H] << 16) | gpu_regs[OLP_L]);
			while (!done && objdata && count++ < 100)
			{
				/* the low 3 bits determine the command */
				switch (objdata[1] & 7)
				{
					/* bitmap object */
					case 0:
						if (logit)
							logerror("bitmap = %08X-%08X %08X-%08X\n", objdata[0], objdata[1], objdata[2], objdata[3]);
						objdata = process_bitmap(objdata, vc, logit);
						break;

					/* scaled bitmap object */
					case 1:
						if (logit)
							logerror("scaled = %08X-%08X %08X-%08X %08X-%08X\n", objdata[0], objdata[1], objdata[2], objdata[3], objdata[4], objdata[5]);
						objdata = process_scaled_bitmap(objdata, vc, logit);
						break;

					/* branch */
					case 3:
						if (logit)
							logerror("branch = %08X-%08X\n", objdata[0], objdata[1]);
						objdata = process_branch(objdata, vc, logit);
						break;

					/* stop */
					case 4:
					{
						int interrupt = (objdata[1] >> 3) & 1;
						done = 1;

						if (logit)
							logerror("stop   = %08X-%08X\n", objdata[0], objdata[1]);
						if (interrupt)
						{
#ifndef MESS
							fprintf(stderr, "stop int=%d\n", interrupt);
#endif
							cpu_irq_state |= 4;
							update_cpu_irq();
						}
						break;
					}

					default:
						fprintf(stderr, "%08X %08X\n", objdata[0], objdata[1]);
						done = 1;
						break;
				}
			}
		}

		/* render this scanline */
		draw_scanline16(bitmap, 0, y, 336, scanline, pen_table, -1);
	}
}
