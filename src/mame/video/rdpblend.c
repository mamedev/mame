#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

namespace RDP
{

bool BlenderT::Blend1Cycle(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel)
{
	INT32 r, g, b;

	if (!m_rdp->OtherModes.alpha_cvg_select)
	{
		DitherA(&m_rdp->PixelColor.i.a, adseed);
	}

	DitherA(&m_rdp->ShadeColor.i.a, adseed);

	if (!AlphaCompare(m_rdp->PixelColor.i.a))
	{
		return false;
	}

	if (m_rdp->OtherModes.antialias_en ? (!m_rdp->MiscState.CurrentPixCvg) : (!m_rdp->MiscState.CurrentCvgBit))
	{
		return false;
	}

	bool dontblend = (partialreject && m_rdp->PixelColor.i.a >= 0xff);
	if (!BlendEnable || dontblend)
	{
		r = *m_rdp->ColorInputs.blender1a_r[0];
		g = *m_rdp->ColorInputs.blender1a_g[0];
		b = *m_rdp->ColorInputs.blender1a_b[0];
	}
	else
	{
		m_rdp->InvPixelColor.i.a = 0xff - *m_rdp->ColorInputs.blender1b_a[0];
		BlendEquationCycle0(&r, &g, &b, special_bsel);
	}

	if (m_rdp->OtherModes.rgb_dither_sel < 3)
	{
		DitherRGB(&r, &g, &b, dith);
	}
	*fr = r;
	*fg = g;
	*fb = b;

	return true;
}

bool BlenderT::Blend2Cycle(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, int special_bsel1)
{
	if (!m_rdp->OtherModes.alpha_cvg_select)
	{
		DitherA(&m_rdp->PixelColor.i.a, adseed);
	}

	DitherA(&m_rdp->ShadeColor.i.a, adseed);

	if (!AlphaCompare(m_rdp->PixelColor.i.a))
	{
		return false;
	}

	if (m_rdp->OtherModes.antialias_en ? (!m_rdp->MiscState.CurrentPixCvg) : (!m_rdp->MiscState.CurrentCvgBit))
	{
		return false;
	}

	m_rdp->InvPixelColor.i.a = 0xff - *m_rdp->ColorInputs.blender1b_a[0];

	INT32 r, g, b;
	BlendEquationCycle0(&r, &g, &b, special_bsel0);

	m_rdp->BlendedPixelColor.i.r = r;
	m_rdp->BlendedPixelColor.i.g = g;
	m_rdp->BlendedPixelColor.i.b = b;
	m_rdp->BlendedPixelColor.i.a = m_rdp->PixelColor.i.a;

	bool dontblend = (partialreject && m_rdp->PixelColor.i.a >= 0xff);
	if (!BlendEnable || dontblend)
	{
		r = *m_rdp->ColorInputs.blender1a_r[1];
		g = *m_rdp->ColorInputs.blender1a_g[1];
		b = *m_rdp->ColorInputs.blender1a_b[1];
	}
	else
	{
		m_rdp->InvPixelColor.i.a = 0xff - *m_rdp->ColorInputs.blender1b_a[1];
		BlendEquationCycle1(&r, &g, &b, special_bsel1);
	}

	if (m_rdp->OtherModes.rgb_dither_sel < 3)
	{
		DitherRGB(&r, &g, &b, dith);
	}

	*fr = r;
	*fg = g;
	*fb = b;

	return true;
}

void BlenderT::BlendEquationCycle0(int* r, int* g, int* b, int bsel_special)
{
	UINT8 blend1a = *m_rdp->ColorInputs.blender1b_a[0] >> 3;
	UINT8 blend2a = *m_rdp->ColorInputs.blender2b_a[0] >> 3;

	if (bsel_special)
	{
		blend1a = (blend1a >> ShiftA) & 0x1C;
		blend2a = (blend2a >> ShiftB) & 0x1C;
	}

	UINT32 sum = ((blend1a >> 2) + (blend2a >> 2) + 1) & 0xf;

	*r = (((int)(*m_rdp->ColorInputs.blender1a_r[0]) * (int)(blend1a))) +
		(((int)(*m_rdp->ColorInputs.blender2a_r[0]) * (int)(blend2a)));

	*g = (((int)(*m_rdp->ColorInputs.blender1a_g[0]) * (int)(blend1a))) +
		(((int)(*m_rdp->ColorInputs.blender2a_g[0]) * (int)(blend2a)));

	*b = (((int)(*m_rdp->ColorInputs.blender1a_b[0]) * (int)(blend1a))) +
		(((int)(*m_rdp->ColorInputs.blender2a_b[0]) * (int)(blend2a)));

	if (bsel_special)
	{
		*r += (((int)*m_rdp->ColorInputs.blender2a_r[0]) << 2);
		*g += (((int)*m_rdp->ColorInputs.blender2a_g[0]) << 2);
		*b += (((int)*m_rdp->ColorInputs.blender2a_b[0]) << 2);
	}
	else
	{
		*r += (int)*m_rdp->ColorInputs.blender2a_r[0];
		*g += (int)*m_rdp->ColorInputs.blender2a_g[0];
		*b += (int)*m_rdp->ColorInputs.blender2a_b[0];
	}

	*r >>= 2;
	*g >>= 2;
	*b >>= 2;

	if (m_rdp->OtherModes.force_blend)
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

void BlenderT::BlendEquationCycle1(INT32* r, INT32* g, INT32* b, int bsel_special)
{
	UINT8 blend1a = *m_rdp->ColorInputs.blender1b_a[1] >> 3;
	UINT8 blend2a = *m_rdp->ColorInputs.blender2b_a[1] >> 3;

	if (bsel_special)
	{
		blend1a = (blend1a >> ShiftA) & 0x1C;
		blend2a = (blend2a >> ShiftB) & 0x1C;
	}

	UINT32 sum = ((blend1a >> 2) + (blend2a >> 2) + 1) & 0xf;

	*r = (((int)(*m_rdp->ColorInputs.blender1a_r[1]) * (int)(blend1a))) +
		(((int)(*m_rdp->ColorInputs.blender2a_r[1]) * (int)(blend2a)));

	*g = (((int)(*m_rdp->ColorInputs.blender1a_g[1]) * (int)(blend1a))) +
		(((int)(*m_rdp->ColorInputs.blender2a_g[1]) * (int)(blend2a)));

	*b = (((int)(*m_rdp->ColorInputs.blender1a_b[1]) * (int)(blend1a))) +
		(((int)(*m_rdp->ColorInputs.blender2a_b[1]) * (int)(blend2a)));

	if (bsel_special)
	{
		*r += (((int)*m_rdp->ColorInputs.blender2a_r[1]) << 2);
		*g += (((int)*m_rdp->ColorInputs.blender2a_g[1]) << 2);
		*b += (((int)*m_rdp->ColorInputs.blender2a_b[1]) << 2);
	}
	else
	{
		*r += (int)*m_rdp->ColorInputs.blender2a_r[1];
		*g += (int)*m_rdp->ColorInputs.blender2a_g[1];
		*b += (int)*m_rdp->ColorInputs.blender2a_b[1];
	}

	*r >>= 2;
	*g >>= 2;
	*b >>= 2;

	if (m_rdp->OtherModes.force_blend)
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

bool BlenderT::AlphaCompare(UINT8 alpha)
{
	INT32 threshold;
	if (m_rdp->OtherModes.alpha_compare_en)
	{
		threshold = (m_rdp->OtherModes.dither_alpha_en) ? m_rdp->GetRandom() : m_rdp->BlendColor.i.a;
		if (alpha < threshold)
		{
			return false;
		}
		return true;
	}
	return true;
}

void BlenderT::DitherA(UINT8 *a, int dith)
{
	INT32 new_a = *a + dith;
	if(new_a & 0x100)
	{
		new_a = 0xff;
	}
	*a = (UINT8)new_a;
}

void BlenderT::DitherRGB(INT32 *r, INT32 *g, INT32 *b, int dith)
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

} // namespace RDP

} // namespace N64
