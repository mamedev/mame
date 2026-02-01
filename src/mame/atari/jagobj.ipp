// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Jaguar object processor

****************************************************************************/

#ifndef INCLUDE_OBJECT_PROCESSOR
#error jagobj.ipp is not directly compilable!
#endif


/*************************************
 *
 *  Object processor init
 *
 *************************************/

void jaguar_state::jagobj_init()
{
	int i;

	/* fill tables */
	for (i = 0; i < 256 * 256; i++)
	{
		int y = (i >> 8) & 0xff;
		int dy = (int8_t)i;
		int c1 = (i >> 8) & 0x0f;
		int dc1 = (int8_t)(i << 4) >> 4;
		int c2 = (i >> 12) & 0x0f;
		int dc2 = (int8_t)(i & 0xf0) >> 4;

		y += dy;
		if (y < 0) y = 0;
		else if (y > 0xff) y = 0xff;
		m_blend_y[i] = y;

		c1 += dc1;
		if (c1 < 0) c1 = 0;
		else if (c1 > 0x0f) c1 = 0x0f;
		c2 += dc2;
		if (c2 < 0) c2 = 0;
		else if (c2 > 0x0f) c2 = 0x0f;
		m_blend_cc[i] = (c2 << 4) | c1;
	}
}



/*************************************
 *
 *  Blending function
 *
 *************************************/

#define BLEND(dst, src)     \
	(dst) = (m_blend_cc[((dst) & 0xff00) | (((src) >> 8) & 0xff)] << 8) | m_blend_y[(((dst) & 0xff) << 8) | ((src) & 0xff)];



/*************************************
 *
 *  4bpp bitmap renderers
 *
 *************************************/

inline void jaguar_state::bitmap_4_draw(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint8_t flags, int32_t dxpos, uint16_t *clutbase)
{
	if (firstpix & 7)
	{
		uint32_t pixsrc = src[firstpix >> 3];
		while (firstpix & 7)
		{
			int pix = (pixsrc >> ((~firstpix & 7) << 2)) & 0x0f;
			if ((!(flags & 4) || pix) && (uint32_t)xpos < 760)
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

	firstpix >>= 3;
	iwidth >>= 3;
	iwidth -= firstpix;

	while (iwidth-- > 0)
	{
		uint32_t pix = src[firstpix++];
		if (!(flags & 4) || pix)
		{
			if ((!(flags & 4) || (pix & 0xf0000000)) && (uint32_t)xpos < 760)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE(pix >> 28)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE(pix >> 28)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x0f000000)) && (uint32_t)xpos < 760)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 24) & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 24) & 0x0f)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x00f00000)) && (uint32_t)xpos < 760)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 20) & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 20) & 0x0f)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x000f0000)) && (uint32_t)xpos < 760)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 16) & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 16) & 0x0f)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x0000f000)) && (uint32_t)xpos < 760)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 12) & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 12) & 0x0f)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x00000f00)) && (uint32_t)xpos < 760)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 8) & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 8) & 0x0f)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x000000f0)) && (uint32_t)xpos < 760)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 4) & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 4) & 0x0f)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x0000000f)) && (uint32_t)xpos < 760)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE(pix & 0x0f)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE(pix & 0x0f)]);
			}
			xpos += dxpos;
		}
		else
			xpos += dxpos << 3;
	}
}

void jaguar_state::bitmap_4_0(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_4_draw(scanline, firstpix, iwidth, src, xpos, 0, 1, clutbase);
}

void jaguar_state::bitmap_4_1(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_4_draw(scanline, firstpix, iwidth, src, xpos, 1, -1, clutbase);
}

void jaguar_state::bitmap_4_2(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_4_draw(scanline, firstpix, iwidth, src, xpos, 2, 1, clutbase);
}

void jaguar_state::bitmap_4_3(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_4_draw(scanline, firstpix, iwidth, src, xpos, 3, -1, clutbase);
}

void jaguar_state::bitmap_4_4(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_4_draw(scanline, firstpix, iwidth, src, xpos, 4, 1, clutbase);
}

void jaguar_state::bitmap_4_5(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_4_draw(scanline, firstpix, iwidth, src, xpos, 5, -1, clutbase);
}

void jaguar_state::bitmap_4_6(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_4_draw(scanline, firstpix, iwidth, src, xpos, 6, 1, clutbase);
}

void jaguar_state::bitmap_4_7(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_4_draw(scanline, firstpix, iwidth, src, xpos, 7, -1, clutbase);
}

void (jaguar_state::*const jaguar_state::bitmap4[8])(uint16_t *, int32_t, int32_t, uint32_t *, int32_t, uint16_t *) =
{
	&jaguar_state::bitmap_4_0,
	&jaguar_state::bitmap_4_1,
	&jaguar_state::bitmap_4_2,
	&jaguar_state::bitmap_4_3,
	&jaguar_state::bitmap_4_4,
	&jaguar_state::bitmap_4_5,
	&jaguar_state::bitmap_4_6,
	&jaguar_state::bitmap_4_7
};



/*************************************
 *
 *  8bpp bitmap renderers
 *
 *************************************/

inline void jaguar_state::bitmap_8_draw(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint8_t flags, int32_t dxpos, uint16_t *clutbase)
{
	if (firstpix & 3)
	{
		uint32_t pixsrc = src[firstpix >> 2];
		while (firstpix & 3)
		{
			uint8_t pix = pixsrc >> ((~firstpix & 3) << 3);
			if ((!(flags & 4) || pix) && (uint32_t)xpos < 760)
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

	firstpix >>= 2;
	iwidth >>= 2;
	iwidth -= firstpix;

	while (iwidth-- > 0)
	{
		uint32_t pix = src[firstpix++];
		if (!(flags & 4) || pix)
		{
			if ((!(flags & 4) || (pix & 0xff000000)) && (uint32_t)xpos < 760)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE(pix >> 24)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE(pix >> 24)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x00ff0000)) && (uint32_t)xpos < 760)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 16) & 0xff)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 16) & 0xff)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x0000ff00)) && (uint32_t)xpos < 760)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE((pix >> 8) & 0xff)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE((pix >> 8) & 0xff)]);
			}
			xpos += dxpos;

			if ((!(flags & 4) || (pix & 0x000000ff)) && (uint32_t)xpos < 760)
			{
				if (!(flags & 2))
					scanline[xpos] = clutbase[BYTE_XOR_BE(pix & 0xff)];
				else
					BLEND(scanline[xpos], clutbase[BYTE_XOR_BE(pix & 0xff)]);
			}
			xpos += dxpos;
		}
		else
			xpos += dxpos << 2;
	}
}

void jaguar_state::bitmap_8_0(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_8_draw(scanline, firstpix, iwidth, src, xpos, 0, 1, clutbase);
}

void jaguar_state::bitmap_8_1(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_8_draw(scanline, firstpix, iwidth, src, xpos, 1, -1, clutbase);
}

void jaguar_state::bitmap_8_2(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_8_draw(scanline, firstpix, iwidth, src, xpos, 2, 1, clutbase);
}

void jaguar_state::bitmap_8_3(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_8_draw(scanline, firstpix, iwidth, src, xpos, 3, -1, clutbase);
}

void jaguar_state::bitmap_8_4(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_8_draw(scanline, firstpix, iwidth, src, xpos, 4, 1, clutbase);
}

void jaguar_state::bitmap_8_5(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_8_draw(scanline, firstpix, iwidth, src, xpos, 5, -1, clutbase);
}

void jaguar_state::bitmap_8_6(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_8_draw(scanline, firstpix, iwidth, src, xpos, 6, 1, clutbase);
}

void jaguar_state::bitmap_8_7(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase)
{
	bitmap_8_draw(scanline, firstpix, iwidth, src, xpos, 7, -1, clutbase);
}

void (jaguar_state::*const jaguar_state::bitmap8[8])(uint16_t *, int32_t, int32_t, uint32_t *, int32_t, uint16_t *) =
{
	&jaguar_state::bitmap_8_0,
	&jaguar_state::bitmap_8_1,
	&jaguar_state::bitmap_8_2,
	&jaguar_state::bitmap_8_3,
	&jaguar_state::bitmap_8_4,
	&jaguar_state::bitmap_8_5,
	&jaguar_state::bitmap_8_6,
	&jaguar_state::bitmap_8_7
};


static inline uint8_t lookup_pixel(const uint32_t *src, int i, int pitch, int depth)
{
	int ppl     = 32 / depth;
	uint32_t data = src[((i & ppl) / ppl) + ((i / (ppl<<1)) * (pitch<<1))];
	uint8_t pix   = (data >> ((~i & (ppl-1)) * depth)) & ((1 << depth) - 1);
	return pix;
}



/*************************************
 *
 *  Standard bitmap processor
 *
 *************************************/

// TODO: convert objdata to 64-bit
uint32_t jaguar_state::process_bitmap(uint16_t *scanline, uint32_t *objdata, int vc)
{
	/* extract minimal data */
	uint32_t upper = objdata[0];
	uint32_t lower = objdata[1];
	uint16_t ypos = (lower >> 3) & 0x7ff;
	uint32_t height = (lower >> 14) & 0x3ff;
	uint32_t link = (lower >> 24) | ((upper & 0x7ff) << 8);
	uint32_t data = (upper >> 11);
	uint32_t *src = (uint32_t *)memory_base(data << 3);

	/* debug logging */
	{
		/* second phrase */
		uint32_t upper2 = objdata[2];
		uint32_t lower2 = objdata[3];

		/* extract data */
		int16_t xpos = util::sext(lower2 & 0xfff, 12);
		uint8_t depth = 1 << ((lower2 >> 12) & 7);
		uint8_t pitch = (lower2 >> 15) & 7;
		uint32_t dwidth = (lower2 >> 18) & 0x3ff;
		int32_t iwidth = (lower2 >> 28) | ((upper2 & 0x3f) << 4);
		uint8_t _index = (upper2 >> 6) & 0x3f;
		// bit 0: REFLECT (a.k.a. flip X)
		// bit 1: RMW
		// bit 2: TRANS(parent)
		// bit 3: RELEASE (bus)
		uint8_t flags = (upper2 >> 13) & 0x0f;
		uint8_t firstpix = (upper2 >> 17) & 0x3f;

		LOGMASKED(LOG_OBJECT_DRAW, "        ypos=%X height=%X link=%06X data=%06X\n", ypos, height, link << 3, data << 3);
		LOGMASKED(LOG_OBJECT_DRAW, "        xpos=%X depth=%X pitch=%X dwidth=%X iwidth=%X index=%X flags=%X firstpix=%X\n", xpos, depth, pitch, dwidth, iwidth, _index, flags, firstpix);
	}

	/* only render if valid */
	if (vc >= ypos && height > 0 && src)
	{
		/* second phrase */
		uint32_t upper2 = objdata[2];
		uint32_t lower2 = objdata[3];

		/* extract data */
		int16_t xpos = util::sext(lower2 & 0xfff, 12);
		uint8_t depthlog = (lower2 >> 12) & 7;
		uint8_t pitch = (lower2 >> 15) & 7;
		uint32_t dwidth = ((lower2 >> 18) & 0x3ff);
		// Needs to be in s32 in particular for 24bpp mode
		int32_t iwidth = ((lower2 >> 28) | ((upper2 & 0x3f) << 4)) << (6 - depthlog);
		uint8_t _index = (upper2 >> 5) & 0xfe;
		uint8_t flags = (upper2 >> 13) & 0x07;
		// needs to be in u32 because this variable is reused in inner loops
		uint32_t firstpix = ((upper2 >> 17) & 0x3f) >> depthlog;
		int i, dxpos = (flags & 1) ? -1 : 1;

		// TODO: iwidth == 0 clamps to 1
		// easy to fix, need use cases
		// rayman & ultravor on transitions
		//if (iwidth == 0)
		//  popmessage("jagobj.ipp: iwidth == 0!");

		/* switch off the depth */
		switch (depthlog)
		{
			/* 1bpp case */
			case 0:
			{
				uint16_t *clut = (uint16_t *)&m_gpu_clut[0] + _index;
				xpos += firstpix * dxpos;

				/* non-blending */
				if (!(flags & 2))
				{
					for (i = firstpix; i < iwidth; i++)
					{
						uint8_t pix = lookup_pixel(src, i, pitch, 1);

						if (xpos >= 0 && xpos < 760 && (pix || !(flags & 4)))
							scanline[xpos] = clut[BYTE_XOR_BE(pix)];
						xpos += dxpos;
					}
				}

				/* blending */
				else
				{
					for (i = firstpix; i < iwidth; i++)
					{
						uint8_t pix = lookup_pixel(src, i, pitch, 1);

						if (xpos >= 0 && xpos < 760 && (pix || !(flags & 4)))
							BLEND(scanline[xpos], clut[BYTE_XOR_BE(pix)]);
						xpos += dxpos;
					}
				}
				break;
			}

			/* 2bpp case */
			case 1:
			{
				uint16_t *clut = (uint16_t *)&m_gpu_clut[0] + (_index & 0xfc);
				xpos += firstpix * dxpos;

				/* non-blending */
				if (!(flags & 2))
				{
					for (i = firstpix; i < iwidth; i++)
					{
						uint8_t pix = lookup_pixel(src, i, pitch, 2);

						if (xpos >= 0 && xpos < 760 && (pix || !(flags & 4)))
							scanline[xpos] = clut[BYTE_XOR_BE(pix)];
						xpos += dxpos;
					}
				}

				/* blending */
				else
				{
					for (i = firstpix; i < iwidth; i++)
					{
						uint8_t pix = lookup_pixel(src, i, pitch, 2);

						if (xpos >= 0 && xpos < 760 && (pix || !(flags & 4)))
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
					logerror("Unhandled pitch = %d at 4bpp\n", pitch);
				xpos += firstpix * dxpos;

				(this->*bitmap4[flags])(scanline, firstpix, iwidth, src, xpos, (uint16_t *)&m_gpu_clut[0] + (_index & 0xf8));
				break;

			/* 8bpp case */
			case 3:
				/* only handle pitch=1 for now */
				if (pitch != 1)
					logerror("Unhandled pitch = %d at 8bpp\n", pitch);
				xpos += firstpix * dxpos;

				(this->*bitmap8[flags])(scanline, firstpix, iwidth, src, xpos, (uint16_t *)&m_gpu_clut[0]);
				break;

			/* 16bpp case */
			case 4:
				{
					// TODO: firstpix matters only on <= 8bpp objects
					// firstpix &= 0x3e;
					xpos += firstpix * dxpos;

					while (iwidth > 0)
					{
						uint64_t datax = ((u64)src[firstpix] << 32) | (src[firstpix + 1]);
						firstpix += pitch << 1;

						for (i = 0; i < 4 && iwidth > 0; i++, iwidth--)
						{
							u16 pix = (datax >> ((3 - i) * 16)) & 0xffff;
							if ((!(flags & 4) || pix) && xpos == std::clamp(xpos, (int16_t)0, (int16_t)759))
							{
								if (!(flags & 2))
									scanline[xpos] = pix;
								else
									BLEND(scanline[xpos], pix);
							}

							xpos += dxpos;
						}
					}

				}
				break;

			/* 24bpp case */
			// - ironsold on title screen and attract mode
			case 5:
				{
					// TODO: firstpix matters only on <= 8bpp objects
					//firstpix &= 0x3e;
					xpos += firstpix * dxpos;
					//iwidth -= firstpix;

					while (iwidth > 0)
					{
						uint64_t datax = ((u64)src[firstpix] << 32) | (src[firstpix + 1]);
						firstpix += pitch << 1;

						for (i = 0; i < 2 && iwidth > 0; i++, iwidth--)
						{
							u32 pix = (datax >> ((1 - i) * 32)) & 0xffffffff;

							// NOTE: 24bpp shouldn't support RMW
							if ((!(flags & 6) || pix) && xpos == std::clamp(xpos, (int16_t)0, (int16_t)759))
							{
								scanline[xpos + 0] = pix >> 16;
								scanline[xpos + 1] = pix & 0xffff;
								//else
								//  BLEND(scanline[xpos], pix);
							}

							xpos += dxpos * 2;
						}
					}
				}
				break;

			default:
				fprintf(stderr, "Unhandled bitmap source depth = %d\n", depthlog);
				break;
		}

		/* decrement the height and add to the source data offset */
		objdata[0] = (upper & 0x7ff) | ((upper + (dwidth << 11)) & ~0x7ff);
		objdata[1] = (lower & ~0xffc000) | ((lower - (1 << 14)) & 0xffc000);
	}

	return link << 3;
}



/*************************************
 *
 *  Scaled bitmap object processor
 *
 *************************************/

uint32_t jaguar_state::process_scaled_bitmap(uint16_t *scanline, uint32_t *objdata, int vc)
{
	/* extract data */
	uint32_t upper = objdata[0];
	uint32_t lower = objdata[1];
	uint16_t ypos = (lower >> 3) & 0x7ff;
	uint32_t height = (lower >> 14) & 0x3ff;
	uint32_t link = (lower >> 24) | ((upper & 0x7ff) << 8);
	uint32_t data = (upper >> 11);
	uint32_t *src = (uint32_t *)memory_base(data << 3);

	/* third phrase */
	uint32_t lower3 = objdata[5];
	int32_t remainder = (lower3 >> 16) & 0xff;

	/* debug logging */
	{
		/* second phrase */
		uint32_t upper2 = objdata[2];
		uint32_t lower2 = objdata[3];

		/* extract data */
		int16_t xpos = util::sext(lower2 & 0xfff, 12);
		uint8_t depth = 1 << ((lower2 >> 12) & 7);
		uint8_t pitch = (lower2 >> 15) & 7;
		uint32_t dwidth = (lower2 >> 18) & 0x3ff;
		uint32_t iwidth = (lower2 >> 28) | ((upper2 & 0x3f) << 4);
		uint8_t _index = (upper2 >> 6) & 0x3f;
		uint8_t flags = (upper2 >> 13) & 0x0f;
		uint8_t firstpix = (upper2 >> 17) & 0x3f;

		int32_t hscale = lower3 & 0xff;
		int32_t vscale = (lower3 >> 8) & 0xff;

		LOGMASKED(LOG_OBJECT_DRAW, "        ypos=%X height=%X link=%06X data=%06X\n", ypos, height, link << 3, data << 3);
		LOGMASKED(LOG_OBJECT_DRAW, "        xpos=%X depth=%X pitch=%X dwidth=%X iwidth=%X index=%X flags=%X firstpix=%X\n", xpos, depth, pitch, dwidth, iwidth, _index, flags, firstpix);
		LOGMASKED(LOG_OBJECT_DRAW, "        hscale=%X vscale=%X remainder=%X\n", hscale, vscale, remainder);
	}

	/* only render if valid */
	if (vc >= ypos && (height > 0 || remainder > 0) && src)
	{
		/* second phrase */
		uint32_t upper2 = objdata[2];
		uint32_t lower2 = objdata[3];

		/* extract data */
		int16_t xpos = util::sext(lower2 & 0xfff, 12);
		uint8_t depthlog = (lower2 >> 12) & 7;
		uint8_t pitch = (lower2 >> 15) & 7;
		uint32_t dwidth = (lower2 >> 18) & 0x3ff;
		int32_t iwidth = ((lower2 >> 28) | ((upper2 & 0x3f) << 4)) << (6 - depthlog);
		uint8_t _index = (upper2 >> 5) & 0xfe;
		uint8_t flags = (upper2 >> 13) & 0x07;
		uint32_t firstpix = ((upper2 >> 17) & 0x3f) >> depthlog;

		int32_t hscale = lower3 & 0xff;
		int32_t vscale = (lower3 >> 8) & 0xff;
		int32_t xleft = hscale;
		int dxpos = (flags & 1) ? -1 : 1;
		int xpix = firstpix, yinc;

		/* preadjust for firstpix */
		xpos += firstpix * dxpos;

		/* ignore hscale = 0 */
		if (hscale != 0)
		{
			/* switch off the depth */
			switch (depthlog)
			{
				// 1bpp
				case 0:
				{
					/* only handle pitch=1 (sequential data) for now */
					if (pitch != 1)
						logerror("Unhandled pitch = %d in 1bpp scaled bitmap\n", pitch);
					if (flags & 2)
						logerror("Unhandled blend mode in 1bpp scaled bitmap\n");

					uint16_t *clut = (uint16_t *)&m_gpu_clut[0] + _index;

					/* render in phrases */
					while (xpix < iwidth)
					{
						uint16_t pix = (src[xpix >> 5] >> (~xpix & 31)) & 0x01;

						while (xleft > 0)
						{
							if (xpos >= 0 && xpos < 760 && (pix || !(flags & 4)))
								scanline[xpos] = clut[BYTE_XOR_BE(pix)];
							xpos += dxpos;
							xleft -= 0x20;
						}
						while (xleft <= 0)
							xleft += hscale, xpix++;
					}
					break;
				}

				// 2bpp
				case 1:
				{
					/* only handle pitch=1 (sequential data) for now */
					if (pitch != 1)
						logerror("Unhandled pitch = %d in 2bpp scaled mode\n", pitch);
					if (flags & 2)
						logerror("Unhandled blend mode in 2bpp scaled bitmap\n");

					uint16_t *clut = (uint16_t *)&m_gpu_clut[0] + (_index & 0xfc);

					/* render in phrases */
					while (xpix < iwidth)
					{
						uint16_t pix = (src[xpix >> 4] >> ((~xpix & 15) << 1)) & 0x03;

						while (xleft > 0)
						{
							if (xpos >= 0 && xpos < 760 && (pix || !(flags & 4)))
								scanline[xpos] = clut[BYTE_XOR_BE(pix)];
							xpos += dxpos;
							xleft -= 0x20;
						}
						while (xleft <= 0)
							xleft += hscale, xpix++;
					}
					break;
				}

				// 4bpp
				case 2:
				{
					/* only handle pitch=1 (sequential data) for now */
					if (pitch != 1)
						logerror("Unhandled pitch = %d in 4bpp scaled mode\n", pitch);
					if (flags & 2)
						logerror("Unhandled blend mode in 4bpp scaled bitmap\n");

					uint16_t *clut = (uint16_t *)&m_gpu_clut[0] + (_index & 0xf8);

					/* render in phrases */
					while (xpix < iwidth)
					{
						uint16_t pix = (src[xpix >> 3] >> ((~xpix & 7) << 2)) & 0x0f;

						while (xleft > 0)
						{
							if (xpos >= 0 && xpos < 760 && (pix || !(flags & 4)))
								scanline[xpos] = clut[BYTE_XOR_BE(pix)];
							xpos += dxpos;
							xleft -= 0x20;
						}
						while (xleft <= 0)
							xleft += hscale, xpix++;
					}
					break;
				}

				// 8bpp
				case 3:
				{
					if (flags & 2)
						logerror("Unhandled blend mode in 8bpp scaled bitmap\n");

					uint16_t *clut = (uint16_t *)&m_gpu_clut[0];

					/* render in phrases */
					while (xpix < iwidth)
					{
						// - pitch on mutntpng title screen
						uint16_t pix = (src[(xpix >> 2) * pitch] >> ((~xpix & 3) << 3)) & 0xff;

						while (xleft > 0)
						{
							if (xpos >= 0 && xpos < 760 && (pix || !(flags & 4)))
							{
								scanline[xpos] = clut[BYTE_XOR_BE(pix)];
							}
							xpos += dxpos;
							xleft -= 0x20;
						}
						while (xleft <= 0)
							xleft += hscale, xpix++;
					}
					break;
				}

				// 16bpp
				case 4:
				{
					/* only handle pitch=1 (sequential data) for now */
					if (pitch != 1)
						logerror("Unhandled pitch = %d in 16bpp scaled mode\n", pitch);

					while (xpix < iwidth)
					{
						uint16_t pix = src[xpix >> 1] >> ((~xpix & 1) << 4);

						while (xleft > 0)
						{
							if (xpos >= 0 && xpos < 760 && (pix || !(flags & 4)))
							{
								// - blending in phase0 main menu (pillar at center)
								if (!(flags & 2))
									scanline[xpos] = pix;
								else
									BLEND(scanline[xpos], pix);
							}
							xpos += dxpos;
							xleft -= 0x20;
						}
						while (xleft <= 0)
							xleft += hscale, xpix++;
					}
					break;
				}

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
		objdata[0] = (upper & 0x7ff) | ((upper + yinc * (dwidth << 11)) & ~0x7ff);
		objdata[1] = (lower & ~0xffc000) | ((lower - yinc * (1 << 14)) & 0xffc000);
		objdata[5] = (lower3 & ~0xff0000) | ((remainder & 0xff) << 16);
	}

	return link << 3;
}



/*************************************
 *
 *  Branch object processor
 *
 *************************************/

uint32_t jaguar_state::process_branch(uint32_t *objdata, u32 object_pointer, int vc)
{
	uint32_t upper = objdata[0];
	uint32_t lower = objdata[1];
	uint32_t ypos = (lower >> 3) & 0x7ff;
	uint32_t cc = (lower >> 14) & 7;
	uint32_t link = (lower >> 24) | ((upper & 0x7ff) << 8);
	int taken = 0;

//  if ((ypos & 1) && ypos != 0x7ff)
//      fprintf(stderr, "        branch cc=%d ypos=%X link=%06X - \n", cc, ypos, link << 3);

	switch (cc)
	{
		/* 0: branch if ypos == vc or ypos == 0x7ff */
		case 0:
			LOGMASKED(LOG_OBJECT_BRANCH, "        branch if %X == vc or %X == 0x7ff to %06X\n", ypos, ypos, link << 3);
			taken = (ypos == vc) || (ypos == 0x7ff);
			break;

		/* 1: branch if ypos > vc */
		case 1:
			LOGMASKED(LOG_OBJECT_BRANCH, "        branch if %X > vc to %06X\n", ypos, link << 3);
			taken = (ypos > vc);
			break;

		/* 2: branch if ypos < vc */
		case 2:
			LOGMASKED(LOG_OBJECT_BRANCH, "        branch if %X < vc to %06X\n", ypos, link << 3);
			taken = (ypos < vc);
			break;

		/* 3: branch if object processor flag is set */
		case 3:
			LOGMASKED(LOG_OBJECT_BRANCH, "        branch if object flag set to %06X\n", link << 3);
			taken = m_gpu_regs[OBF] & 1;
			break;

		/* 4: branch on second half of display line */
		case 4:
			LOGMASKED(LOG_OBJECT_BRANCH, "        branch if second half of line to %06X\n", link << 3);
			// TODO: verify this one up
			taken = (vc & 1);
			break;

		default:
			fprintf(stderr, "Invalid branch!\n");
			// OP treats any other branch type as a NOP
			// - totalcar (which crashes anyway)
			taken = 0;
			//link = 0;
			break;
	}

	/* handle the branch */
	return taken ? link << 3 : (object_pointer + 8);
}



/*************************************
 *
 *  Process object list
 *
 *************************************/

void jaguar_state::process_object_list(int vc, uint16_t *scanline)
{
	int done = 0, count = 0;
	uint32_t *objdata;

	/* erase the scanline first */
	if (!m_suspend_object_pointer)
	{
		for (int x = 0; x < 760; x++)
			scanline[x] = m_gpu_regs[BG];
	}

	/* fetch the object pointer */
	u32 object_pointer = ((m_gpu_regs[OLP_H] << 16) | m_gpu_regs[OLP_L]);

	if (m_suspend_object_pointer)
	{
		object_pointer = m_suspend_object_pointer;
		m_suspend_object_pointer = 0;
	}

	// HACK: avoid a potential crash in raiden after Atari logo
	// Where it's clearly not expecting the object processor running, sets $0 minus 8 = $ffff'fff8
	if (BIT(object_pointer, 31))
		return;

	bool gpu_suspend = false;

	// TODO: count == 200 is wrong juju, particularly with branches
	// - raiden hits 115 ~ 137 objects on ranking screen
	// - ttoonadv hits 140 objects
	// - valdiser keeps looping due of said branches
	while (!done && count++ < 200)
	{
		objdata = (uint32_t *)memory_base(object_pointer);

		/* the low 3 bits determine the command */
		switch (objdata[1] & 7)
		{
			/* bitmap object */
			case 0:
				LOGMASKED(LOG_OBJECTS, "%08x: bitmap = %08X-%08X %08X-%08X\n", object_pointer, objdata[0], objdata[1], objdata[2], objdata[3]);
				object_pointer = process_bitmap(scanline, objdata, vc);
				break;

			/* scaled bitmap object */
			case 1:
				LOGMASKED(LOG_OBJECTS, "%08x: scaled = %08X-%08X %08X-%08X %08X-%08X\n", object_pointer, objdata[0], objdata[1], objdata[2], objdata[3], objdata[4], objdata[5]);
				object_pointer = process_scaled_bitmap(scanline, objdata, vc);
				break;


			/* GPU interrupt */
			case 2:
			{
				// mutntpng, atarikrt YPOS = 0
				// kasumi YPOS = 0x7ff
				// valdiser variable, depends on raster split
				// defender YPOS = 0 (2k), YPOS=2047 (classic & plus)
				// TODO: is YPOS really used?
				uint16_t ypos = (objdata[1] >> 3) & 0x7ff;

				LOGMASKED(LOG_OBJECTS, "%08x: GPU irq = %08X-%08X (YPOS=%d)\n", object_pointer, objdata[0], objdata[1], ypos);

				// kasumi wants the format to be like this (cfr. GPU lv3 irq service, with the rorq $10)
				// Object processor seems to run with swapped endianness
				m_gpu_regs[OB_HL] = (objdata[1] & 0xffff0000) >> 16;
				m_gpu_regs[OB_HH] = objdata[1] & 0xffff;
				m_gpu_regs[OB_LL] = (objdata[0] & 0xffff0000) >> 16;
				m_gpu_regs[OB_LH] = objdata[0] & 0xffff;
				// TODO: trigger timing
				gpu_suspend = true;
				m_gpu->set_input_line(3, ASSERT_LINE);
				done = 1;

				break;
			}

			/* branch */
			case 3:
				LOGMASKED(LOG_OBJECTS, "%08x: branch = %08X-%08X\n", object_pointer, objdata[0], objdata[1]);
				object_pointer = process_branch(objdata, object_pointer, vc);
				break;

			/* stop */
			case 4:
			{
				m_gpu_regs[OB_HL] = (objdata[1] & 0xffff0000) >> 16;
				m_gpu_regs[OB_HH] = objdata[1] & 0xffff;
				m_gpu_regs[OB_LL] = (objdata[0] & 0xffff0000) >> 16;
				m_gpu_regs[OB_LH] = objdata[0] & 0xffff;

				int interrupt = (objdata[1] >> 3) & 1;
				done = 1;

				LOGMASKED(LOG_OBJECTS, "%08x: stop = %08X-%08X (int=%d & %d)\n", object_pointer, objdata[0], objdata[1], interrupt, BIT(m_gpu_regs[INT1], 2));
				if (interrupt)
				{
					// fball95 and zoop depends on this irq being masked (inside fn)
					// TODO: trigger timing
					trigger_host_cpu_irq(2);
				}
				break;
			}

			case 5:
				// bretth: FF000020 0000FEE5
				LOGMASKED(LOG_OBJECTS, "%08x: <illegal 5> %08X-%08X!\n", object_pointer, objdata[0], objdata[1]);
				done = 1;
				object_pointer += 8;
				break;

			case 6:
				// kasumi: F7000000 00F0311E (nop? bad align?)
				LOGMASKED(LOG_OBJECTS, "%08x: <illegal 6> %08X-%08X!\n", object_pointer, objdata[0], objdata[1]);
				done = 1;

				object_pointer += 8;
				break;

			case 7:
				// ttoonadv: F5F104DE 05E706EF
				LOGMASKED(LOG_OBJECTS, "%08x: <illegal 7> %08X-%08X!\n", object_pointer, objdata[0], objdata[1]);
				done = 1;

				object_pointer += 8;
				break;

			// shouldn't happen
			default:
				fprintf(stderr, "jagobj: undocumented/illegal %08X %08X\n", objdata[0], objdata[1]);
				//done = 1;
				object_pointer += 8;
				break;
		}
	}

	// save the current pointer in case we found a GPU irq
	// kasumi and valdiser depends on this
	if (gpu_suspend)
	{
		m_suspend_object_pointer = object_pointer + 8;
		//m_gpu_regs[OLP_H] = object_pointer >> 16;
		//m_gpu_regs[OLP_L] = object_pointer & 0xffff;
	}
}
