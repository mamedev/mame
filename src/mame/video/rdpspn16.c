static void render_spans_16_c1(running_machine *machine, int start, int end, TILE* tex_tile, int shade, int texture, int zbuffer, int flip)
{
	UINT16 *fb = (UINT16*)&rdram[fb_address / 4];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];

	int i, j;

	int clipx1, clipx2, clipy1, clipy2;

	UINT32 prim_tile = tex_tile->num;
	UINT32 disable_lod = 0;
	int tilenum;

	SPAN_PARAM dr = span[0].dr;
	SPAN_PARAM dg = span[0].dg;
	SPAN_PARAM db = span[0].db;
	SPAN_PARAM da = span[0].da;
	SPAN_PARAM dz = span[0].dz;
	SPAN_PARAM ds = span[0].ds;
	SPAN_PARAM dt = span[0].dt;
	SPAN_PARAM dw = span[0].dw;
	int dzpix = span[0].dzpix;
	int drinc, dginc, dbinc, dainc, dzinc, dsinc, dtinc, dwinc;
	int xinc = flip ? 1 : -1;

	calculate_clamp_diffs(tex_tile->num);

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	if (texture && !other_modes.tex_lod_en)
	{
		tilenum = prim_tile;
		tex_tile = &tile[tilenum];
	}

	if (other_modes.tex_lod_en) // Used by World Driver Championship
	{
		disable_lod = 1;
	}

	drinc = flip ? (dr.w) : -dr.w;
	dginc = flip ? (dg.w) : -dg.w;
	dbinc = flip ? (db.w) : -db.w;
	dainc = flip ? (da.w) : -da.w;
	dzinc = flip ? (dz.w) : -dz.w;
	dsinc = flip ? (ds.w) : -ds.w;
	dtinc = flip ? (dt.w) : -dt.w;
	dwinc = flip ? (dw.w) : -dw.w;

	if (start < clipy1)
	{
		start = clipy1;
	}
	if (start >= clipy2)
	{
		start = clipy2 - 1;
	}
	if (end < clipy1)
	{
		end = clipy1;
	}
	if (end >= clipy2) // Needed by 40 Winks
	{
		end = clipy2 - 1;
	}

	set_shade_for_tris(shade); // Needed by backgrounds in Top Gear Rally 1

	for (i = start; i <= end; i++)
	{
		int xstart = span[i].lx;
		int xend = span[i].rx;
		SPAN_PARAM r;
		SPAN_PARAM g;
		SPAN_PARAM b;
		SPAN_PARAM a;
		SPAN_PARAM z = span[i].z;
		SPAN_PARAM s = span[i].s;
		SPAN_PARAM t = span[i].t;
		SPAN_PARAM w = span[i].w;

		int x;

		int fb_index = fb_width * i;
		int length;

		if (shade)
		{
			r = span[i].r;
			g = span[i].g;
			b = span[i].b;
			a = span[i].a;
		}

		x = xend;

		length = flip ? (xstart - xend) : (xend - xstart); //Moogly

		for (j = 0; j <= length; j++)
		{
			int sr = 0, sg = 0, sb = 0, sa = 0;
			int ss = s.h.h;
			int st = t.h.h;
			int sw = w.h.h;
			int sz = z.w >> 13;
			int sss = 0, sst = 0;
			COLOR c1, c2;
			if (shade)
			{
				sr = r.h.h;
				sg = g.h.h;
				sb = b.h.h;
				sa = a.h.h;
			}
			c1.c = 0;
			c2.c = 0;
			if (other_modes.z_source_sel)
			{
				sz = (((UINT32)primitive_z) << 3) & 0x3ffff;
				dzpix = primitive_delta_z;
			}


			if (x >= clipx1 && x < clipx2)
			{
				int z_compare_result = 1;

				curpixel_cvg=span[i].cvg[x];

				if (curpixel_cvg > 8)
				{
					stricterror("render_spans_16: cvg of current pixel is %d", curpixel_cvg);
				}

				if (curpixel_cvg)
				{
					int curpixel = fb_index + x;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];

					if (other_modes.persp_tex_en)
					{
						tcdiv(ss, st, sw, &sss, &sst);
					}
					else // Hack for Bust-a-Move 2
					{
						sss = ss;
						sst = st;
					}

					if (shade)
					{
						if (sr > 0xff) sr = 0xff;
						if (sg > 0xff) sg = 0xff;
						if (sb > 0xff) sb = 0xff;
						if (sa > 0xff) sa = 0xff;
						if (sr < 0) sr = 0;
						if (sg < 0) sg = 0;
						if (sb < 0) sb = 0;
						if (sa < 0) sa = 0;
						shade_color.i.r = sr;
						shade_color.i.g = sg;
						shade_color.i.b = sb;
						shade_color.i.a = sa;
					}

					if (texture)
					{
						TEXTURE_PIPELINE(&texel0_color, sss, sst, tex_tile);
					}

					c1 = COLOR_COMBINER1(machine);

					if ((zbuffer || other_modes.z_source_sel) && other_modes.z_compare_en)
					{
						z_compare_result = z_compare(fbcur, hbcur, zbcur, zhbcur, sz, dzpix);
					}

					if(z_compare_result)
					{
						int rendered = 0;
						int dith = 0;
						if (!other_modes.rgb_dither_sel)
						{
							dith = magic_matrix[(((i) & 3) << 2) + ((x ^ WORD_ADDR_XOR) & 3)];
						}
						else if (other_modes.rgb_dither_sel == 1)
						{
							dith = bayer_matrix[(((i) & 3) << 2) + ((x ^ WORD_ADDR_XOR) & 3)];
						}

						rendered = BLENDER1_16(machine, fbcur, hbcur, c1, dith);

						if (other_modes.z_update_en && rendered)
						{
							z_store(zbcur, zhbcur, sz, dzpix);
						}
					}
				}
			}

			if (shade)
			{
				r.w += drinc;
				g.w += dginc;
				b.w += dbinc;
				a.w += dainc;
			}
			z.w += dzinc;
			s.w += dsinc;
			t.w += dtinc;
			w.w += dwinc;

			x += xinc;
		}
	}
}

static void render_spans_16_c2(running_machine *machine, int start, int end, TILE* tex_tile, int shade, int texture, int zbuffer, int flip)
{
	UINT16 *fb = (UINT16*)&rdram[fb_address / 4];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];

	int i, j;

	int clipx1, clipx2, clipy1, clipy2;

	UINT32 prim_tile = tex_tile->num;
	UINT32 tilenum2 = 0;
	TILE *tex_tile2 = NULL;
	int tilenum;

	int LOD = 0;
	INT32 horstep, vertstep;
	INT32 l_tile;
	UINT32 magnify = 0;
	UINT32 distant = 0;

	SPAN_PARAM dr = span[0].dr;
	SPAN_PARAM dg = span[0].dg;
	SPAN_PARAM db = span[0].db;
	SPAN_PARAM da = span[0].da;
	SPAN_PARAM dz = span[0].dz;
	SPAN_PARAM ds = span[0].ds;
	SPAN_PARAM dt = span[0].dt;
	SPAN_PARAM dw = span[0].dw;
	int dzpix = span[0].dzpix;
	int drinc, dginc, dbinc, dainc, dzinc, dsinc, dtinc, dwinc;
	int xinc = flip ? 1 : -1;

	int nexts, nextt, nextsw;
	int lodclamp = 0;

	calculate_clamp_diffs(tex_tile->num);

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	if (texture && !other_modes.tex_lod_en)
	{
		tilenum = prim_tile;
		tex_tile = &tile[tilenum];
		tilenum2 = (prim_tile + 1) & 7;
		tex_tile2 = &tile[tilenum2];
	}

	drinc = flip ? (dr.w) : -dr.w;
	dginc = flip ? (dg.w) : -dg.w;
	dbinc = flip ? (db.w) : -db.w;
	dainc = flip ? (da.w) : -da.w;
	dzinc = flip ? (dz.w) : -dz.w;
	dsinc = flip ? (ds.w) : -ds.w;
	dtinc = flip ? (dt.w) : -dt.w;
	dwinc = flip ? (dw.w) : -dw.w;

	if (start < clipy1)
	{
		start = clipy1;
	}
	if (start >= clipy2)
	{
		start = clipy2 - 1;
	}
	if (end < clipy1)
	{
		end = clipy1;
	}
	if (end >= clipy2) // Needed by 40 Winks
	{
		end = clipy2 - 1;
	}

	set_shade_for_tris(shade); // Needed by backgrounds in Top Gear Rally 1

	for (i = start; i <= end; i++)
	{
		int xstart = span[i].lx;
		int xend = span[i].rx;
		SPAN_PARAM r;
		SPAN_PARAM g;
		SPAN_PARAM b;
		SPAN_PARAM a;
		SPAN_PARAM z = span[i].z;
		SPAN_PARAM s = span[i].s;
		SPAN_PARAM t = span[i].t;
		SPAN_PARAM w = span[i].w;

		int x;

		int fb_index = fb_width * i;
		int length;

		if (shade)
		{
			r = span[i].r;
			g = span[i].g;
			b = span[i].b;
			a = span[i].a;
		}

		x = xend;

		length = flip ? (xstart - xend) : (xend - xstart); //Moogly

		for (j = 0; j <= length; j++)
		{
			int sr = 0, sg = 0, sb = 0, sa = 0;
			int ss = s.h.h;
			int st = t.h.h;
			int sw = w.h.h;
			int sz = z.w >> 13;
			int sss = 0, sst = 0;
			COLOR c1, c2;
			if (shade)
			{
				sr = r.h.h;
				sg = g.h.h;
				sb = b.h.h;
				sa = a.h.h;
			}
			c1.c = 0;
			c2.c = 0;
			if (other_modes.z_source_sel)
			{
				sz = (((UINT32)primitive_z) << 3) & 0x3ffff;
				dzpix = primitive_delta_z;
			}


			if (x >= clipx1 && x < clipx2)
			{
				int z_compare_result = 1;

				curpixel_cvg=span[i].cvg[x];

				if (curpixel_cvg > 8)
				{
					stricterror("render_spans_16: cvg of current pixel is %d", curpixel_cvg);
				}

				if (curpixel_cvg)
				{
					int curpixel = fb_index + x;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];

					if (other_modes.persp_tex_en)
					{
						tcdiv(ss, st, sw, &sss, &sst);
					}
					else // Hack for Bust-a-Move 2
					{
						sss = ss;
						sst = st;
					}

					if (other_modes.tex_lod_en)
					{
						if (other_modes.persp_tex_en)
						{
							nextsw = (w.w + dwinc) >> 16;
							nexts = (s.w + dsinc) >> 16;
							nextt = (t.w + dtinc) >> 16;
							tcdiv(nexts, nextt, nextsw, &nexts, &nextt);
						}
						else
						{
							nexts = (s.w + dsinc)>>16;
							nextt = (t.w + dtinc)>>16;
						}

						lodclamp = 0;

						horstep = SIGN17(nexts & 0x1ffff) - SIGN17(sss & 0x1ffff);
						vertstep = SIGN17(nextt & 0x1ffff) - SIGN17(sst & 0x1ffff);
						if (horstep & 0x20000)
						{
							horstep = ~horstep & 0x1ffff;
						}
						if (vertstep & 0x20000)
						{
							vertstep = ~vertstep & 0x1ffff;
						}
						LOD = ((horstep >= vertstep) ? horstep : vertstep);
						LOD = (LOD >= span[0].dymax) ? LOD : span[0].dymax;

						if ((LOD & 0x1c000) || lodclamp)
						{
							LOD = 0x7fff;
						}
						if (LOD < min_level)
						{
							LOD = min_level;
						}

						magnify = (LOD < 32) ? 1: 0;
						l_tile = getlog2((LOD >> 5) & 0xff);
						distant = ((LOD & 0x6000) || (l_tile >= max_level)) ? 1 : 0;

						lod_frac = ((LOD << 3) >> l_tile) & 0xff;

						if (distant)
						{
							l_tile = max_level;
						}
						if(!other_modes.sharpen_tex_en && !other_modes.detail_tex_en && magnify)
						{
							lod_frac = 0;
						}
						if(!other_modes.sharpen_tex_en && !other_modes.detail_tex_en && distant)
						{
							lod_frac = 0xff;
						}
						if(other_modes.sharpen_tex_en && magnify)
						{
							lod_frac |= 0x100;
						}

						if (!other_modes.detail_tex_en)
						{
							tilenum = (prim_tile + l_tile);
							tilenum &= 7;
							if (other_modes.sharpen_tex_en)
							{
								tilenum2 = (tilenum + 1) & 7;
							}
							else if (!distant)
							{
								tilenum2 = (tilenum + 1) & 7;
							}
							else
							{
								tilenum2 = tilenum;
							}
							tex_tile = &tile[tilenum];
							tex_tile2 = &tile[tilenum2];
						}
						else
						{
							if (!magnify)
							{
								tilenum = (prim_tile + l_tile + 1);
							}
							else
							{
								tilenum = (prim_tile + l_tile);
							}
							tilenum &= 7;

							if (!distant && !magnify)
							{
								tilenum2 = (prim_tile + l_tile + 2) & 7;
							}
							else
							{
								tilenum2 = (prim_tile + l_tile + 1) & 7;
							}
							tex_tile = &tile[tilenum];
							tex_tile2 = &tile[tilenum2];
						}
					}

					if (shade)
					{
						if (sr > 0xff) sr = 0xff;
						if (sg > 0xff) sg = 0xff;
						if (sb > 0xff) sb = 0xff;
						if (sa > 0xff) sa = 0xff;
						if (sr < 0) sr = 0;
						if (sg < 0) sg = 0;
						if (sb < 0) sb = 0;
						if (sa < 0) sa = 0;
						shade_color.i.r = sr;
						shade_color.i.g = sg;
						shade_color.i.b = sb;
						shade_color.i.a = sa;
					}

					if (texture)
					{
						TEXTURE_PIPELINE(&texel0_color, sss, sst, tex_tile);
						TEXTURE_PIPELINE(&texel1_color, sss, sst, tex_tile2);
					}

					c1 = COLOR_COMBINER2_C0(machine);
					c2 = COLOR_COMBINER2_C1(machine);

					if ((zbuffer || other_modes.z_source_sel) && other_modes.z_compare_en)
					{
						z_compare_result = z_compare(fbcur, hbcur, zbcur, zhbcur, sz, dzpix);
					}

					if(z_compare_result)
					{
						int rendered = 0;
						int dith = 0;
						if (!other_modes.rgb_dither_sel)
						{
							dith = magic_matrix[(((i) & 3) << 2) + ((x ^ WORD_ADDR_XOR) & 3)];
						}
						else if (other_modes.rgb_dither_sel == 1)
						{
							dith = bayer_matrix[(((i) & 3) << 2) + ((x ^ WORD_ADDR_XOR) & 3)];
						}

						rendered = BLENDER2_16(machine, fbcur, hbcur, c1, c2, dith);

						if (other_modes.z_update_en && rendered)
						{
							z_store(zbcur, zhbcur, sz, dzpix);
						}
					}
				}
			}

			if (shade)
			{
				r.w += drinc;
				g.w += dginc;
				b.w += dbinc;
				a.w += dainc;
			}
			z.w += dzinc;
			s.w += dsinc;
			t.w += dtinc;
			w.w += dwinc;

			x += xinc;
		}
	}
}
