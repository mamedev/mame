#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

bool N64BlenderT::Blend1Cycle(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	if (!object.OtherModes.alpha_cvg_select)
	{
		DitherA(&userdata->PixelColor.i.a, adseed);
	}

	DitherA(&userdata->ShadeColor.i.a, adseed);

	if (!AlphaCompare(userdata->PixelColor.i.a, userdata, object))
	{
		return false;
	}

	if (object.OtherModes.antialias_en ? (!userdata->CurrentPixCvg) : (!userdata->CurrentCvgBit))
	{
		return false;
	}

	bool dontblend = (partialreject && userdata->PixelColor.i.a >= 0xff);
	if (!userdata->BlendEnable || dontblend)
	{
		r = *userdata->ColorInputs.blender1a_r[0];
		g = *userdata->ColorInputs.blender1a_g[0];
		b = *userdata->ColorInputs.blender1a_b[0];
	}
	else
	{
		userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
		BlendEquationCycle0(&r, &g, &b, special_bsel, userdata, object);
	}

	if (object.OtherModes.rgb_dither_sel < 3)
	{
		DitherRGB(&r, &g, &b, dith);
	}

	*fr = r;
	*fg = g;
	*fb = b;

	return true;
}

bool N64BlenderT::Blend2Cycle(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, int special_bsel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	if (!object.OtherModes.alpha_cvg_select)
	{
		DitherA(&userdata->PixelColor.i.a, adseed);
	}

	DitherA(&userdata->ShadeColor.i.a, adseed);

	if (!AlphaCompare(userdata->PixelColor.i.a, userdata, object))
	{
		return false;
	}

	if (object.OtherModes.antialias_en ? (!userdata->CurrentPixCvg) : (!userdata->CurrentCvgBit))
	{
		return false;
	}

	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];

	INT32 r, g, b;
	BlendEquationCycle0(&r, &g, &b, special_bsel0, userdata, object);

	userdata->BlendedPixelColor.i.r = r;
	userdata->BlendedPixelColor.i.g = g;
	userdata->BlendedPixelColor.i.b = b;
	userdata->BlendedPixelColor.i.a = userdata->PixelColor.i.a;

	bool dontblend = (partialreject && userdata->PixelColor.i.a >= 0xff);
	if (!userdata->BlendEnable || dontblend)
	{
		r = *userdata->ColorInputs.blender1a_r[1];
		g = *userdata->ColorInputs.blender1a_g[1];
		b = *userdata->ColorInputs.blender1a_b[1];
	}
	else
	{
		userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[1];
		BlendEquationCycle1(&r, &g, &b, special_bsel1, userdata, object);
	}

	if (object.OtherModes.rgb_dither_sel < 3)
	{
		DitherRGB(&r, &g, &b, dith);
	}

	*fr = r;
	*fg = g;
	*fb = b;

	return true;
}

void N64BlenderT::BlendEquationCycle0(int* r, int* g, int* b, int bsel_special, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	UINT8 blend1a = *userdata->ColorInputs.blender1b_a[0] >> 3;
	UINT8 blend2a = *userdata->ColorInputs.blender2b_a[0] >> 3;

	if (bsel_special)
	{
		blend1a = (blend1a >> userdata->ShiftA) & 0x1C;
		blend2a = (blend2a >> userdata->ShiftB) & 0x1C;
	}

	UINT32 sum = ((blend1a >> 2) + (blend2a >> 2) + 1) & 0xf;

	*r = (((int)(*userdata->ColorInputs.blender1a_r[0]) * (int)(blend1a))) +
		(((int)(*userdata->ColorInputs.blender2a_r[0]) * (int)(blend2a)));

	*g = (((int)(*userdata->ColorInputs.blender1a_g[0]) * (int)(blend1a))) +
		(((int)(*userdata->ColorInputs.blender2a_g[0]) * (int)(blend2a)));

	*b = (((int)(*userdata->ColorInputs.blender1a_b[0]) * (int)(blend1a))) +
		(((int)(*userdata->ColorInputs.blender2a_b[0]) * (int)(blend2a)));

	if (bsel_special)
	{
		*r += (((int)*userdata->ColorInputs.blender2a_r[0]) << 2);
		*g += (((int)*userdata->ColorInputs.blender2a_g[0]) << 2);
		*b += (((int)*userdata->ColorInputs.blender2a_b[0]) << 2);
	}
	else
	{
		*r += (int)*userdata->ColorInputs.blender2a_r[0];
		*g += (int)*userdata->ColorInputs.blender2a_g[0];
		*b += (int)*userdata->ColorInputs.blender2a_b[0];
	}

	*r >>= 2;
	*g >>= 2;
	*b >>= 2;

	if (object.OtherModes.force_blend)
	{
		*r >>= 3;
		*g >>= 3;
		*b >>= 3;
	}
	else
	{
		if (sum)
		{
			*r /= sum;
			*g /= sum;
			*b /= sum;
		}
		else
		{
			*r = *g = *b = 0xff;
		}
	}

	if (*r > 255) *r = 255;
	if (*g > 255) *g = 255;
	if (*b > 255) *b = 255;
}

void N64BlenderT::BlendEquationCycle1(INT32* r, INT32* g, INT32* b, int bsel_special, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	UINT8 blend1a = *userdata->ColorInputs.blender1b_a[1] >> 3;
	UINT8 blend2a = *userdata->ColorInputs.blender2b_a[1] >> 3;

	if (bsel_special)
	{
		blend1a = (blend1a >> userdata->ShiftA) & 0x1C;
		blend2a = (blend2a >> userdata->ShiftB) & 0x1C;
	}

	UINT32 sum = ((blend1a >> 2) + (blend2a >> 2) + 1) & 0xf;

	*r = (((int)(*userdata->ColorInputs.blender1a_r[1]) * (int)(blend1a))) +
		(((int)(*userdata->ColorInputs.blender2a_r[1]) * (int)(blend2a)));

	*g = (((int)(*userdata->ColorInputs.blender1a_g[1]) * (int)(blend1a))) +
		(((int)(*userdata->ColorInputs.blender2a_g[1]) * (int)(blend2a)));

	*b = (((int)(*userdata->ColorInputs.blender1a_b[1]) * (int)(blend1a))) +
		(((int)(*userdata->ColorInputs.blender2a_b[1]) * (int)(blend2a)));

	if (bsel_special)
	{
		*r += (((int)*userdata->ColorInputs.blender2a_r[1]) << 2);
		*g += (((int)*userdata->ColorInputs.blender2a_g[1]) << 2);
		*b += (((int)*userdata->ColorInputs.blender2a_b[1]) << 2);
	}
	else
	{
		*r += (int)*userdata->ColorInputs.blender2a_r[1];
		*g += (int)*userdata->ColorInputs.blender2a_g[1];
		*b += (int)*userdata->ColorInputs.blender2a_b[1];
	}

	*r >>= 2;
	*g >>= 2;
	*b >>= 2;

	if (object.OtherModes.force_blend)
	{
		*r >>= 3;
		*g >>= 3;
		*b >>= 3;
	}
	else
	{
		if (sum)
		{
			*r /= sum;
			*g /= sum;
			*b /= sum;
		}
		else
		{
			*r = *g = *b = 0xff;
		}
	}

	if (*r > 255) *r = 255;
	if (*g > 255) *g = 255;
	if (*b > 255) *b = 255;
}

bool N64BlenderT::AlphaCompare(UINT8 alpha, const rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 threshold;
	if (object.OtherModes.alpha_compare_en)
	{
		threshold = (object.OtherModes.dither_alpha_en) ? m_rdp->GetRandom() : userdata->BlendColor.i.a;
		if (alpha < threshold)
		{
			return false;
		}
		return true;
	}
	return true;
}

void N64BlenderT::DitherA(UINT8 *a, int dith)
{
	INT32 new_a = *a + dith;
	if(new_a & 0x100)
	{
		new_a = 0xff;
	}
	*a = (UINT8)new_a;
}

void N64BlenderT::DitherRGB(INT32 *r, INT32 *g, INT32 *b, int dith)
{
	if ((*r & 7) > dith)
	{
		*r = (*r & 0xf8) + 8;
		if (*r > 247)
		{
			*r = 255;
		}
	}
	if ((*g & 7) > dith)
	{
		*g = (*g & 0xf8) + 8;
		if (*g > 247)
		{
			*g = 255;
		}
	}
	if ((*b & 7) > dith)
	{
		*b = (*b & 0xf8) + 8;
		if (*b > 247)
		{
			*b = 255;
		}
	}
}
