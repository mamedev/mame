static void texture_rectangle_16bit_c1_nzc_nzu(running_machine *machine, TEX_RECTANGLE *rect)
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];

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

	t = ((int)(rect->t)) << 5;

	for (j = y1; j < y2; j++)
	{
		int fb_index = j * fb_width;
		if (j >= clipy1 && j < clipy2)
		{
			s = ((int)(rect->s)) << 5;

			for (i = x1; i < x2; i++)
			{
				if (i >= clipx1 && i < clipx2)
				{
					COLOR c;
					int rendered = 0;
					int dith = 0;
					int curpixel = fb_index + i;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];

					curpixel_cvg = 8;

					ss = s >> 5;
					st = t >> 5;
					if (rect->flip)
					{
						TEXTURE_PIPELINE(&texel0_color, st, ss, tex_tile);
					}
					else
					{
						TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
					}

					c = COLOR_COMBINER1(machine);

					if (!other_modes.rgb_dither_sel)
					{
						dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}
					else if (other_modes.rgb_dither_sel == 1)
					{
						dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}


					rendered = BLENDER1_16(machine, fbcur, hbcur, c, dith);

					if (other_modes.z_update_en && other_modes.z_source_sel && rendered)
					{
						z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
					}
				}

				s += (int)(rect->dsdx);
			}
		}
		t += (int)(rect->dtdy);
	}
}

static void texture_rectangle_16bit_c2_nzc_nzu(running_machine *machine, TEX_RECTANGLE *rect)
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];

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
					COLOR c1, c2;
					int rendered=0;
					int curpixel = fb_index + i;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];
					int dith = 0;

					curpixel_cvg = 8;

					ss = s >> 5;
					st = t >> 5;

					if (rect->flip)
					{
						TEXTURE_PIPELINE(&texel0_color, st, ss, tex_tile);
						TEXTURE_PIPELINE(&texel1_color, st, ss, tex_tile2);
					}
					else
					{
						TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
						TEXTURE_PIPELINE(&texel1_color, ss, st, tex_tile2);
					}
					c1 = COLOR_COMBINER2_C0(machine);
					c2 = COLOR_COMBINER2_C1(machine);

					if (!other_modes.rgb_dither_sel)
					{
						dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}
					else if (other_modes.rgb_dither_sel == 1)
					{
						dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}

					rendered = BLENDER2_16(machine, fbcur, hbcur, c1, c2, dith);

					if (other_modes.z_update_en && other_modes.z_source_sel && rendered)
					{
						z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
					}
				}

				s += (rect->dsdx);
			}
		}
		t += (rect->dtdy);
	}
}

static void texture_rectangle_16bit_cc_nzc_nzu(running_machine *machine, TEX_RECTANGLE *rect)
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

					if (rect->flip)
					{
						TEXTURE_PIPELINE(&texel0_color, st, ss, tex_tile);
					}
					else
					{
						TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
					}

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

static void texture_rectangle_16bit_cf_nzc_nzu(running_machine *machine, TEX_RECTANGLE *rect)
{
	fatalerror("texture_rectangle with FILL cycle type is not supported\n");
}

static void texture_rectangle_16bit_c1_zc_nzu(running_machine *machine, TEX_RECTANGLE *rect)
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];

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

	t = ((int)(rect->t)) << 5;

	for (j = y1; j < y2; j++)
	{
		int fb_index = j * fb_width;
		if (j >= clipy1 && j < clipy2)
		{
			s = ((int)(rect->s)) << 5;

			for (i = x1; i < x2; i++)
			{
				if (i >= clipx1 && i < clipx2)
				{
					COLOR c;
					int rendered = 0;
					int dith = 0;
					int curpixel = fb_index + i;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];

					curpixel_cvg = 8;

					ss = s >> 5;
					st = t >> 5;
					if (rect->flip)
					{
						TEXTURE_PIPELINE(&texel0_color, st, ss, tex_tile);
					}
					else
					{
						TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
					}

					c = COLOR_COMBINER1(machine);

					if (!other_modes.rgb_dither_sel)
					{
						dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}
					else if (other_modes.rgb_dither_sel == 1)
					{
						dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}

					if (z_compare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)primitive_z)<<3,primitive_delta_z))
					{
						rendered = BLENDER1_16(machine, fbcur, hbcur, c, dith);
					}

					if (other_modes.z_update_en && other_modes.z_source_sel && rendered)
					{
						z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
					}
				}

				s += (int)(rect->dsdx);
			}
		}
		t += (int)(rect->dtdy);
	}
}

static void texture_rectangle_16bit_c2_zc_nzu(running_machine *machine, TEX_RECTANGLE *rect)
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];

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
					COLOR c1, c2;
					int rendered=0;
					int curpixel = fb_index + i;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];
					int dith = 0;

					curpixel_cvg = 8;

					ss = s >> 5;
					st = t >> 5;

					if (rect->flip)
					{
						TEXTURE_PIPELINE(&texel0_color, st, ss, tex_tile);
						TEXTURE_PIPELINE(&texel1_color, st, ss, tex_tile2);
					}
					else
					{
						TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
						TEXTURE_PIPELINE(&texel1_color, ss, st, tex_tile2);
					}
					c1 = COLOR_COMBINER2_C0(machine);
					c2 = COLOR_COMBINER2_C1(machine);

					if (!other_modes.rgb_dither_sel)
					{
						dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}
					else if (other_modes.rgb_dither_sel == 1)
					{
						dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}

					if (z_compare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z))
					{
						rendered = BLENDER2_16(machine, fbcur, hbcur, c1, c2, dith);
					}

					if (other_modes.z_update_en && other_modes.z_source_sel && rendered)
					{
						z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
					}
				}

				s += (rect->dsdx);
			}
		}
		t += (rect->dtdy);
	}
}

static void texture_rectangle_16bit_cc_zc_nzu(running_machine *machine, TEX_RECTANGLE *rect)
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

					if (rect->flip)
					{
						TEXTURE_PIPELINE(&texel0_color, st, ss, tex_tile);
					}
					else
					{
						TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
					}

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

static void texture_rectangle_16bit_cf_zc_nzu(running_machine *machine, TEX_RECTANGLE *rect)
{
	fatalerror("texture_rectangle with FILL cycle type is not supported\n");
}

static void texture_rectangle_16bit_c1_nzc_zu(running_machine *machine, TEX_RECTANGLE *rect)
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];

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

	t = ((int)(rect->t)) << 5;

	for (j = y1; j < y2; j++)
	{
		int fb_index = j * fb_width;
		if (j >= clipy1 && j < clipy2)
		{
			s = ((int)(rect->s)) << 5;

			for (i = x1; i < x2; i++)
			{
				if (i >= clipx1 && i < clipx2)
				{
					COLOR c;
					int dith = 0;
					int curpixel = fb_index + i;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];

					curpixel_cvg = 8;

					ss = s >> 5;
					st = t >> 5;
					if (rect->flip)
					{
						TEXTURE_PIPELINE(&texel0_color, st, ss, tex_tile);
					}
					else
					{
						TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
					}

					c = COLOR_COMBINER1(machine);

					if (!other_modes.rgb_dither_sel)
					{
						dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}
					else if (other_modes.rgb_dither_sel == 1)
					{
						dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}

					if (BLENDER1_16(machine, fbcur, hbcur, c, dith))
					{
						z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
					}
				}

				s += (int)(rect->dsdx);
			}
		}
		t += (int)(rect->dtdy);
	}
}

static void texture_rectangle_16bit_c2_nzc_zu(running_machine *machine, TEX_RECTANGLE *rect)
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];

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
					COLOR c1, c2;
					int curpixel = fb_index + i;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];
					int dith = 0;

					curpixel_cvg = 8;

					ss = s >> 5;
					st = t >> 5;

					if (rect->flip)
					{
						TEXTURE_PIPELINE(&texel0_color, st, ss, tex_tile);
						TEXTURE_PIPELINE(&texel1_color, st, ss, tex_tile2);
					}
					else
					{
						TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
						TEXTURE_PIPELINE(&texel1_color, ss, st, tex_tile2);
					}
					c1 = COLOR_COMBINER2_C0(machine);
					c2 = COLOR_COMBINER2_C1(machine);

					if (!other_modes.rgb_dither_sel)
					{
						dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}
					else if (other_modes.rgb_dither_sel == 1)
					{
						dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}

					if (BLENDER2_16(machine, fbcur, hbcur, c1, c2, dith))
					{
						z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
					}
				}

				s += (rect->dsdx);
			}
		}
		t += (rect->dtdy);
	}
}

static void texture_rectangle_16bit_cc_nzc_zu(running_machine *machine, TEX_RECTANGLE *rect)
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

					if (rect->flip)
					{
						TEXTURE_PIPELINE(&texel0_color, st, ss, tex_tile);
					}
					else
					{
						TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
					}

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

static void texture_rectangle_16bit_cf_nzc_zu(running_machine *machine, TEX_RECTANGLE *rect)
{
	fatalerror("texture_rectangle with FILL cycle type is not supported\n");
}

static void texture_rectangle_16bit_c1_zc_zu(running_machine *machine, TEX_RECTANGLE *rect)
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];

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

	t = ((int)(rect->t)) << 5;

	for (j = y1; j < y2; j++)
	{
		int fb_index = j * fb_width;
		if (j >= clipy1 && j < clipy2)
		{
			s = ((int)(rect->s)) << 5;

			for (i = x1; i < x2; i++)
			{
				if (i >= clipx1 && i < clipx2)
				{
					COLOR c;
					int dith = 0;
					int curpixel = fb_index + i;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];

					curpixel_cvg = 8;

					ss = s >> 5;
					st = t >> 5;
					if (rect->flip)
					{
						TEXTURE_PIPELINE(&texel0_color, st, ss, tex_tile);
					}
					else
					{
						TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
					}

					c = COLOR_COMBINER1(machine);

					if (!other_modes.rgb_dither_sel)
					{
						dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}
					else if (other_modes.rgb_dither_sel == 1)
					{
						dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}

					if (z_compare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)primitive_z)<<3,primitive_delta_z))
					{
						if(BLENDER1_16(machine, fbcur, hbcur, c, dith))
						{
							z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
						}
					}

				}

				s += (int)(rect->dsdx);
			}
		}
		t += (int)(rect->dtdy);
	}
}

static void texture_rectangle_16bit_c2_zc_zu(running_machine *machine, TEX_RECTANGLE *rect)
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];

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
					COLOR c1, c2;
					int curpixel = fb_index + i;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];
					int dith = 0;

					curpixel_cvg = 8;

					ss = s >> 5;
					st = t >> 5;

					if (rect->flip)
					{
						TEXTURE_PIPELINE(&texel0_color, st, ss, tex_tile);
						TEXTURE_PIPELINE(&texel1_color, st, ss, tex_tile2);
					}
					else
					{
						TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
						TEXTURE_PIPELINE(&texel1_color, ss, st, tex_tile2);
					}
					c1 = COLOR_COMBINER2_C0(machine);
					c2 = COLOR_COMBINER2_C1(machine);

					if (!other_modes.rgb_dither_sel)
					{
						dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}
					else if (other_modes.rgb_dither_sel == 1)
					{
						dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
					}

					if (z_compare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z))
					{
						if (BLENDER2_16(machine, fbcur, hbcur, c1, c2, dith))
						{
							z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
						}
					}

				}

				s += (rect->dsdx);
			}
		}
		t += (rect->dtdy);
	}
}

static void texture_rectangle_16bit_cc_zc_zu(running_machine *machine, TEX_RECTANGLE *rect)
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

					if (rect->flip)
					{
						TEXTURE_PIPELINE(&texel0_color, st, ss, tex_tile);
					}
					else
					{
						TEXTURE_PIPELINE(&texel0_color, ss, st, tex_tile);
					}

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

static void texture_rectangle_16bit_cf_zc_zu(running_machine *machine, TEX_RECTANGLE *rect)
{
	fatalerror("texture_rectangle with FILL cycle type is not supported\n");
}

