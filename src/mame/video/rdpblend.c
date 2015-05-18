// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    SGI/Nintendo Reality Display Processor Blend Unit (BL)
    -------------------

    by MooglyGuy
    based on initial C code by Ville Linde
    contains additional improvements from angrylion, Ziggy, Gonetz and Orkin


******************************************************************************/

#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"
#include "video/rdpbhelp.h"

n64_blender_t::n64_blender_t()
{
	blend1[0] = &n64_blender_t::cycle1_noblend_noacvg_nodither;
	blend1[1] = &n64_blender_t::cycle1_noblend_noacvg_dither;
	blend1[2] = &n64_blender_t::cycle1_noblend_acvg_nodither;
	blend1[3] = &n64_blender_t::cycle1_noblend_acvg_dither;
	blend1[4] = &n64_blender_t::cycle1_blend_noacvg_nodither;
	blend1[5] = &n64_blender_t::cycle1_blend_noacvg_dither;
	blend1[6] = &n64_blender_t::cycle1_blend_acvg_nodither;
	blend1[7] = &n64_blender_t::cycle1_blend_acvg_dither;

	blend2[0] = &n64_blender_t::cycle2_noblend_noacvg_nodither;
	blend2[1] = &n64_blender_t::cycle2_noblend_noacvg_dither;
	blend2[2] = &n64_blender_t::cycle2_noblend_acvg_nodither;
	blend2[3] = &n64_blender_t::cycle2_noblend_acvg_dither;
	blend2[4] = &n64_blender_t::cycle2_blend_noacvg_nodither;
	blend2[5] = &n64_blender_t::cycle2_blend_noacvg_dither;
	blend2[6] = &n64_blender_t::cycle2_blend_acvg_nodither;
	blend2[7] = &n64_blender_t::cycle2_blend_acvg_dither;

	for (int value = 0; value < 256; value++)
	{
		for (int dither = 0; dither < 8; dither++)
		{
			m_color_dither[(value << 3) | dither] = (UINT8)dither_color(value, dither);
			m_alpha_dither[(value << 3) | dither] = (UINT8)dither_alpha(value, dither);
		}
	}
}

INT32 n64_blender_t::dither_alpha(INT32 alpha, INT32 dither)
{
	return min(alpha + dither, 0xff);
}

INT32 n64_blender_t::dither_color(INT32 color, INT32 dither)
{
	if ((color & 7) > dither)
	{
		color = (color & 0xf8) + 8;
		if (color > 247)
		{
			color = 255;
		}
	}
	return color;
}

bool n64_blender_t::test_for_reject(rdp_span_aux* userdata, const rdp_poly_state& object)
{
	if (alpha_reject(userdata, object))
	{
		return true;
	}
	if (object.m_other_modes.antialias_en ? !userdata->m_current_pix_cvg : !userdata->m_current_cvg_bit)
	{
		return true;
	}
	return false;
}

bool n64_blender_t::alpha_reject(rdp_span_aux* userdata, const rdp_poly_state& object)
{
	switch (object.m_other_modes.alpha_dither_mode)
	{
		case 0:
		case 1:
			return false;

		case 2:
			return userdata->m_pixel_color.i.a < userdata->m_blend_color.i.a;

		case 3:
			return userdata->m_pixel_color.i.a < (rand() & 0xff);

		default:
			return false;
	}
}

bool n64_blender_t::cycle1_noblend_noacvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_pixel_color.i.a = m_alpha_dither[(userdata->m_pixel_color.i.a << 3) | adseed];
	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];
	if (test_for_reject(userdata, object))
	{
		return false;
	}
	*fr = *userdata->m_color_inputs.blender1a_r[0];
	*fg = *userdata->m_color_inputs.blender1a_g[0];
	*fb = *userdata->m_color_inputs.blender1a_b[0];

	return true;
}

bool n64_blender_t::cycle1_noblend_noacvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_pixel_color.i.a = m_alpha_dither[(userdata->m_pixel_color.i.a << 3) | adseed];
	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];
	if (test_for_reject(userdata, object))
	{
		return false;
	}
	*fr = m_color_dither[((*userdata->m_color_inputs.blender1a_r[0] & 0xff) << 3) | dith];
	*fg = m_color_dither[((*userdata->m_color_inputs.blender1a_g[0] & 0xff) << 3) | dith];
	*fb = m_color_dither[((*userdata->m_color_inputs.blender1a_b[0] & 0xff) << 3) | dith];

	return true;
}

bool n64_blender_t::cycle1_noblend_acvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}
	*fr = *userdata->m_color_inputs.blender1a_r[0];
	*fg = *userdata->m_color_inputs.blender1a_g[0];
	*fb = *userdata->m_color_inputs.blender1a_b[0];

	return true;
}

bool n64_blender_t::cycle1_noblend_acvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}
	*fr = m_color_dither[((*userdata->m_color_inputs.blender1a_r[0] & 0xff) << 3) | dith];
	*fg = m_color_dither[((*userdata->m_color_inputs.blender1a_g[0] & 0xff) << 3) | dith];
	*fb = m_color_dither[((*userdata->m_color_inputs.blender1a_b[0] & 0xff) << 3) | dith];

	return true;
}

bool n64_blender_t::cycle1_blend_noacvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->m_pixel_color.i.a = m_alpha_dither[(userdata->m_pixel_color.i.a << 3) | adseed];
	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	blend_with_partial_reject(&r, &g, &b, 0, partialreject, sel0, userdata, object);

	*fr = r;
	*fg = g;
	*fb = b;

	return true;
}

bool n64_blender_t::cycle1_blend_noacvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->m_pixel_color.i.a = m_alpha_dither[(userdata->m_pixel_color.i.a << 3) | adseed];
	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	blend_with_partial_reject(&r, &g, &b, 0, partialreject, sel0, userdata, object);

	*fr = m_color_dither[((r & 0xff) << 3) | dith];
	*fg = m_color_dither[((g & 0xff) << 3) | dith];
	*fb = m_color_dither[((b & 0xff) << 3) | dith];

	return true;
}

bool n64_blender_t::cycle1_blend_acvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	blend_with_partial_reject(&r, &g, &b, 0, partialreject, sel0, userdata, object);

	*fr = r;
	*fg = g;
	*fb = b;

	return true;
}

bool n64_blender_t::cycle1_blend_acvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	blend_with_partial_reject(&r, &g, &b, 0, partialreject, sel0, userdata, object);

	*fr = m_color_dither[((r & 0xff) << 3) | dith];
	*fg = m_color_dither[((g & 0xff) << 3) | dith];
	*fb = m_color_dither[((b & 0xff) << 3) | dith];

	return true;
}

bool n64_blender_t::cycle2_noblend_noacvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->m_pixel_color.i.a = m_alpha_dither[(userdata->m_pixel_color.i.a << 3) | adseed];
	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.i.a = 0xff - *userdata->m_color_inputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->m_blended_pixel_color.i.r = r;
	userdata->m_blended_pixel_color.i.g = g;
	userdata->m_blended_pixel_color.i.b = b;
	userdata->m_blended_pixel_color.i.a = userdata->m_pixel_color.i.a;
	*fr = *userdata->m_color_inputs.blender1a_r[1];
	*fg = *userdata->m_color_inputs.blender1a_g[1];
	*fb = *userdata->m_color_inputs.blender1a_b[1];

	return true;
}

bool n64_blender_t::cycle2_noblend_noacvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->m_pixel_color.i.a = m_alpha_dither[(userdata->m_pixel_color.i.a << 3) | adseed];
	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.i.a = 0xff - *userdata->m_color_inputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->m_blended_pixel_color.i.r = r;
	userdata->m_blended_pixel_color.i.g = g;
	userdata->m_blended_pixel_color.i.b = b;
	userdata->m_blended_pixel_color.i.a = userdata->m_pixel_color.i.a;
	*fr = m_color_dither[((*userdata->m_color_inputs.blender1a_r[1] & 0xff) << 3) | dith];
	*fg = m_color_dither[((*userdata->m_color_inputs.blender1a_g[1] & 0xff) << 3) | dith];
	*fb = m_color_dither[((*userdata->m_color_inputs.blender1a_b[1] & 0xff) << 3) | dith];

	return true;
}

bool n64_blender_t::cycle2_noblend_acvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.i.a = 0xff - *userdata->m_color_inputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->m_blended_pixel_color.i.r = r;
	userdata->m_blended_pixel_color.i.g = g;
	userdata->m_blended_pixel_color.i.b = b;
	userdata->m_blended_pixel_color.i.a = userdata->m_pixel_color.i.a;
	*fr = *userdata->m_color_inputs.blender1a_r[1];
	*fg = *userdata->m_color_inputs.blender1a_g[1];
	*fb = *userdata->m_color_inputs.blender1a_b[1];

	return true;
}

bool n64_blender_t::cycle2_noblend_acvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.i.a = 0xff - *userdata->m_color_inputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->m_blended_pixel_color.i.r = r;
	userdata->m_blended_pixel_color.i.g = g;
	userdata->m_blended_pixel_color.i.b = b;
	userdata->m_blended_pixel_color.i.a = userdata->m_pixel_color.i.a;
	*fr = m_color_dither[((*userdata->m_color_inputs.blender1a_r[1] & 0xff) << 3) | dith];
	*fg = m_color_dither[((*userdata->m_color_inputs.blender1a_g[1] & 0xff) << 3) | dith];
	*fb = m_color_dither[((*userdata->m_color_inputs.blender1a_b[1] & 0xff) << 3) | dith];

	return true;
}

bool n64_blender_t::cycle2_blend_noacvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->m_pixel_color.i.a = m_alpha_dither[(userdata->m_pixel_color.i.a << 3) | adseed];
	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.i.a = 0xff - *userdata->m_color_inputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->m_blended_pixel_color.i.r = r;
	userdata->m_blended_pixel_color.i.g = g;
	userdata->m_blended_pixel_color.i.b = b;
	userdata->m_blended_pixel_color.i.a = userdata->m_pixel_color.i.a;

	blend_with_partial_reject(&r, &g, &b, 1, partialreject, sel1, userdata, object);

	*fr = r;
	*fg = g;
	*fb = b;

	return true;
}

bool n64_blender_t::cycle2_blend_noacvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->m_pixel_color.i.a = m_alpha_dither[(userdata->m_pixel_color.i.a << 3) | adseed];
	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.i.a = 0xff - *userdata->m_color_inputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->m_blended_pixel_color.i.r = r;
	userdata->m_blended_pixel_color.i.g = g;
	userdata->m_blended_pixel_color.i.b = b;
	userdata->m_blended_pixel_color.i.a = userdata->m_pixel_color.i.a;

	blend_with_partial_reject(&r, &g, &b, 1, partialreject, sel1, userdata, object);

	*fr = m_color_dither[((r & 0xff) << 3) | dith];
	*fg = m_color_dither[((g & 0xff) << 3) | dith];
	*fb = m_color_dither[((b & 0xff) << 3) | dith];

	return true;
}

bool n64_blender_t::cycle2_blend_acvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.i.a = 0xff - *userdata->m_color_inputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->m_blended_pixel_color.i.r = r;
	userdata->m_blended_pixel_color.i.g = g;
	userdata->m_blended_pixel_color.i.b = b;
	userdata->m_blended_pixel_color.i.a = userdata->m_pixel_color.i.a;

	blend_with_partial_reject(&r, &g, &b, 1, partialreject, sel1, userdata, object);

	*fr = r;
	*fg = g;
	*fb = b;

	return true;
}

bool n64_blender_t::cycle2_blend_acvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->m_shade_color.i.a = m_alpha_dither[(userdata->m_shade_color.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.i.a = 0xff - *userdata->m_color_inputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->m_blended_pixel_color.i.r = r;
	userdata->m_blended_pixel_color.i.g = g;
	userdata->m_blended_pixel_color.i.b = b;
	userdata->m_blended_pixel_color.i.a = userdata->m_pixel_color.i.a;

	blend_with_partial_reject(&r, &g, &b, 1, partialreject, sel1, userdata, object);

	*fr = m_color_dither[((r & 0xff) << 3) | dith];
	*fg = m_color_dither[((g & 0xff) << 3) | dith];
	*fb = m_color_dither[((b & 0xff) << 3) | dith];

	return true;
}

void n64_blender_t::blend_with_partial_reject(INT32* r, INT32* g, INT32* b, INT32 cycle, INT32 partialreject, INT32 select, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	if (partialreject && userdata->m_pixel_color.i.a >= 0xff)
	{
		*r = *userdata->m_color_inputs.blender1a_r[cycle];
		*g = *userdata->m_color_inputs.blender1a_g[cycle];
		*b = *userdata->m_color_inputs.blender1a_b[cycle];
	}
	else
	{
		userdata->m_inv_pixel_color.i.a = 0xff - *userdata->m_color_inputs.blender1b_a[cycle];
		blend_pipe(cycle, select, r, g, b, userdata, object);
	}
}

void n64_blender_t::blend_pipe(const int cycle, const int special, int* r_out, int* g_out, int* b_out, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	const INT32 mask = 0xff &~ (0x73 * special);
	const INT32 shift_a = 3 + userdata->m_shift_a * special;
	const INT32 shift_b = 3 + userdata->m_shift_b * special;
	const INT32 blend1a = (*userdata->m_color_inputs.blender1b_a[cycle] >> shift_a) & mask;
	const INT32 blend2a = (*userdata->m_color_inputs.blender2b_a[cycle] >> shift_b) & mask;
	const INT32 special_shift = special << 1;

	INT32 r = (((int)(*userdata->m_color_inputs.blender1a_r[cycle]) * (int)(blend1a))) + (((int)(*userdata->m_color_inputs.blender2a_r[cycle]) * (int)(blend2a)));
	INT32 g = (((int)(*userdata->m_color_inputs.blender1a_g[cycle]) * (int)(blend1a))) + (((int)(*userdata->m_color_inputs.blender2a_g[cycle]) * (int)(blend2a)));
	INT32 b = (((int)(*userdata->m_color_inputs.blender1a_b[cycle]) * (int)(blend1a))) + (((int)(*userdata->m_color_inputs.blender2a_b[cycle]) * (int)(blend2a)));

	r += ((int)*userdata->m_color_inputs.blender2a_r[cycle]) << special_shift;
	g += ((int)*userdata->m_color_inputs.blender2a_g[cycle]) << special_shift;
	b += ((int)*userdata->m_color_inputs.blender2a_b[cycle]) << special_shift;

	r >>= object.m_other_modes.blend_shift;
	g >>= object.m_other_modes.blend_shift;
	b >>= object.m_other_modes.blend_shift;

	if (!object.m_other_modes.force_blend)
	{
		INT32 factor_sum = ((blend1a >> 2) + (blend2a >> 2) + 1) & 0xf;
		if (factor_sum)
		{
			r /= factor_sum;
			g /= factor_sum;
			b /= factor_sum;
		}
		else
		{
			r = g = b = 0xff;
		}
	}

	*r_out = min(r, 255);
	*g_out = min(g, 255);
	*b_out = min(b, 255);
}

inline INT32 n64_blender_t::min(const INT32 x, const INT32 min)
{
	if (x < min)
	{
		return x;
	}
	return min;
}
