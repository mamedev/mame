// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert, ElSemi, Angelo Salese, Matthew Daniels
/********************************************************************

    Sega Model 2 3D rasterization functions

********************************************************************/
#ifndef MAME_SEGA_MODEL2RD_IPP
#define MAME_SEGA_MODEL2RD_IPP

#pragma once


// non-textured render path
template <bool Translucent>
void model2_renderer::draw_scanline_solid(int32_t scanline, const extent_t& extent, const m2_poly_extra_data& object, int threadid)
{
	model2_state *state = object.state;
	u32 *const p = &m_destmap.pix(scanline);
	u8 *const fill = &m_fillmap.pix(scanline);
	u8  *gamma_value = &state->m_gamma_table[0];

	// extract color information
	const u16 *colortable_r = &state->m_colorxlat[0x0000/2];
	const u16 *colortable_g = &state->m_colorxlat[0x4000/2];
	const u16 *colortable_b = &state->m_colorxlat[0x8000/2];
	u32  color = object.colorbase;
	u8   checker = object.checker;
	u8   luma;
	u32  tr, tg, tb;

	// if it's translucent, there's nothing to render
	if (Translucent)
		return;

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

	// build the final color
	color = rgb_t(tr, tg, tb);

	int x = extent.startx;
	int dx = checker ? 2 : 1;
	if (checker && !((x ^ scanline) & 1))
		x++;

	for ( ; x < extent.stopx; x += dx)
	{
		if (fill[x] == 0)
		{
			p[x] = color;
			fill[x] = 0xff;
		}
	}
}

constexpr u32 LERP(u32 x, u32 y, unsigned a)
{
	return (x + (((y - x) * a) >> 8)) & 0x00ff00ff;
}

template <bool Translucent>
u32 model2_renderer::fetch_bilinear_texel(const m2_poly_extra_data& object, const s32 miplevel, s32 u, s32 v)
{
	u32 tex_wrap_x = object.texwrapx;
	u32 tex_wrap_y = object.texwrapy;
	u32 tex_mirr_x = object.texmirrorx;
	u32 tex_mirr_y = object.texmirrory;
	u32 tex_width, tex_height;
	u32 tex_x, tex_y;
	u32* sheet;

	if (miplevel == -1)
	{
		// microtexture
		tex_width = 128;
		tex_height = 128;
		tex_x = object.utexx;
		tex_y = object.utexy;
		sheet = object.texsheet[1];
		u <<= 1 << object.utexminlod;
		v <<= 1 << object.utexminlod;
	}
	else
	{
		// regular texture
		tex_width = object.texwidth >> miplevel;
		tex_height = object.texheight >> miplevel;
		tex_x = ((object.texx - 2048) >> miplevel) & 2047;
		tex_y = ((object.texy - 1024) >> miplevel) & 1023;
		sheet = object.texsheet[miplevel & 1];
		u >>= miplevel;
		v >>= miplevel;
	}

	if (tex_mirr_x && (u & (tex_width << 8)))
		u = ~u;

	if (tex_mirr_y && (v & (tex_height << 8)))
		v = ~v;

	// subtract 1/2 texel
	u -= 0x80;
	v -= 0x80;

	// extract the fractions to use as blending factors
	u32 ufrac = u & 0xff;
	u32 vfrac = v & 0xff;

	// get the four texel locations and confine to texture dimensions
	u32 u0 = (u >> 8) & (tex_width - 1);
	u32 u1 = (u0 + 1) & (tex_width - 1);
	u32 v0 = (v >> 8) & (tex_height - 1);
	u32 v1 = (v0 + 1) & (tex_height - 1);

	// clamp the texture coordinates if smooth wrapping is not enabled
	if (!tex_wrap_x && u1 == 0)
	{
		if (ufrac >= 0x80)
			u0 = u1, u1++, ufrac = 0;     // left edge of texture
		else
			u1 = u0, u0--, ufrac = 0x100; // right edge of texture
	}

	if (!tex_wrap_y && v1 == 0)
	{
		if (vfrac >= 0x80)
			v0 = 0, v1++, vfrac = 0;      // top edge of texture
		else
			v1 = v0, v0--, vfrac = 0x100; // bottom edge of texture
	}

	// read the four texels from the texture sheet
	u32 tex00 = get_texel(tex_x, tex_y, u0, v0, sheet) << 4;
	u32 tex01 = get_texel(tex_x, tex_y, u1, v0, sheet) << 4;
	u32 tex10 = get_texel(tex_x, tex_y, u0, v1, sheet) << 4;
	u32 tex11 = get_texel(tex_x, tex_y, u1, v1, sheet) << 4;

	if (Translucent)
	{
		// pack the alpha components into the upper 16 bits
		if (tex00 != 0xf0) tex00 |= 0x00800000;
		if (tex01 != 0xf0) tex01 |= 0x00800000;
		if (tex10 != 0xf0) tex10 |= 0x00800000;
		if (tex11 != 0xf0) tex11 |= 0x00800000;

		// if a texel is transparent, it takes the luma value of the neighboring texel
		if (tex00 == 0x000000f0) tex00 = tex01 & 0xff;
		if (tex01 == 0x000000f0) tex01 = tex00 & 0xff;
		if (tex10 == 0x000000f0) tex10 = tex11 & 0xff;
		if (tex11 == 0x000000f0) tex11 = tex10 & 0xff;
	}

	// linearly interpolate between left and right texels
	u32 tex0x = LERP(tex00, tex01, ufrac);
	u32 tex1x = LERP(tex10, tex11, ufrac);

	if (Translucent)
	{
		if (tex0x == 0x000000f0) tex0x = tex1x & 0xff;
		if (tex1x == 0x000000f0) tex1x = tex0x & 0xff;
	}

	// calculate the final bilinear filtered texel
	return LERP(tex0x, tex1x, vfrac);
}

// mostly copied from video/voodoo_render.cpp
inline s32 ATTR_FORCE_INLINE fast_log2(float value)
{
	// return 0 for negative values; should never happen
	if (UNEXPECTED(value < 0.0F))
		return 0;

	// we only need the exponent and highest 7 bits of mantissa
	u32 ival = f2u(value) >> 16;

	// extract exponent
	s32 exp = (ival >> 7) - 127;

	// use top 7 bits of mantissa to look up fractional log2
	static u8 const s_log2_table[128] =
	{
		  0,   2,   5,   8,  11,  14,  16,  19,  22,  25,  27,  30,  33,  35,  38,  40,
		 43,  46,  48,  51,  53,  56,  58,  61,  63,  65,  68,  70,  73,  75,  77,  80,
		 82,  84,  87,  89,  91,  93,  96,  98, 100, 102, 104, 106, 109, 111, 113, 115,
		117, 119, 121, 123, 125, 127, 129, 132, 134, 136, 138, 140, 141, 143, 145, 147,
		149, 151, 153, 155, 157, 159, 161, 162, 164, 166, 168, 170, 172, 173, 175, 177,
		179, 181, 182, 184, 186, 188, 189, 191, 193, 194, 196, 198, 200, 201, 203, 205,
		206, 208, 209, 211, 213, 214, 216, 218, 219, 221, 222, 224, 225, 227, 229, 230,
		232, 233, 235, 236, 238, 239, 241, 242, 244, 245, 247, 248, 250, 251, 253, 254
	};

	// combine and return result
	return (exp << 8) | s_log2_table[ival & 127];
}

// textured render path
template <bool Translucent>
void model2_renderer::draw_scanline_tex(int32_t scanline, const extent_t &extent, const m2_poly_extra_data& object, int threadid)
{
	model2_state *state = object.state;
	u32 *const p = &m_destmap.pix(scanline);
	u8 *const fill = &m_fillmap.pix(scanline);

	/* extract color information */
	const u16 *colortable_r = &state->m_colorxlat[0x0000/2];
	const u16 *colortable_g = &state->m_colorxlat[0x4000/2];
	const u16 *colortable_b = &state->m_colorxlat[0x8000/2];
	const u8 *lumaram = &state->m_lumaram[0];
	u32  colorbase = object.colorbase;
	u32  lumabase = object.lumabase;
	u8   checker = object.checker;
	u8  *gamma_value = &state->m_gamma_table[0];
	float ooz = extent.param[0].start;
	float uoz = extent.param[1].start;
	float voz = extent.param[2].start;
	float dooz = extent.param[0].dpdx;
	float duoz = extent.param[1].dpdx;
	float dvoz = extent.param[2].dpdx;

	// calculate maximum mipmap level from texture dimensions; we go down to 2x2
	s32 max_level = 30 - count_leading_zeros_32(std::min(object.texwidth, object.texheight));

	colorbase = state->m_palram[(colorbase + 0x1000)] & 0x7fff;

	colortable_r += ((colorbase >>  0) & 0x1f) << 8;
	colortable_g += ((colorbase >>  5) & 0x1f) << 8;
	colortable_b += ((colorbase >> 10) & 0x1f) << 8;

	int x = extent.startx;
	int dx = 1;
	if (checker)
	{
		// if the first pixel is transparent, skip to the next one
		if (!((x ^ scanline) & 1))
		{
			x++;
			ooz += dooz;
			uoz += duoz;
			voz += dvoz;
		}

		// increment by 2 pixels each time, skipping every other pixel
		dx = 2;
		dooz *= 2.0F;
		duoz *= 2.0F;
		dvoz *= 2.0F;
	}

	for ( ; x < extent.stopx; x += dx, ooz += dooz, uoz += duoz, voz += dvoz)
	{
		if (fill[x] > 0)
			continue;

		float z = recip_approx(ooz);

		s32 mml = -object.texlod + fast_log2(z);    // equivalent to log2(z^2)
		s32 level = std::clamp(mml >> 7, 0, max_level);

		// we give texture coordinates 8 fractional bits
		s32 u = s32(uoz * z * 256.0F);
		s32 v = s32(voz * z * 256.0F);

		u32 t = fetch_bilinear_texel<Translucent>(object, level, u, v);

		if (mml > 0 && level < max_level)
		{
			u32 t2 = fetch_bilinear_texel<Translucent>(object, level + 1, u, v);
			s32 frac = (mml & 127) << 1;
			t = LERP(t, t2, frac);
		}
		else if (object.utex && mml < 0)
		{
			// microtexture; blend up to almost 50%
			u32 t2 = fetch_bilinear_texel<Translucent>(object, -1, u, v);
			s32 frac = std::min(-mml >> object.utexminlod, 127);
			t = LERP(t, t2, frac);
		}

		if (Translucent)
		{
			// if alpha is less than 50%, discard
			if (t < 0x00400000)
				continue;

			// remove the alpha value; no longer needed
			t &= 0xff;
		}

		// filtered texel has 8 bits of precision but translator map has 128 (7-bit) entries; need to shift right by 1
		u8 luma = u32(lumaram[lumabase + (t >> 1)]) * object.luma / 256;

		// Virtua Striker sets up a luma of 0x40 for national flags on bleachers, fix here.
		luma = std::min(luma, u8(0x3f));

		// we have the 6 bits of luma information along with 5 bits per color component
		// now build and index into the master color lookup table and extract the raw RGB values
		u32 tr = colortable_r[luma] & 0xff;
		u32 tg = colortable_g[luma] & 0xff;
		u32 tb = colortable_b[luma] & 0xff;
		tr = gamma_value[tr];
		tg = gamma_value[tg];
		tb = gamma_value[tb];

		p[x] = rgb_t(tr, tg, tb);
		fill[x] = 0xff;
	}
}

#endif // MAME_SEGA_MODEL2RD_IPP
