// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    SGI/Nintendo Reality Display Processor Blend Unit (BL)
    -------------------

    by Ryan Holtz
    based on initial C code by Ville Linde
    contains additional improvements from angrylion, Ziggy, Gonetz and Orkin


******************************************************************************/

#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

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
			return userdata->m_pixel_color.get_a() < userdata->m_blend_color.get_a();

		case 3:
			return userdata->m_pixel_color.get_a() < (rand() & 0xff);

		default:
			return false;
	}
}

bool n64_blender_t::cycle1_noblend_noacvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_pixel_color.set_a(m_alpha_dither[((UINT8)userdata->m_pixel_color.get_a() << 3) | adseed]);
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);
	if (test_for_reject(userdata, object))
	{
		return false;
	}
	blended_pixel.set(*userdata->m_color_inputs.blender1a_rgb[0]);

	return true;
}

bool n64_blender_t::cycle1_noblend_noacvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_pixel_color.set_a(m_alpha_dither[((UINT8)userdata->m_pixel_color.get_a() << 3) | adseed]);
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);
	if (test_for_reject(userdata, object))
	{
		return false;
	}

	rgbaint_t index(*userdata->m_color_inputs.blender1a_rgb[0]);
	index.shl_imm(3);
	index.or_imm(dith);
	index.and_imm(0x7ff);
	blended_pixel.set(0, m_color_dither[index.get_r32()], m_color_dither[index.get_g32()], m_color_dither[index.get_b32()]);

	return true;
}

bool n64_blender_t::cycle1_noblend_acvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}
	blended_pixel.set(*userdata->m_color_inputs.blender1a_rgb[0]);

	return true;
}

bool n64_blender_t::cycle1_noblend_acvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	rgbaint_t index(*userdata->m_color_inputs.blender1a_rgb[0]);
	index.shl_imm(3);
	index.or_imm(dith);
	index.and_imm(0x7ff);
	blended_pixel.set(0, m_color_dither[index.get_r32()], m_color_dither[index.get_g32()], m_color_dither[index.get_b32()]);

	return true;
}

bool n64_blender_t::cycle1_blend_noacvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_pixel_color.set_a(m_alpha_dither[((UINT8)userdata->m_pixel_color.get_a() << 3) | adseed]);
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	blend_with_partial_reject(blended_pixel, 0, partialreject, sel0, userdata, object);

	return true;
}

bool n64_blender_t::cycle1_blend_noacvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_pixel_color.set_a(m_alpha_dither[((UINT8)userdata->m_pixel_color.get_a() << 3) | adseed]);
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	color_t rgb;
	blend_with_partial_reject(rgb, 0, partialreject, sel0, userdata, object);

	rgb.shl_imm(3);
	rgb.or_imm(dith);
	rgb.and_imm(0x7ff);
	blended_pixel.set(0, m_color_dither[rgb.get_r32()], m_color_dither[rgb.get_g32()], m_color_dither[rgb.get_b32()]);

	return true;
}

bool n64_blender_t::cycle1_blend_acvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	blend_with_partial_reject(blended_pixel, 0, partialreject, sel0, userdata, object);

	return true;
}

bool n64_blender_t::cycle1_blend_acvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	color_t rgb;
	blend_with_partial_reject(rgb, 0, partialreject, sel0, userdata, object);

	rgb.shl_imm(3);
	rgb.or_imm(dith);
	rgb.and_imm(0x7ff);
	blended_pixel.set(0, m_color_dither[rgb.get_r32()], m_color_dither[rgb.get_g32()], m_color_dither[rgb.get_b32()]);

	return true;
}

bool n64_blender_t::cycle2_noblend_noacvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_pixel_color.set_a(m_alpha_dither[((UINT8)userdata->m_pixel_color.get_a() << 3) | adseed]);
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.set_a(0xff - userdata->m_color_inputs.blender1b_a[0]->get_a());
	blend_pipe(0, sel0, userdata->m_blended_pixel_color, userdata, object);
	userdata->m_blended_pixel_color.set_a(userdata->m_pixel_color.get_a());

	blended_pixel.set(*userdata->m_color_inputs.blender1a_rgb[1]);

	return true;
}

bool n64_blender_t::cycle2_noblend_noacvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_pixel_color.set_a(m_alpha_dither[((UINT8)userdata->m_pixel_color.get_a() << 3) | adseed]);
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.set_a(0xff - (UINT8)userdata->m_color_inputs.blender1b_a[0]->get_a());
	blend_pipe(0, sel0, userdata->m_blended_pixel_color, userdata, object);
	userdata->m_blended_pixel_color.set_a(userdata->m_pixel_color.get_a());

	rgbaint_t index(*userdata->m_color_inputs.blender1a_rgb[1]);
	index.shl_imm(3);
	index.or_imm(dith);
	index.and_imm(0x7ff);
	blended_pixel.set(0, m_color_dither[index.get_r32()], m_color_dither[index.get_g32()], m_color_dither[index.get_b32()]);

	return true;
}

bool n64_blender_t::cycle2_noblend_acvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.set_a(0xff - userdata->m_color_inputs.blender1b_a[0]->get_a());
	blend_pipe(0, sel0, userdata->m_blended_pixel_color, userdata, object);
	userdata->m_blended_pixel_color.set_a(userdata->m_pixel_color.get_a());

	blended_pixel.set(*userdata->m_color_inputs.blender1a_rgb[1]);

	return true;
}

bool n64_blender_t::cycle2_noblend_acvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.set_a(0xff - userdata->m_color_inputs.blender1b_a[0]->get_a());
	blend_pipe(0, sel0, userdata->m_blended_pixel_color, userdata, object);
	userdata->m_blended_pixel_color.set_a(userdata->m_pixel_color.get_a());

	rgbaint_t index(*userdata->m_color_inputs.blender1a_rgb[1]);
	index.shl_imm(3);
	index.or_imm(dith);
	index.and_imm(0x7ff);
	blended_pixel.set(0, m_color_dither[index.get_r32()], m_color_dither[index.get_g32()], m_color_dither[index.get_b32()]);

	return true;
}

bool n64_blender_t::cycle2_blend_noacvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_pixel_color.set_a(m_alpha_dither[((UINT8)userdata->m_pixel_color.get_a() << 3) | adseed]);
	userdata->m_shade_color.set_a(m_alpha_dither[((UINT8)userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.set_a(0xff - userdata->m_color_inputs.blender1b_a[0]->get_a());
	blend_pipe(0, sel0, userdata->m_blended_pixel_color, userdata, object);
	userdata->m_blended_pixel_color.set_a(userdata->m_pixel_color.get_a());

	blend_with_partial_reject(blended_pixel, 1, partialreject, sel1, userdata, object);

	return true;
}

bool n64_blender_t::cycle2_blend_noacvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_pixel_color.set_a(m_alpha_dither[(userdata->m_pixel_color.get_a() << 3) | adseed]);
	userdata->m_shade_color.set_a(m_alpha_dither[(userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.set_a(0xff - userdata->m_color_inputs.blender1b_a[0]->get_a());
	blend_pipe(0, sel0, userdata->m_blended_pixel_color, userdata, object);
	userdata->m_blended_pixel_color.set_a(userdata->m_pixel_color.get_a());

	color_t rgb;
	blend_with_partial_reject(rgb, 1, partialreject, sel1, userdata, object);

	rgb.shl_imm(3);
	rgb.or_imm(dith);
	rgb.and_imm(0x7ff);
	blended_pixel.set(0, m_color_dither[rgb.get_r32()], m_color_dither[rgb.get_g32()], m_color_dither[rgb.get_b32()]);

	return true;
}

bool n64_blender_t::cycle2_blend_acvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_shade_color.set_a(m_alpha_dither[(userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.set_a(0xff - userdata->m_color_inputs.blender1b_a[0]->get_a());
	blend_pipe(0, sel0, userdata->m_blended_pixel_color, userdata, object);
	userdata->m_blended_pixel_color.set_a(userdata->m_pixel_color.get_a());

	blend_with_partial_reject(blended_pixel, 1, partialreject, sel1, userdata, object);

	return true;
}

bool n64_blender_t::cycle2_blend_acvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	userdata->m_shade_color.set_a(m_alpha_dither[(userdata->m_shade_color.get_a() << 3) | adseed]);

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->m_inv_pixel_color.set_a(0xff - userdata->m_color_inputs.blender1b_a[0]->get_a());
	blend_pipe(0, sel0, userdata->m_blended_pixel_color, userdata, object);
	userdata->m_blended_pixel_color.set_a(userdata->m_pixel_color.get_a());

	color_t rgb;
	blend_with_partial_reject(rgb, 1, partialreject, sel1, userdata, object);

	rgb.shl_imm(3);
	rgb.or_imm(dith);
	rgb.and_imm(0x7ff);
	blended_pixel.set(0, m_color_dither[rgb.get_r32()], m_color_dither[rgb.get_g32()], m_color_dither[rgb.get_b32()]);

	return true;
}

void n64_blender_t::blend_with_partial_reject(color_t& out, INT32 cycle, INT32 partialreject, INT32 select, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	if (partialreject && userdata->m_pixel_color.get_a() >= 0xff)
	{
		out.set(*userdata->m_color_inputs.blender1a_rgb[cycle]);
	}
	else
	{
		userdata->m_inv_pixel_color.set_a(0xff - userdata->m_color_inputs.blender1b_a[cycle]->get_a());
		blend_pipe(cycle, select, out, userdata, object);
	}
}

void n64_blender_t::blend_pipe(const int cycle, const int special, color_t& out, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	const INT32 mask = 0xff &~ (0x73 * special);
	const INT32 shift_a = 3 + userdata->m_shift_a * special;
	const INT32 shift_b = 3 + userdata->m_shift_b * special;
	const INT32 blend1a = (userdata->m_color_inputs.blender1b_a[cycle]->get_a() >> shift_a) & mask;
	const INT32 blend2a = (userdata->m_color_inputs.blender2b_a[cycle]->get_a() >> shift_b) & mask;
	const INT32 special_shift = special << 1;

	rgbaint_t temp(*userdata->m_color_inputs.blender1a_rgb[cycle]);
	temp.mul_imm(blend1a);

	rgbaint_t secondary(*userdata->m_color_inputs.blender2a_rgb[cycle]);
	rgbaint_t other(*userdata->m_color_inputs.blender2a_rgb[cycle]);
	other.mul_imm(blend2a);

	temp.add(other);
	secondary.shl_imm(special_shift);
	temp.add(secondary);
	temp.shr_imm(object.m_other_modes.blend_shift);

	INT32 factor_sum = 0;
	if (!object.m_other_modes.force_blend)
	{
		factor_sum = ((blend1a >> 2) + (blend2a >> 2) + 1) & 0xf;
		if (factor_sum)
		{
			temp.set_r(temp.get_r32() / factor_sum);
			temp.set_g(temp.get_g32() / factor_sum);
			temp.set_b(temp.get_b32() / factor_sum);
		}
		else
		{
			temp.set(0, 0xff, 0xff, 0xff);
		}
	}

	temp.min(255);
	out.set(temp);
}

inline INT32 n64_blender_t::min(const INT32 x, const INT32 min)
{
	if (x < min)
	{
		return x;
	}
	return min;
}
