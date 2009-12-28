#if defined(ALPHACOMPARE)
	#if defined(RGBDITHER1)
		#if defined(ZCOMPARE)
			#if defined(IMGREAD)
				INLINE int BLENDER1_16_IMR_ZC_DITH_AC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#else
				INLINE int BLENDER1_16_NIMR_ZC_DITH_AC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#endif
		#else
			#if defined(IMGREAD)
				INLINE int BLENDER1_16_IMR_NZC_DITH_AC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#else
				INLINE int BLENDER1_16_NIMR_NZC_DITH_AC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#endif
		#endif
	#else
		#if defined(ZCOMPARE)
			#if defined(IMGREAD)
				INLINE int BLENDER1_16_IMR_ZC_NDITH_AC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#else
				INLINE int BLENDER1_16_NIMR_ZC_NDITH_AC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#endif
		#else
			#if defined(IMGREAD)
				INLINE int BLENDER1_16_IMR_NZC_NDITH_AC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#else
				INLINE int BLENDER1_16_NIMR_NZC_NDITH_AC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#endif
		#endif
	#endif
#else
	#if defined(RGBDITHER1)
		#if defined(ZCOMPARE)
			#if defined(IMGREAD)
				INLINE int BLENDER1_16_IMR_ZC_DITH_NAC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#else
				INLINE int BLENDER1_16_NIMR_ZC_DITH_NAC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#endif
		#else
			#if defined(IMGREAD)
				INLINE int BLENDER1_16_IMR_NZC_DITH_NAC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#else
				INLINE int BLENDER1_16_NIMR_NZC_DITH_NAC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#endif
		#endif
	#else
		#if defined(ZCOMPARE)
			#if defined(IMGREAD)
				INLINE int BLENDER1_16_IMR_ZC_NDITH_NAC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#else
				INLINE int BLENDER1_16_NIMR_ZC_NDITH_NAC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#endif
		#else
			#if defined(IMGREAD)
				INLINE int BLENDER1_16_IMR_NZC_NDITH_NAC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#else
				INLINE int BLENDER1_16_NIMR_NZC_NDITH_NAC(UINT16 *fb, UINT8* hb, COLOR c, int dith)
			#endif
		#endif
	#endif
#endif
{
	int r, g, b;
	int special_bsel = 0;
	UINT16 mem = *fb;
#if defined(IMGREAD)
	UINT32 memory_cvg = ((mem & 1) << 2) + (*hb & 3);
#endif

#if defined(ALPHACOMPARE)
	// Alpha compare
	if (!alpha_compare(c.i.a))
	{
		return 0;
	}
#endif

	if (!curpixel_cvg) // New coverage is zero, so abort
	{
		return 0;
	}

	if (blender2b_a[0] == &memory_color.i.a)
	{
		special_bsel = 1;
	}

	pixel_color.c = c.c;

#if !defined(ZCOMPARE)
	curpixel_overlap = 0;
#endif

	memory_color.c = rgb16_to_32_lut[mem];

#if defined(IMGREAD)
	memory_color.i.a = (memory_cvg << 5) & 0xe0;
#else
	memory_color.i.a = 0xe0;
#endif

#if defined(ZCOMPARE)
	if (!curpixel_overlap && !other_modes.force_blend)
	{
		r = *blender1a_r[0];
		g = *blender1a_g[0];
		b = *blender1a_b[0];
	}
	else
	{
		inv_pixel_color.i.a = 0xff - *blender1b_a[0];

		BLENDER_EQUATION0(&r, &g, &b, special_bsel);
	}
#else
	if (!other_modes.force_blend)
	{
		r = *blender1a_r[0];
		g = *blender1a_g[0];
		b = *blender1a_b[0];
	}
	else
	{
		inv_pixel_color.i.a = 0xff - *blender1b_a[0];

		BLENDER_EQUATION0(&r, &g, &b, special_bsel);
	}
#endif

#if !defined(RGBDITHER1)
	// Hack to prevent "double-dithering" artifacts
	if (!(((r & 0xf8)==(memory_color.i.r&0xf8) && (g & 0xf8) == (memory_color.i.g & 0xf8) &&(b&0xf8)==(memory_color.i.b&0xf8))))
	{
		rgb_dither(&r, &g, &b, dith);
	}
#endif

    return (FBWRITE_16(fb, hb, r, g, b));
}

#if defined(ALPHACOMPARE)
	#if defined(RGBDITHER1)
		#if defined(ZCOMPARE)
			#if defined(IMGREAD)
				INLINE int BLENDER2_16_IMR_ZC_DITH_AC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#else
				INLINE int BLENDER2_16_NIMR_ZC_DITH_AC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#endif
		#else
			#if defined(IMGREAD)
				INLINE int BLENDER2_16_IMR_NZC_DITH_AC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#else
				INLINE int BLENDER2_16_NIMR_NZC_DITH_AC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#endif
		#endif
	#else
		#if defined(ZCOMPARE)
			#if defined(IMGREAD)
				INLINE int BLENDER2_16_IMR_ZC_NDITH_AC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#else
				INLINE int BLENDER2_16_NIMR_ZC_NDITH_AC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#endif
		#else
			#if defined(IMGREAD)
				INLINE int BLENDER2_16_IMR_NZC_NDITH_AC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#else
				INLINE int BLENDER2_16_NIMR_NZC_NDITH_AC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#endif
		#endif
	#endif
#else
	#if defined(RGBDITHER1)
		#if defined(ZCOMPARE)
			#if defined(IMGREAD)
				INLINE int BLENDER2_16_IMR_ZC_DITH_NAC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#else
				INLINE int BLENDER2_16_NIMR_ZC_DITH_NAC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#endif
		#else
			#if defined(IMGREAD)
				INLINE int BLENDER2_16_IMR_NZC_DITH_NAC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#else
				INLINE int BLENDER2_16_NIMR_NZC_DITH_NAC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#endif
		#endif
	#else
		#if defined(ZCOMPARE)
			#if defined(IMGREAD)
				INLINE int BLENDER2_16_IMR_ZC_NDITH_NAC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#else
				INLINE int BLENDER2_16_NIMR_ZC_NDITH_NAC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#endif
		#else
			#if defined(IMGREAD)
				INLINE int BLENDER2_16_IMR_NZC_NDITH_NAC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#else
				INLINE int BLENDER2_16_NIMR_NZC_NDITH_NAC(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
			#endif
		#endif
	#endif
#endif
{
	int r, g, b;
	int special_bsel = 0;
	UINT16 mem = *fb;
#if defined(IMGREAD)
	UINT32 memory_cvg = ((mem & 1) << 2) + (*hb & 3);
#endif

#if defined(ALPHACOMPARE)
	// Alpha compare
	if (!alpha_compare(c2.i.a))
	{
		return 0;
	}
#endif

	if (!curpixel_cvg)
	{
		return 0;
	}

	if (blender2b_a[0] == &memory_color.i.a)
	{
		special_bsel = 1;
	}

	pixel_color.c = c2.c;

#if !defined(ZCOMPARE)
	curpixel_overlap = 0;
#endif

	memory_color.c = rgb16_to_32_lut[mem];

#if defined(IMGREAD)
	memory_color.i.a = (memory_cvg << 5) & 0xe0;
#else
	memory_color.i.a = 0xe0;
#endif

	inv_pixel_color.i.a = 0xff - *blender1b_a[0];

	BLENDER_EQUATION0(&r, &g, &b, special_bsel);

	blended_pixel_color.i.r = r;
	blended_pixel_color.i.g = g;
	blended_pixel_color.i.b = b;
	blended_pixel_color.i.a = pixel_color.i.a;

	pixel_color.i.r = r;
	pixel_color.i.g = g;
	pixel_color.i.b = b;

	inv_pixel_color.i.a = 0xff - *blender1b_a[1];

	if (!curpixel_overlap && !other_modes.force_blend)
	{
		r = *blender1a_r[1];
		g = *blender1a_g[1];
		b = *blender1a_b[1];
	}
	else
	{
		if (blender2b_a[1] == &memory_color.i.a)
		{
			special_bsel = 1;
		}
		else
		{
			special_bsel = 0;
		}

		BLENDER_EQUATION1(&r, &g, &b, special_bsel);
	}

#if !defined(RGBDITH1)
	// Hack to prevent "double-dithering" artifacts
	if (!(((r & 0xf8)==(memory_color.i.r&0xf8) && (g & 0xf8) == (memory_color.i.g & 0xf8) &&(b&0xf8)==(memory_color.i.b&0xf8))))
	{
		rgb_dither(&r, &g, &b, dith);
	}
#endif

	return (FBWRITE_16(fb, hb, r, g, b));
}

#if !defined(RGBDITHER1)
#if defined(ALPHACOMPARE)
	#if defined(ZCOMPARE)
		#if defined(IMGREAD)
			INLINE int BLENDER1_16_IMR_ZC_AC_ND(UINT16 *fb, UINT8* hb, COLOR c)
		#else
			INLINE int BLENDER1_16_NIMR_ZC_AC_ND(UINT16 *fb, UINT8* hb, COLOR c)
		#endif
	#else
		#if defined(IMGREAD)
			INLINE int BLENDER1_16_IMR_NZC_AC_ND(UINT16 *fb, UINT8* hb, COLOR c)
		#else
			INLINE int BLENDER1_16_NIMR_NZC_AC_ND(UINT16 *fb, UINT8* hb, COLOR c)
		#endif
	#endif
#else
	#if defined(ZCOMPARE)
		#if defined(IMGREAD)
			INLINE int BLENDER1_16_IMR_ZC_NAC_ND(UINT16 *fb, UINT8* hb, COLOR c)
		#else
			INLINE int BLENDER1_16_NIMR_ZC_NAC_ND(UINT16 *fb, UINT8* hb, COLOR c)
		#endif
	#else
		#if defined(IMGREAD)
			INLINE int BLENDER1_16_IMR_NZC_NAC_ND(UINT16 *fb, UINT8* hb, COLOR c)
		#else
			INLINE int BLENDER1_16_NIMR_NZC_NAC_ND(UINT16 *fb, UINT8* hb, COLOR c)
		#endif
	#endif
#endif
{
	int r, g, b;
	int special_bsel = 0;
	UINT16 mem = *fb;
#if defined(IMGREAD)
	UINT32 memory_cvg = ((mem & 1) << 2) + (*hb & 3);
#endif

#if defined(ALPHACOMPARE)
	// Alpha compare
	if (!alpha_compare(c.i.a))
	{
		return 0;
	}
#endif

	if (!curpixel_cvg) // New coverage is zero, so abort
	{
		return 0;
	}

	if (blender2b_a[0] == &memory_color.i.a)
	{
		special_bsel = 1;
	}

	pixel_color.c = c.c;

#if !defined(ZCOMPARE)
	curpixel_overlap = 0;
#endif

	memory_color.c = rgb16_to_32_lut[mem];

#if defined(IMGREAD)
	memory_color.i.a = (memory_cvg << 5) & 0xe0;
#else
	memory_color.i.a = 0xe0;
#endif

#if defined(ZCOMPARE)
	if (!curpixel_overlap && !other_modes.force_blend)
	{
		r = *blender1a_r[0];
		g = *blender1a_g[0];
		b = *blender1a_b[0];
	}
	else
	{
		inv_pixel_color.i.a = 0xff - *blender1b_a[0];

		BLENDER_EQUATION0(&r, &g, &b, special_bsel);
	}
#else
	if (!other_modes.force_blend)
	{
		r = *blender1a_r[0];
		g = *blender1a_g[0];
		b = *blender1a_b[0];
	}
	else
	{
		inv_pixel_color.i.a = 0xff - *blender1b_a[0];

		BLENDER_EQUATION0(&r, &g, &b, special_bsel);
	}
#endif

    return (FBWRITE_16(fb, hb, r, g, b));
}

#if defined(ALPHACOMPARE)
	#if defined(ZCOMPARE)
		#if defined(IMGREAD)
			INLINE int BLENDER2_16_IMR_ZC_AC_ND(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2)
		#else
			INLINE int BLENDER2_16_NIMR_ZC_AC_ND(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2)
		#endif
	#else
		#if defined(IMGREAD)
			INLINE int BLENDER2_16_IMR_NZC_AC_ND(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2)
		#else
			INLINE int BLENDER2_16_NIMR_NZC_AC_ND(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2)
		#endif
	#endif
#else
	#if defined(ZCOMPARE)
		#if defined(IMGREAD)
			INLINE int BLENDER2_16_IMR_ZC_NAC_ND(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2)
		#else
			INLINE int BLENDER2_16_NIMR_ZC_NAC_ND(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2)
		#endif
	#else
		#if defined(IMGREAD)
			INLINE int BLENDER2_16_IMR_NZC_NAC_ND(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2)
		#else
			INLINE int BLENDER2_16_NIMR_NZC_NAC_ND(UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2)
		#endif
	#endif
#endif
{
	int r, g, b;
	int special_bsel = 0;
	UINT16 mem = *fb;
#if defined(IMGREAD)
	UINT32 memory_cvg = ((mem & 1) << 2) + (*hb & 3);
#endif

#if defined(ALPHACOMPARE)
	// Alpha compare
	if (!alpha_compare(c2.i.a))
	{
		return 0;
	}
#endif

	if (!curpixel_cvg)
	{
		return 0;
	}

	if (blender2b_a[0] == &memory_color.i.a)
	{
		special_bsel = 1;
	}

	pixel_color.c = c2.c;

#if !defined(ZCOMPARE)
	curpixel_overlap = 0;
#endif

	memory_color.c = rgb16_to_32_lut[mem];

#if defined(IMGREAD)
	memory_color.i.a = (memory_cvg << 5) & 0xe0;
#else
	memory_color.i.a = 0xe0;
#endif

	inv_pixel_color.i.a = 0xff - *blender1b_a[0];

	BLENDER_EQUATION0(&r, &g, &b, special_bsel);

	blended_pixel_color.i.r = r;
	blended_pixel_color.i.g = g;
	blended_pixel_color.i.b = b;
	blended_pixel_color.i.a = pixel_color.i.a;

	pixel_color.i.r = r;
	pixel_color.i.g = g;
	pixel_color.i.b = b;

	inv_pixel_color.i.a = 0xff - *blender1b_a[1];

	if (!curpixel_overlap && !other_modes.force_blend)
	{
		r = *blender1a_r[1];
		g = *blender1a_g[1];
		b = *blender1a_b[1];
	}
	else
	{
		if (blender2b_a[1] == &memory_color.i.a)
		{
			special_bsel = 1;
		}
		else
		{
			special_bsel = 0;
		}

		BLENDER_EQUATION1(&r, &g, &b, special_bsel);
	}

	return (FBWRITE_16(fb, hb, r, g, b));
}
#endif
