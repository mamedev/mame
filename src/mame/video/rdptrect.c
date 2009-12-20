#if defined(ZCOMPARE)
	#if defined(ZUPDATE)
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_c1_zc_zu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_c1_zc_zu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_c1_zc_zu_dn(TEX_RECTANGLE *rect)
		#endif
	#else
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_c1_zc_nzu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_c1_zc_nzu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_c1_zc_nzu_dn(TEX_RECTANGLE *rect)
		#endif
	#endif
#else
	#if defined(ZUPDATE)
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_c1_nzc_zu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_c1_nzc_zu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_c1_nzc_zu_dn(TEX_RECTANGLE *rect)
		#endif
	#else
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_c1_nzc_nzu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_c1_nzc_nzu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_c1_nzc_nzu_dn(TEX_RECTANGLE *rect)
		#endif
	#endif
#endif
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
#if (defined(ZCOMPARE) || defined(ZUPDATE))
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];
#endif

	int i, j;
	int x1, x2, y1, y2;
	int s, t;

	int clipx1, clipx2, clipy1, clipy2;

	UINT32 tilenum = rect->tilenum;
	TILE *tex_tile = &tile[rect->tilenum];

	x1 = (rect->xh / 4);
	x2 = (rect->xl / 4);
	y1 = (rect->yh / 4);
	y2 = (rect->yl / 4);

	if (x2<=x1)
	{
		x2 = x1 + 1;
	}
	if (y1==y2)
	{
		y2 = y1 + 1; // Needed by Goldeneye
	}

	if ((rect->xl & 3) == 3) // Needed by Mega Man 64
	{
		x2++;
	}
	if ((rect->yl & 3) == 3)
	{
		y2++;
	}

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	calculate_clamp_diffs(tilenum);

	set_shade_for_rects(); // Needed by Pilotwings 64

	if(rect->flip)
	{
		int temp = rect->t;
		rect->t = rect->s;
		rect->s = temp;
		temp = rect->dtdy;
		rect->dtdy = rect->dsdx;
		rect->dsdx= temp;
	}

	t = ((int)(rect->t)) << 5;

	for (j = y1; j < y2; j++)
	{
		if (j >= clipy1 && j < clipy2)
		{
			int fb_index = j * fb_width;
#if defined(MAGICDITHER) || defined(BAYERDITHER)
			int mline = (j & 3) << 2;
#endif
			s = ((int)(rect->s)) << 5;

			for (i = x1; i < x2; i++)
			{
				if (i >= clipx1 && i < clipx2)
				{
					COLOR c;
#if defined(ZUPDATE)
					int rendered = 0;
#endif
					int dith = 0;
					int curpixel = fb_index + i;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
#if (defined(ZCOMPARE) || defined(ZUPDATE))
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];
#endif

					curpixel_cvg = 8;

					TEXTURE_PIPELINE(&texel0_color, s >> 5, t >> 5, tex_tile);

					COLOR_COMBINER1(&c);

#if defined(MAGICDITHER)
					dith = magic_matrix[mline + ((i ^ WORD_ADDR_XOR) & 3)];
#elif defined(BAYERDITHER)
					dith = bayer_matrix[mline + ((i ^ WORD_ADDR_XOR) & 3)];
#endif

#if defined(ZCOMPARE)
					if (z_compare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)primitive_z)<<3,primitive_delta_z))
					{
#if defined(ZUPDATE)
						rendered = BLENDER1_16(fbcur, hbcur, c, dith);
#else
						BLENDER1_16(fbcur, hbcur, c, dith);
#endif
					}
#elif defined(ZUPDATE)
					rendered = BLENDER1_16(fbcur, hbcur, c, dith);
#else
					BLENDER1_16(fbcur, hbcur, c, dith);
#endif

#if defined(ZUPDATE)
					if(rendered)
					{
						z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
					}
#endif
				}

				s += (int)(rect->dsdx);
			}
		}
		t += (int)(rect->dtdy);
	}
}

#if defined(ZCOMPARE)
	#if defined(ZUPDATE)
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_c2_zc_zu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_c2_zc_zu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_c2_zc_zu_dn(TEX_RECTANGLE *rect)
		#endif
	#else
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_c2_zc_nzu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_c2_zc_nzu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_c2_zc_nzu_dn(TEX_RECTANGLE *rect)
		#endif
	#endif
#else
	#if defined(ZUPDATE)
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_c2_nzc_zu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_c2_nzc_zu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_c2_nzc_zu_dn(TEX_RECTANGLE *rect)
		#endif
	#else
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_c2_nzc_nzu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_c2_nzc_nzu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_c2_nzc_nzu_dn(TEX_RECTANGLE *rect)
		#endif
	#endif
#endif
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
#if (defined(ZCOMPARE) || defined(ZUPDATE))
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];
#endif

	int i, j;
	int x1, x2, y1, y2;
	int s, t;
	int ss, st;

	int clipx1, clipx2, clipy1, clipy2;

	UINT32 tilenum = rect->tilenum;
	UINT32 tilenum2 = 0;
	TILE *tex_tile = &tile[rect->tilenum];
	TILE *tex_tile2 = NULL;

	x1 = (rect->xh / 4);
	x2 = (rect->xl / 4);
	y1 = (rect->yh / 4);
	y2 = (rect->yl / 4);

	if (x2<=x1)
	{
		x2 = x1 + 1;
	}
	if (y1==y2)
	{
		y2 = y1 + 1; // Needed by Goldeneye
	}

	if ((rect->xl & 3) == 3) // Needed by Mega Man 64
	{
		x2++;
	}
	if ((rect->yl & 3) == 3)
	{
		y2++;
	}

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	calculate_clamp_diffs(tilenum);

	if (!other_modes.tex_lod_en)
	{
		tilenum2 = (tilenum + 1) & 7;
		tex_tile2 = &tile[tilenum2];
	}
	else
	{
		tilenum2 = (tilenum + 1) & 7;
		tex_tile2 = &tile[tilenum2];
	}


	set_shade_for_rects(); // Needed by Pilotwings 64

	if(rect->flip)
	{
		int temp = rect->t;
		rect->t = rect->s;
		rect->s = temp;
		temp = rect->dtdy;
		rect->dtdy = rect->dsdx;
		rect->dsdx= temp;
	}

	t = ((int)(rect->t)) << 5;

	for (j = y1; j < y2; j++)
	{

		if (j >= clipy1 && j < clipy2)
		{
			int fb_index = j * fb_width;
#if defined(MAGICDITHER) || defined(BAYERDITHER)
			int mline = (j & 3) << 2;
#endif
			s = (int)(rect->s) << 5;

			for (i = x1; i < x2; i++)
			{
				if (i >= clipx1 && i < clipx2)
				{
					COLOR c1, c2;
#if defined(ZUPDATE)
					int rendered=0;
#endif
					int curpixel = fb_index + i;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
#if (defined(ZCOMPARE) || defined(ZUPDATE))
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];
#endif
					int dith = 0;

					curpixel_cvg = 8;

					ss = s >> 5;
					st = t >> 5;

					TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
					TEXTURE_PIPELINE(&texel1_color, ss, st, tex_tile2);

					COLOR_COMBINER2_C0(&c1);
					COLOR_COMBINER2_C1(&c2);

#if defined(MAGICDITHER)
					dith = magic_matrix[mline + ((i ^ WORD_ADDR_XOR) & 3)];
#elif defined(BAYERDITHER)
					dith = bayer_matrix[mline + ((i ^ WORD_ADDR_XOR) & 3)];
#endif

#if defined(ZCOMPARE)
					if (z_compare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)primitive_z)<<3,primitive_delta_z))
					{
#if defined(ZUPDATE)
						rendered = BLENDER2_16(fbcur, hbcur, c1, c2, dith);
#else
						BLENDER2_16(fbcur, hbcur, c1, c2, dith);
#endif
					}
#else
#if defined(ZUPDATE)
					rendered = BLENDER2_16(fbcur, hbcur, c1, c2, dith);
#else
					BLENDER2_16(fbcur, hbcur, c1, c2, dith);
#endif
#endif

#if defined(ZUPDATE)
					if(rendered)
					{
						z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
					}
#endif
				}

				s += (rect->dsdx);
			}
		}
		t += (rect->dtdy);
	}
}

#if defined(ZCOMPARE)
	#if defined(ZUPDATE)
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_cc_zc_zu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_cc_zc_zu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_cc_zc_zu_dn(TEX_RECTANGLE *rect)
		#endif
	#else
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_cc_zc_nzu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_cc_zc_nzu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_cc_zc_nzu_dn(TEX_RECTANGLE *rect)
		#endif
	#endif
#else
	#if defined(ZUPDATE)
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_cc_nzc_zu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_cc_nzc_zu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_cc_nzc_zu_dn(TEX_RECTANGLE *rect)
		#endif
	#else
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_cc_nzc_nzu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_cc_nzc_nzu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_cc_nzc_nzu_dn(TEX_RECTANGLE *rect)
		#endif
	#endif
#endif
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];

	int i, j;
	int x1, x2, y1, y2;
	int s, t;
	int ss, st;

	int clipx1, clipx2, clipy1, clipy2;

	UINT32 tilenum = rect->tilenum;
	TILE *tex_tile = &tile[rect->tilenum];

	x1 = (rect->xh / 4);
	x2 = (rect->xl / 4);
	y1 = (rect->yh / 4);
	y2 = (rect->yl / 4);

	if (x2<=x1)
	{
		x2 = x1 + 1;
	}
	if (y1==y2)
	{
		y2 = y1 + 1; // Needed by Goldeneye
	}

	rect->dsdx /= 4;
	x2 += 1;
	y2 += 1;

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	calculate_clamp_diffs(tilenum);

	set_shade_for_rects(); // Needed by Pilotwings 64

	if(rect->flip)
	{
		int temp = rect->t;
		rect->t = rect->s;
		rect->s = temp;
		temp = rect->dtdy;
		rect->dtdy = rect->dsdx;
		rect->dsdx= temp;
	}

	t = ((int)(rect->t)) << 5;

	for (j = y1; j < y2; j++)
	{
		int fb_index = j * fb_width;
		if (j >= clipy1 && j < clipy2)
		{
			s = (int)(rect->s) << 5;

			for (i = x1; i < x2; i++)
			{
				if (i >= clipx1 && i < clipx2)
				{
					ss = s >> 5;
					st = t >> 5;

					TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);

					curpixel_cvg = 8;

					if ((texel0_color.i.a != 0)||(!other_modes.alpha_compare_en))
					{
						fb[(fb_index + i) ^ WORD_ADDR_XOR] = ((texel0_color.i.r >> 3) << 11) | ((texel0_color.i.g >> 3) << 6) | ((texel0_color.i.b >> 3) << 1)|1;
					}
				}
				s += rect->dsdx;
			}
		}
		t += rect->dtdy;
	}
}

#if defined(ZCOMPARE)
	#if defined(ZUPDATE)
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_cf_zc_zu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_cf_zc_zu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_cf_zc_zu_dn(TEX_RECTANGLE *rect)
		#endif
	#else
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_cf_zc_nzu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_cf_zc_nzu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_cf_zc_nzu_dn(TEX_RECTANGLE *rect)
		#endif
	#endif
#else
	#if defined(ZUPDATE)
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_cf_nzc_zu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_cf_nzc_zu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_cf_nzc_zu_dn(TEX_RECTANGLE *rect)
		#endif
	#else
		#if defined(MAGICDITHER)
			static void texture_rectangle_16bit_cf_nzc_nzu_dm(TEX_RECTANGLE *rect)
		#elif defined(BAYERDITHER)
			static void texture_rectangle_16bit_cf_nzc_nzu_db(TEX_RECTANGLE *rect)
		#else
			static void texture_rectangle_16bit_cf_nzc_nzu_dn(TEX_RECTANGLE *rect)
		#endif
	#endif
#endif
{
	fatalerror("texture_rectangle with FILL cycle type is not supported\n");
}
