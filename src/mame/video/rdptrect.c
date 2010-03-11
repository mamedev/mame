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

	UINT32 tilenum = rect->tilenum;
	TILE *tex_tile = &tile[rect->tilenum];

	int x1 = rect->xh;
	int x2 = rect->xl;
	int y1 = rect->yh;
	int y2 = rect->yl;

	if (x2 <= x1)
	{
		x2 = x1 + 1;
	}
	if (y1 == y2)
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

	calculate_clamp_diffs(tilenum);

	shade_color.c = 0;	// Needed by Pilotwings 64

	CACHE_TEXTURE_PARAMS(tex_tile);

	if(y1 < clip.yh)
	{
		rect->t += rect->dtdy * (clip.yh - y1);
		y1 = clip.yh;
	}
	if(y2 > clip.yl)
	{
		y2 = clip.yl;
	}
	if(x1 < clip.xh)
	{
		rect->s += rect->dsdx * (clip.xh - x1);
		x1 = clip.xh;
	}
	if(x2 > clip.xl)
	{
		x2 = clip.xl;
	}
	rect->dsdx >>= 5;
	rect->dtdy >>= 5;

	int t = ((int)(rect->t));

	if(rect->flip)
	{
		for (int j = y1; j < y2; j++)
		{
			int fb_index = j * fb_width;
#if defined(MAGICDITHER) || defined(BAYERDITHER)
			int mline = (j & 3) << 2;
#endif
			int s = ((int)(rect->s));

			for (int i = x1; i < x2; i++)
			{
				COLOR c;
#if defined(ZUPDATE)
				int rendered = 0;
#endif
#if defined(MAGICDITHER) || defined(BAYERDITHER)
				int dith = 0;
#endif

				int curpixel = fb_index + i;
				UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
				UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
#if (defined(ZCOMPARE) || defined(ZUPDATE))
				UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
				UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];
#endif
				curpixel_cvg = 8;

				texel0_color.c = TEXTURE_PIPELINE(t, s, tex_tile);

				COLOR_COMBINER1(c);

#if defined(MAGICDITHER)
				dith = magic_matrix[mline + ((i ^ WORD_ADDR_XOR) & 3)];
#elif defined(BAYERDITHER)
				dith = bayer_matrix[mline + ((i ^ WORD_ADDR_XOR) & 3)];
#endif

#if defined(ZCOMPARE)
				if (z_compare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)primitive_z)<<3,primitive_delta_z))
				{
#endif
#if defined(MAGICDITHER) || defined(BAYERDITHER)
#if defined(ZUPDATE)
					rendered = BLENDER1_16_DITH(fbcur, hbcur, c, dith);
#else
					BLENDER1_16_DITH(fbcur, hbcur, c, dith);
#endif
#else
#if defined(ZUPDATE)
					rendered = BLENDER1_16_NDITH(fbcur, hbcur, c);
#else
					BLENDER1_16_NDITH(fbcur, hbcur, c);
#endif
#endif
#if defined(ZCOMPARE)
				}
#endif

#if defined(ZUPDATE)
				if(rendered)
				{
					z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
				}
#endif


				s += (int)(rect->dsdx);
			}
			t += (int)(rect->dtdy);
		}
	}
	else
	{
		for (int j = y1; j < y2; j++)
		{
			int fb_index = j * fb_width;
#if defined(MAGICDITHER) || defined(BAYERDITHER)
			int mline = (j & 3) << 2;
#endif

			int s = ((int)(rect->s));

			for (int i = x1; i < x2; i++)
			{
				COLOR c;
#if defined(ZUPDATE)
				int rendered = 0;
#endif
#if defined(MAGICDITHER) || defined(BAYERDITHER)
				int dith = 0;
#endif
				int curpixel = fb_index + i;
				UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
				UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
#if (defined(ZCOMPARE) || defined(ZUPDATE))
				UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
				UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];
#endif
				curpixel_cvg = 8;

				texel0_color.c = TEXTURE_PIPELINE(s, t, tex_tile);

				COLOR_COMBINER1(c);

#if defined(MAGICDITHER)
				dith = magic_matrix[mline + ((i ^ WORD_ADDR_XOR) & 3)];
#elif defined(BAYERDITHER)
				dith = bayer_matrix[mline + ((i ^ WORD_ADDR_XOR) & 3)];
#endif

#if defined(ZCOMPARE)
				if (z_compare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)primitive_z)<<3,primitive_delta_z))
				{
#endif
#if defined(MAGICDITHER) || defined(BAYERDITHER)
#if defined(ZUPDATE)
					rendered = BLENDER1_16_DITH(fbcur, hbcur, c, dith);
#else
					BLENDER1_16_DITH(fbcur, hbcur, c, dith);
#endif
#else
#if defined(ZUPDATE)
					rendered = BLENDER1_16_NDITH(fbcur, hbcur, c);
#else
					BLENDER1_16_NDITH(fbcur, hbcur, c);
#endif
#endif
#if defined(ZCOMPARE)
				}
#endif

#if defined(ZUPDATE)
				if(rendered)
				{
					z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
				}
#endif


				fbcur++;
				hbcur++;
#if (defined(ZCOMPARE) || defined(ZUPDATE))
				zbcur++;
				zhbcur++;
#endif
				s += (int)(rect->dsdx);
			}
			t += (int)(rect->dtdy);
		}
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

	UINT32 tilenum = rect->tilenum;
	UINT32 tilenum2 = 0;
	TILE *tex_tile = &tile[rect->tilenum];
	TILE *tex_tile2 = NULL;

	int x1 = rect->xh;
	int x2 = rect->xl;
	int y1 = rect->yh;
	int y2 = rect->yl;

	if (x2 <= x1)
	{
		x2 = x1 + 1;
	}
	if (y1 == y2)
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

	shade_color.c = 0;	// Needed by Pilotwings 64

	if(y1 < clip.yh)
	{
		rect->t += rect->dtdy * (clip.yh - y1);
		y1 = clip.yh;
	}
	if(y2 > clip.yl)
	{
		y2 = clip.yl;
	}
	if(x1 < clip.xh)
	{
		rect->s += rect->dsdx * (clip.xh - x1);
		x1 = clip.xh;
	}
	if(x2 > clip.xl)
	{
		x2 = clip.xl;
	}
	rect->dsdx >>= 5;
	rect->dtdy >>= 5;

	int t = ((int)(rect->t));

	if(rect->flip)
	{
		for (int j = y1; j < y2; j++)
		{
			{
				int fb_index = j * fb_width;
#if defined(MAGICDITHER) || defined(BAYERDITHER)
				int mline = (j & 3) << 2;
#endif
				int s = (int)(rect->s);

				for (int i = x1; i < x2; i++)
				{
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
#if defined(MAGICDITHER) || defined(BAYERDITHER)
						int dith = 0;
#endif

						curpixel_cvg = 8;

						CACHE_TEXTURE_PARAMS(tex_tile);
						texel0_color.c = TEXTURE_PIPELINE(t, s, tex_tile);

						CACHE_TEXTURE_PARAMS(tex_tile2);
						texel1_color.c = TEXTURE_PIPELINE(t, s, tex_tile2);

						COLOR_COMBINER2_C0(c1);
						COLOR_COMBINER2_C1(c2);

#if defined(MAGICDITHER)
						dith = magic_matrix[mline + ((i ^ WORD_ADDR_XOR) & 3)];
#elif defined(BAYERDITHER)
						dith = bayer_matrix[mline + ((i ^ WORD_ADDR_XOR) & 3)];
#endif

#if defined(ZCOMPARE)
						if (z_compare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)primitive_z)<<3,primitive_delta_z))
						{
#endif
#if defined(MAGICDITHER) || defined(BAYERDITHER)
#if defined(ZUPDATE)
							rendered = BLENDER2_16_DITH(fbcur, hbcur, c1, c2, dith);
#else
							BLENDER2_16_DITH(fbcur, hbcur, c1, c2, dith);
#endif
#else
#if defined(ZUPDATE)
							rendered = BLENDER2_16_NDITH(fbcur, hbcur, c1, c2);
#else
							BLENDER2_16_NDITH(fbcur, hbcur, c1, c2);
#endif
#endif
#if defined(ZCOMPARE)
						}
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
	else
	{
		for (int j = y1; j < y2; j++)
		{
			{
				int fb_index = j * fb_width;
#if defined(MAGICDITHER) || defined(BAYERDITHER)
				int mline = (j & 3) << 2;
#endif
				int s = (int)(rect->s);

				for (int i = x1; i < x2; i++)
				{
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
#if defined(MAGICDITHER) || defined(BAYERDITHER)
						int dith = 0;
#endif

						curpixel_cvg = 8;

						CACHE_TEXTURE_PARAMS(tex_tile);
						texel0_color.c = TEXTURE_PIPELINE(s, t, tex_tile);

						CACHE_TEXTURE_PARAMS(tex_tile2);
						texel1_color.c = TEXTURE_PIPELINE(s, t, tex_tile2);

						COLOR_COMBINER2_C0(c1);
						COLOR_COMBINER2_C1(c2);

#if defined(MAGICDITHER)
						dith = magic_matrix[mline + ((i ^ WORD_ADDR_XOR) & 3)];
#elif defined(BAYERDITHER)
						dith = bayer_matrix[mline + ((i ^ WORD_ADDR_XOR) & 3)];
#endif

#if defined(ZCOMPARE)
						if (z_compare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)primitive_z)<<3,primitive_delta_z))
						{
#endif
#if defined(MAGICDITHER) || defined(BAYERDITHER)
#if defined(ZUPDATE)
							rendered = BLENDER2_16_DITH(fbcur, hbcur, c1, c2, dith);
#else
							BLENDER2_16_DITH(fbcur, hbcur, c1, c2, dith);
#endif
#else
#if defined(ZUPDATE)
							rendered = BLENDER2_16_NDITH(fbcur, hbcur, c1, c2);
#else
							BLENDER2_16_NDITH(fbcur, hbcur, c1, c2);
#endif
#endif
#if defined(ZCOMPARE)
						}
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

	UINT32 tilenum = rect->tilenum;
	TILE *tex_tile = &tile[rect->tilenum];

	int x1 = rect->xh;
	int x2 = rect->xl;
	int y1 = rect->yh;
	int y2 = rect->yl;

	if (x2 <= x1)
	{
		x2 = x1 + 1;
	}
	if (y1 == y2)
	{
		y2 = y1 + 1; // Needed by Goldeneye
	}

	rect->dsdx /= 4;
	x2 += 1;
	y2 += 1;

	calculate_clamp_diffs(tilenum);

	shade_color.c = 0;	// Needed by Pilotwings 64

	CACHE_TEXTURE_PARAMS(tex_tile);

	if(y1 < clip.yh)
	{
		rect->t += rect->dtdy * (clip.yh - y1);
		y1 = clip.yh;
	}
	if(y2 > clip.yl)
	{
		y2 = clip.yl;
	}
	if(x1 < clip.xh)
	{
		rect->s += rect->dsdx * (clip.xh - x1);
		x1 = clip.xh;
	}
	if(x2 > clip.xl)
	{
		x2 = clip.xl;
	}
	rect->dsdx >>= 5;
	rect->dtdy >>= 5;

	int t = ((int)(rect->t));

	if(rect->flip)
	{
		for (int j = y1; j < y2; j++)
		{
			int fb_index = j * fb_width;
			{
				int s = (int)(rect->s);

				for (int i = x1; i < x2; i++)
				{
					{
						texel0_color.c = TEXTURE_PIPELINE(t, s, tex_tile);

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
	else
	{
		for (int j = y1; j < y2; j++)
		{
			int fb_index = j * fb_width;
			{
				int s = (int)(rect->s);

				for (int i = x1; i < x2; i++)
				{
					texel0_color.c = TEXTURE_PIPELINE(s, t, tex_tile);

					curpixel_cvg = 8;

					if ((texel0_color.i.a != 0)||(!other_modes.alpha_compare_en))
					{
						fb[(fb_index + i) ^ WORD_ADDR_XOR] = ((texel0_color.i.r >> 3) << 11) | ((texel0_color.i.g >> 3) << 6) | ((texel0_color.i.b >> 3) << 1)|1;
					}
					s += rect->dsdx;
				}
			}
			t += rect->dtdy;
		}
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
