// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert, ElSemi, Angelo Salese
/********************************************************************

    Sega Model 2 3D rasterization functions

********************************************************************/

#undef MODEL2_CHECKER
#undef MODEL2_TEXTURED
#undef MODEL2_TRANSLUCENT

#ifndef MODEL2_FUNC
#error "Model 2 renderer: No function defined!"
#endif

#ifndef MODEL2_FUNC_NAME
#error "Model 2 renderer: No function name defined!"
#endif

#if MODEL2_FUNC == 0
#undef MODEL2_CHECKER
#undef MODEL2_TEXTURED
#undef MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 1
#undef MODEL2_CHECKER
#undef MODEL2_TEXTURED
#define MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 2
#undef MODEL2_CHECKER
#define MODEL2_TEXTURED
#undef MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 3
#undef MODEL2_CHECKER
#define MODEL2_TEXTURED
#define MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 4
#define MODEL2_CHECKER
#undef MODEL2_TEXTURED
#undef MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 5
#define MODEL2_CHECKER
#undef MODEL2_TEXTURED
#define MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 6
#define MODEL2_CHECKER
#define MODEL2_TEXTURED
#undef MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 7
#define MODEL2_CHECKER
#define MODEL2_TEXTURED
#define MODEL2_TRANSLUCENT
#else
#error "Model 2 renderer: Invalid function selected!"
#endif

#ifndef MODEL2_TEXTURED
/* non-textured render path */
void MODEL2_FUNC_NAME(int32_t scanline, const extent_t& extent, const m2_poly_extra_data& object, int threadid)
{
#if !defined( MODEL2_TRANSLUCENT)
	model2_state *state = object.state;
	u32 *const p = &m_destmap.pix(scanline);
	u8  *gamma_value = &state->m_gamma_table[0];

	/* extract color information */
	const u16 *colortable_r = &state->m_colorxlat[0x0000/2];
	const u16 *colortable_g = &state->m_colorxlat[0x4000/2];
	const u16 *colortable_b = &state->m_colorxlat[0x8000/2];
	u32  color = object.colorbase;
	u8   luma;
	u32  tr, tg, tb;
	int     x;
#endif
	/* if it's translucent, there's nothing to render */
#if defined( MODEL2_TRANSLUCENT)
	return;
#else

	luma = object.luma >> 2;

	color = state->m_palram[(color + 0x1000)] & 0xffff;

	colortable_r += ((color >>  0) & 0x1f) << 8;
	colortable_g += ((color >>  5) & 0x1f) << 8;
	colortable_b += ((color >> 10) & 0x1f) << 8;

	/* we have the 6 bits of luma information along with 5 bits per color component */
	/* now build and index into the master color lookup table and extract the raw RGB values */

	tr = colortable_r[(luma)] & 0xff;
	tg = colortable_g[(luma)] & 0xff;
	tb = colortable_b[(luma)] & 0xff;
	tr = gamma_value[tr];
	tg = gamma_value[tg];
	tb = gamma_value[tb];

	/* build the final color */
	color = rgb_t(tr, tg, tb);

	for(x = extent.startx; x < extent.stopx; x++)
#if defined(MODEL2_CHECKER)
		if ((x^scanline) & 1) p[x] = color;
#else
		p[x] = color;
#endif
#endif
}

#else
#define MODEL2_CONCAT(x, y) x##y
#define MODEL2_XCONCAT(x, y) MODEL2_CONCAT(x, y)
#define MODEL2_BILINEAR_FUNC_NAME MODEL2_XCONCAT(MODEL2_FUNC_NAME,_BilinearSample)

u32 MODEL2_BILINEAR_FUNC_NAME(const m2_poly_extra_data& object, const u32 miplevel, const float fu, const float fv )
{
	float lodfactor[6] = { 256.0f, 128.0f, 64.0f, 32.0f, 16.0f, 8.0f };
	u32  tex_mirr_x = object.texmirrorx;
	u32  tex_mirr_y = object.texmirrory;
	u32  tex_width = object.texwidth[miplevel];
	u32  tex_height = object.texheight[miplevel];
	u32* sheet = object.texsheet[miplevel];
	u32  tex_x = object.texx[miplevel];
	u32  tex_y = object.texy[miplevel];
	u32  tex_x_mask = tex_width - 1;
	u32  tex_y_mask = tex_height - 1;
	int32_t u = fu * lodfactor[miplevel];
	int32_t v = fv * lodfactor[miplevel];
	u32  t, tex1, tex2, tex3, tex4, frac1, frac2, frac3, frac4;
#if defined(MODEL2_TRANSLUCENT)
	u32  alp, alp1, alp2, alp3, alp4;
#endif
	int u2, u2n;
	int v2, v2n;

	u2 = u >> 8;
	v2 = v >> 8;

	if (tex_mirr_x && ((u2 & tex_width) != 0)) // Only flip if even number of tilings
	{
		u2 = (u2 ^ tex_x_mask) & tex_x_mask;
		u2n = std::max(0, u2 - 1); // Ensure sample is inside texture
	}
	else
	{
		u2 &= tex_x_mask;
		u2n = std::min(u2 + 1, (int)tex_x_mask); // Ensure sample is inside texture
	}
	if (tex_mirr_y && ((v2 & tex_height) != 0)) // Only flip if even number of tilings
	{
		v2 = (v2 ^ tex_y_mask) & tex_y_mask;
		v2n = std::max(0, v2 - 1); // Ensure sample is inside texture
	}
	else
	{
		v2 &= tex_y_mask;
		v2n = std::min(v2 + 1, (int)tex_y_mask);  // Ensure sample is inside texture
	}

	frac1 = u & 0xFF;
	frac2 = 0x100 - frac1;
	frac3 = v & 0xFF;
	frac4 = 0x100 - frac3;
	tex1 = get_texel(tex_x, tex_y, u2, v2, sheet);
	tex2 = get_texel(tex_x, tex_y, u2n, v2, sheet);
	tex3 = get_texel(tex_x, tex_y, u2, v2n, sheet);
	tex4 = get_texel(tex_x, tex_y, u2n, v2n, sheet);
#if defined(MODEL2_TRANSLUCENT)
	alp1 = (tex1 + 1) >> 4;
	alp2 = (tex2 + 1) >> 4;
	alp3 = (tex3 + 1) >> 4;
	alp4 = (tex4 + 1) >> 4;
	alp = alp1 * frac2 * frac4 + alp2 * frac1 * frac4 + alp3 * frac2 * frac3 + alp4 * frac1 * frac3;
	if (alp >= 0x8000)
		return 0xFFFFFFFF;

	// Anti Alpha Highlighted Edges
	tex1 &= alp1 - 1;
	tex2 &= alp2 - 1;
	tex3 &= alp3 - 1;
	tex4 &= alp4 - 1;
	u32 maxValidTex = std::max(std::max(std::max(tex1, tex2), tex3), tex4);
	if (alp1)
		tex1 = maxValidTex;
	if (alp2)
		tex2 = maxValidTex;
	if (alp3)
		tex3 = maxValidTex;
	if (alp4)
		tex4 = maxValidTex;
#endif
	t = tex1 * frac2 * frac4 + tex2 * frac1 * frac4 + tex3 * frac2 * frac3 + tex4 * frac1 * frac3;
	return t >> 8;
}

/* textured render path */
void MODEL2_FUNC_NAME(int32_t scanline, const extent_t& extent, const m2_poly_extra_data& object, int threadid)
{
	model2_state *state = object.state;
	u32 *const p = &m_destmap.pix(scanline);

	/* extract color information */
	const u16 *colortable_r = &state->m_colorxlat[0x0000/2];
	const u16 *colortable_g = &state->m_colorxlat[0x4000/2];
	const u16 *colortable_b = &state->m_colorxlat[0x8000/2];
	const u16 *lumaram = &state->m_lumaram[0];
	u32  colorbase = object.colorbase;
	u32  lumabase = object.lumabase;
	u8  *gamma_value = &state->m_gamma_table[0];
	float ooz = extent.param[0].start;
	float uoz = extent.param[1].start;
	float voz = extent.param[2].start;
	float dooz = extent.param[0].dpdx;
	float dudxoz = extent.param[1].dpdx;
	float dvdxoz = extent.param[2].dpdx;
	float dudyoz = extent.param[1].dpdy;
	float dvdyoz = extent.param[2].dpdy;
	float norm = sqrtf( std::max(dudxoz * dudxoz + dvdxoz * dvdxoz, dudyoz * dudyoz + dvdyoz * dvdyoz) );
	int  tr, tg, tb;
	u32	t, t2;
	u8 luma;
	int     x;

	colorbase = state->m_palram[(colorbase + 0x1000)] & 0x7fff;

	colortable_r += ((colorbase >>  0) & 0x1f) << 8;
	colortable_g += ((colorbase >>  5) & 0x1f) << 8;
	colortable_b += ((colorbase >> 10) & 0x1f) << 8;

	for (x = extent.startx; x < extent.stopx; x++, uoz += dudxoz, voz += dvdxoz, ooz += dooz)
	{
#if defined(MODEL2_CHECKER)
		if (((x ^ scanline) & 1) == 0)
			continue;
#endif

		float z = recip_approx(ooz);
		float mml = log2(norm * z) - 2.0f; // No parts are squared so no need for the usual 0.5 factor
		u32 level = std::min(std::max(0, (int)mml), 4); // We need room for one more level for trilinear
		float fu = uoz * z;
		float fv = voz * z;

		t = MODEL2_BILINEAR_FUNC_NAME(object, level, fu, fv);
		if (t == 0xFFFFFFFF)
			continue;

		t2 = MODEL2_BILINEAR_FUNC_NAME(object, level + 1, fu, fv);
		if (t2 != 0xFFFFFFFF)
		{
			// Trilinear combination
			int frac = (int)((mml - level) * 256.0f);
			frac = std::min(std::max(frac, 0), 256);
			t = ((256 - frac) * t + frac * t2) >> 8;
		}

		// Trilinear combination has 8 bits of precision, and the table needs t to be shifted by 3 on the left
		luma = (u32)lumaram[lumabase + (t >> (8 - 3))] * object.luma / 256;

		// Virtua Striker sets up a luma of 0x40 for national flags on bleachers, fix here.
		luma = std::min((int)luma,0x3f);

		/* we have the 6 bits of luma information along with 5 bits per color component */
		/* now build and index into the master color lookup table and extract the raw RGB values */
		tr = colortable_r[(luma)] & 0xff;
		tg = colortable_g[(luma)] & 0xff;
		tb = colortable_b[(luma)] & 0xff;
		tr = gamma_value[tr];
		tg = gamma_value[tg];
		tb = gamma_value[tb];

		p[x] = rgb_t(tr, tg, tb);
	}
}

#endif
