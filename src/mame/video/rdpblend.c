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

N64BlenderT::N64BlenderT()
{
	blend1[0] = &N64BlenderT::Blend1CycleNoBlendNoACVGNoDither;
	blend1[1] = &N64BlenderT::Blend1CycleNoBlendNoACVGDither;
	blend1[2] = &N64BlenderT::Blend1CycleNoBlendACVGNoDither;
	blend1[3] = &N64BlenderT::Blend1CycleNoBlendACVGDither;
	blend1[4] = &N64BlenderT::Blend1CycleBlendNoACVGNoDither;
	blend1[5] = &N64BlenderT::Blend1CycleBlendNoACVGDither;
	blend1[6] = &N64BlenderT::Blend1CycleBlendACVGNoDither;
	blend1[7] = &N64BlenderT::Blend1CycleBlendACVGDither;

	blend2[0] = &N64BlenderT::Blend2CycleNoBlendNoACVGNoDither;
	blend2[1] = &N64BlenderT::Blend2CycleNoBlendNoACVGDither;
	blend2[2] = &N64BlenderT::Blend2CycleNoBlendACVGNoDither;
	blend2[3] = &N64BlenderT::Blend2CycleNoBlendACVGDither;
	blend2[4] = &N64BlenderT::Blend2CycleBlendNoACVGNoDither;
	blend2[5] = &N64BlenderT::Blend2CycleBlendNoACVGDither;
	blend2[6] = &N64BlenderT::Blend2CycleBlendACVGNoDither;
	blend2[7] = &N64BlenderT::Blend2CycleBlendACVGDither;

	for (int value = 0; value < 256; value++)
	{
		for (int dither = 0; dither < 8; dither++)
		{
			m_color_dither[(value << 3) | dither] = (UINT8)dither_color(value, dither);
			m_alpha_dither[(value << 3) | dither] = (UINT8)dither_alpha(value, dither);
		}
	}
}

INT32 N64BlenderT::dither_alpha(INT32 alpha, INT32 dither)
{
	return min(alpha + dither, 0xff);
}

INT32 N64BlenderT::dither_color(INT32 color, INT32 dither)
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

bool N64BlenderT::test_for_reject(rdp_span_aux *userdata, const rdp_poly_state& object)
{
	if (alpha_reject(userdata, object))
	{
		return true;
	}
	if (object.OtherModes.antialias_en ? !userdata->CurrentPixCvg : !userdata->CurrentCvgBit)
	{
		return true;
	}
	return false;
}

bool N64BlenderT::alpha_reject(rdp_span_aux *userdata, const rdp_poly_state& object)
{
	switch (object.OtherModes.alpha_dither_mode)
	{
		case 0:
		case 1:
			return false;

		case 2:
			return userdata->PixelColor.i.a < userdata->BlendColor.i.a;

		case 3:
			return userdata->PixelColor.i.a < (rand() & 0xff);

		default:
			return false;
	}
}

bool N64BlenderT::Blend1CycleNoBlendNoACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	userdata->PixelColor.i.a = m_alpha_dither[(userdata->PixelColor.i.a << 3) | adseed];
	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];
	if (test_for_reject(userdata, object))
	{
		return false;
	}
	*fr = *userdata->ColorInputs.blender1a_r[0];
	*fg = *userdata->ColorInputs.blender1a_g[0];
	*fb = *userdata->ColorInputs.blender1a_b[0];

	return true;
}

bool N64BlenderT::Blend1CycleNoBlendNoACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	userdata->PixelColor.i.a = m_alpha_dither[(userdata->PixelColor.i.a << 3) | adseed];
	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];
	if (test_for_reject(userdata, object))
	{
		return false;
	}
	*fr = m_color_dither[((*userdata->ColorInputs.blender1a_r[0] & 0xff) << 3) | dith];
	*fg = m_color_dither[((*userdata->ColorInputs.blender1a_g[0] & 0xff) << 3) | dith];
	*fb = m_color_dither[((*userdata->ColorInputs.blender1a_b[0] & 0xff) << 3) | dith];

	return true;
}

bool N64BlenderT::Blend1CycleNoBlendACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}
	*fr = *userdata->ColorInputs.blender1a_r[0];
	*fg = *userdata->ColorInputs.blender1a_g[0];
	*fb = *userdata->ColorInputs.blender1a_b[0];

	return true;
}

bool N64BlenderT::Blend1CycleNoBlendACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}
	*fr = m_color_dither[((*userdata->ColorInputs.blender1a_r[0] & 0xff) << 3) | dith];
	*fg = m_color_dither[((*userdata->ColorInputs.blender1a_g[0] & 0xff) << 3) | dith];
	*fb = m_color_dither[((*userdata->ColorInputs.blender1a_b[0] & 0xff) << 3) | dith];

	return true;
}

bool N64BlenderT::Blend1CycleBlendNoACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->PixelColor.i.a = m_alpha_dither[(userdata->PixelColor.i.a << 3) | adseed];
	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

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

bool N64BlenderT::Blend1CycleBlendNoACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->PixelColor.i.a = m_alpha_dither[(userdata->PixelColor.i.a << 3) | adseed];
	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

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

bool N64BlenderT::Blend1CycleBlendACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

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

bool N64BlenderT::Blend1CycleBlendACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

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

bool N64BlenderT::Blend2CycleNoBlendNoACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->PixelColor.i.a = m_alpha_dither[(userdata->PixelColor.i.a << 3) | adseed];
	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->BlendedPixelColor.i.r = r;
	userdata->BlendedPixelColor.i.g = g;
	userdata->BlendedPixelColor.i.b = b;
	userdata->BlendedPixelColor.i.a = userdata->PixelColor.i.a;
	*fr = *userdata->ColorInputs.blender1a_r[1];
	*fg = *userdata->ColorInputs.blender1a_g[1];
	*fb = *userdata->ColorInputs.blender1a_b[1];

	return true;
}

bool N64BlenderT::Blend2CycleNoBlendNoACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->PixelColor.i.a = m_alpha_dither[(userdata->PixelColor.i.a << 3) | adseed];
	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->BlendedPixelColor.i.r = r;
	userdata->BlendedPixelColor.i.g = g;
	userdata->BlendedPixelColor.i.b = b;
	userdata->BlendedPixelColor.i.a = userdata->PixelColor.i.a;
	*fr = m_color_dither[((*userdata->ColorInputs.blender1a_r[1] & 0xff) << 3) | dith];
	*fg = m_color_dither[((*userdata->ColorInputs.blender1a_g[1] & 0xff) << 3) | dith];
	*fb = m_color_dither[((*userdata->ColorInputs.blender1a_b[1] & 0xff) << 3) | dith];

	return true;
}

bool N64BlenderT::Blend2CycleNoBlendACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->BlendedPixelColor.i.r = r;
	userdata->BlendedPixelColor.i.g = g;
	userdata->BlendedPixelColor.i.b = b;
	userdata->BlendedPixelColor.i.a = userdata->PixelColor.i.a;
	*fr = *userdata->ColorInputs.blender1a_r[1];
	*fg = *userdata->ColorInputs.blender1a_g[1];
	*fb = *userdata->ColorInputs.blender1a_b[1];

	return true;
}

bool N64BlenderT::Blend2CycleNoBlendACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->BlendedPixelColor.i.r = r;
	userdata->BlendedPixelColor.i.g = g;
	userdata->BlendedPixelColor.i.b = b;
	userdata->BlendedPixelColor.i.a = userdata->PixelColor.i.a;
	*fr = m_color_dither[((*userdata->ColorInputs.blender1a_r[1] & 0xff) << 3) | dith];
	*fg = m_color_dither[((*userdata->ColorInputs.blender1a_g[1] & 0xff) << 3) | dith];
	*fb = m_color_dither[((*userdata->ColorInputs.blender1a_b[1] & 0xff) << 3) | dith];

	return true;
}

bool N64BlenderT::Blend2CycleBlendNoACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->PixelColor.i.a = m_alpha_dither[(userdata->PixelColor.i.a << 3) | adseed];
	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->BlendedPixelColor.i.r = r;
	userdata->BlendedPixelColor.i.g = g;
	userdata->BlendedPixelColor.i.b = b;
	userdata->BlendedPixelColor.i.a = userdata->PixelColor.i.a;

	blend_with_partial_reject(&r, &g, &b, 1, partialreject, sel1, userdata, object);

	*fr = r;
	*fg = g;
	*fb = b;

	return true;
}

bool N64BlenderT::Blend2CycleBlendNoACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->PixelColor.i.a = m_alpha_dither[(userdata->PixelColor.i.a << 3) | adseed];
	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->BlendedPixelColor.i.r = r;
	userdata->BlendedPixelColor.i.g = g;
	userdata->BlendedPixelColor.i.b = b;
	userdata->BlendedPixelColor.i.a = userdata->PixelColor.i.a;

	blend_with_partial_reject(&r, &g, &b, 1, partialreject, sel1, userdata, object);

	*fr = m_color_dither[((r & 0xff) << 3) | dith];
	*fg = m_color_dither[((g & 0xff) << 3) | dith];
	*fb = m_color_dither[((b & 0xff) << 3) | dith];

	return true;
}

bool N64BlenderT::Blend2CycleBlendACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->BlendedPixelColor.i.r = r;
	userdata->BlendedPixelColor.i.g = g;
	userdata->BlendedPixelColor.i.b = b;
	userdata->BlendedPixelColor.i.a = userdata->PixelColor.i.a;

	blend_with_partial_reject(&r, &g, &b, 1, partialreject, sel1, userdata, object);

	*fr = r;
	*fg = g;
	*fb = b;

	return true;
}

bool N64BlenderT::Blend2CycleBlendACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	userdata->ShadeColor.i.a = m_alpha_dither[(userdata->ShadeColor.i.a << 3) | adseed];

	if (test_for_reject(userdata, object))
	{
		return false;
	}

	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	blend_pipe(0, sel0, &r, &g, &b, userdata, object);

	userdata->BlendedPixelColor.i.r = r;
	userdata->BlendedPixelColor.i.g = g;
	userdata->BlendedPixelColor.i.b = b;
	userdata->BlendedPixelColor.i.a = userdata->PixelColor.i.a;

	blend_with_partial_reject(&r, &g, &b, 1, partialreject, sel1, userdata, object);

	*fr = m_color_dither[((r & 0xff) << 3) | dith];
	*fg = m_color_dither[((g & 0xff) << 3) | dith];
	*fb = m_color_dither[((b & 0xff) << 3) | dith];

	return true;
}

void N64BlenderT::blend_with_partial_reject(INT32* r, INT32* g, INT32* b, INT32 cycle, INT32 partialreject, INT32 select, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	if (partialreject && userdata->PixelColor.i.a >= 0xff)
	{
		*r = *userdata->ColorInputs.blender1a_r[cycle];
		*g = *userdata->ColorInputs.blender1a_g[cycle];
		*b = *userdata->ColorInputs.blender1a_b[cycle];
	}
	else
	{
		userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[cycle];
		blend_pipe(cycle, select, r, g, b, userdata, object);
	}
}

void N64BlenderT::blend_pipe(const int cycle, const int special, int* r_out, int* g_out, int* b_out, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	const INT32 mask = 0xff &~ (0x73 * special);
	const INT32 shift_a = 3 + userdata->ShiftA * special;
	const INT32 shift_b = 3 + userdata->ShiftB * special;
	const INT32 blend1a = (*userdata->ColorInputs.blender1b_a[cycle] >> shift_a) & mask;
	const INT32 blend2a = (*userdata->ColorInputs.blender2b_a[cycle] >> shift_b) & mask;
	const INT32 special_shift = special << 1;

	INT32 r = (((int)(*userdata->ColorInputs.blender1a_r[cycle]) * (int)(blend1a))) + (((int)(*userdata->ColorInputs.blender2a_r[cycle]) * (int)(blend2a)));
	INT32 g = (((int)(*userdata->ColorInputs.blender1a_g[cycle]) * (int)(blend1a))) + (((int)(*userdata->ColorInputs.blender2a_g[cycle]) * (int)(blend2a)));
	INT32 b = (((int)(*userdata->ColorInputs.blender1a_b[cycle]) * (int)(blend1a))) + (((int)(*userdata->ColorInputs.blender2a_b[cycle]) * (int)(blend2a)));

	r += ((int)*userdata->ColorInputs.blender2a_r[cycle]) << special_shift;
	g += ((int)*userdata->ColorInputs.blender2a_g[cycle]) << special_shift;
	b += ((int)*userdata->ColorInputs.blender2a_b[cycle]) << special_shift;

	r >>= object.OtherModes.blend_shift;
	g >>= object.OtherModes.blend_shift;
	b >>= object.OtherModes.blend_shift;

	if (!object.OtherModes.force_blend)
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

inline INT32 N64BlenderT::min(const INT32 x, const INT32 min)
{
	if (x < min)
	{
		return x;
	}
	return min;
}
