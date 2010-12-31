#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

namespace RDP
{

bool Blender::Blend1Cycle(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel)
{
	ColorInputs* ci = m_rdp->GetColorInputs();
	INT32 r, g, b;

	if (!m_other_modes->alpha_cvg_select)
	{
		DitherA(&m_rdp->GetPixelColor()->i.a, adseed);
	}

	DitherA(&m_rdp->GetShadeColor()->i.a, adseed);

	if (!AlphaCompare(m_rdp->GetPixelColor()->i.a))
	{
		//return false;
	}

	if (m_other_modes->antialias_en ? (!m_misc_state->m_curpixel_cvg) : (!m_misc_state->m_curpixel_cvbit))
	{
		return false;
	}

	bool dontblend = (partialreject && m_rdp->GetPixelColor()->i.a >= 0xff);
	if (!m_blend_enable || dontblend)
	{
		r = *ci->blender1a_r[0];
		g = *ci->blender1a_g[0];
		b = *ci->blender1a_b[0];
	}
	else
	{
		m_rdp->GetInvPixelColor()->i.a = 0xff - *ci->blender1b_a[0];
		BlendEquationCycle0(&r, &g, &b, special_bsel);
	}

	if (m_other_modes->rgb_dither_sel < 3)
	{
		DitherRGB(&r, &g, &b, dith);
	}
	*fr = r;
	*fg = g;
	*fb = b;
	return true;
}

bool Blender::Blend2Cycle(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, int special_bsel1)
{
	ColorInputs* ci = m_rdp->GetColorInputs();
	if (!m_other_modes->alpha_cvg_select)
	{
		DitherA(&m_rdp->GetPixelColor()->i.a, adseed);
	}

	DitherA(&m_rdp->GetShadeColor()->i.a, adseed);

	if (!AlphaCompare(m_rdp->GetPixelColor()->i.a))
	{
		//return false;
	}

	if (m_other_modes->antialias_en ? (!m_misc_state->m_curpixel_cvg) : (!m_misc_state->m_curpixel_cvbit))
	{
		return false;
	}

	m_rdp->GetInvPixelColor()->i.a = 0xff - *ci->blender1b_a[0];

	INT32 r, g, b;
	BlendEquationCycle0(&r, &g, &b, special_bsel0);

	m_rdp->GetBlendedColor()->i.r = r;
	m_rdp->GetBlendedColor()->i.g = g;
	m_rdp->GetBlendedColor()->i.b = b;
	m_rdp->GetBlendedColor()->i.a = m_rdp->GetPixelColor()->i.a;

	bool dontblend = (partialreject && m_rdp->GetPixelColor()->i.a >= 0xff);
	if (!m_blend_enable || dontblend)
	{
		r = *ci->blender1a_r[1];
		g = *ci->blender1a_g[1];
		b = *ci->blender1a_b[1];
	}
	else
	{
		m_rdp->GetInvPixelColor()->i.a = 0xff - *ci->blender1b_a[1];
		BlendEquationCycle1(&r, &g, &b, special_bsel1);
	}

	if (m_other_modes->rgb_dither_sel < 3)
	{
		DitherRGB(&r, &g, &b, dith);
	}

	*fr = r;
	*fg = g;
	*fb = b;

	return true;
}

void Blender::BlendEquationCycle0(int* r, int* g, int* b, int bsel_special)
{
	ColorInputs* ci = m_rdp->GetColorInputs();
	UINT8 blend1a = *ci->blender1b_a[0] >> 3;
	UINT8 blend2a = *ci->blender2b_a[0] >> 3;

	if (bsel_special)
	{
		blend1a = (blend1a >> m_shift_a) & 0x1C;
		blend2a = (blend2a >> m_shift_b) & 0x1C;
	}

	UINT32 sum = ((blend1a >> 2) + (blend2a >> 2) + 1) & 0xf;

	*r = (((int)(*ci->blender1a_r[0]) * (int)(blend1a))) +
		(((int)(*ci->blender2a_r[0]) * (int)(blend2a)));

	*g = (((int)(*ci->blender1a_g[0]) * (int)(blend1a))) +
		(((int)(*ci->blender2a_g[0]) * (int)(blend2a)));

	*b = (((int)(*ci->blender1a_b[0]) * (int)(blend1a))) +
		(((int)(*ci->blender2a_b[0]) * (int)(blend2a)));

	if (bsel_special)
	{
		*r += (((int)*ci->blender2a_r[0]) << 2);
		*g += (((int)*ci->blender2a_g[0]) << 2);
		*b += (((int)*ci->blender2a_b[0]) << 2);
	}
	else
	{
		*r += (int)*ci->blender2a_r[0];
		*g += (int)*ci->blender2a_g[0];
		*b += (int)*ci->blender2a_b[0];
	}

	*r >>= 2;
	*g >>= 2;
	*b >>= 2;

	if (m_other_modes->force_blend)
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

void Blender::BlendEquationCycle1(INT32* r, INT32* g, INT32* b, int bsel_special)
{
	ColorInputs* ci = m_rdp->GetColorInputs();
	UINT8 blend1a = *ci->blender1b_a[1] >> 3;
	UINT8 blend2a = *ci->blender2b_a[1] >> 3;

	if (bsel_special)
	{
		blend1a = (blend1a >> m_shift_a) & 0x1C;
		blend2a = (blend2a >> m_shift_b) & 0x1C;
	}

	UINT32 sum = ((blend1a >> 2) + (blend2a >> 2) + 1) & 0xf;

	*r = (((int)(*ci->blender1a_r[1]) * (int)(blend1a))) +
		(((int)(*ci->blender2a_r[1]) * (int)(blend2a)));

	*g = (((int)(*ci->blender1a_g[1]) * (int)(blend1a))) +
		(((int)(*ci->blender2a_g[1]) * (int)(blend2a)));

	*b = (((int)(*ci->blender1a_b[1]) * (int)(blend1a))) +
		(((int)(*ci->blender2a_b[1]) * (int)(blend2a)));

	if (bsel_special)
	{
		*r += (((int)*ci->blender2a_r[1]) << 2);
		*g += (((int)*ci->blender2a_g[1]) << 2);
		*b += (((int)*ci->blender2a_b[1]) << 2);
	}
	else
	{
		*r += (int)*ci->blender2a_r[1];
		*g += (int)*ci->blender2a_g[1];
		*b += (int)*ci->blender2a_b[1];
	}

	*r >>= 2;
	*g >>= 2;
	*b >>= 2;

	if (m_other_modes->force_blend)
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

bool Blender::AlphaCompare(UINT8 alpha)
{
	INT32 threshold;
	if (m_other_modes->alpha_compare_en)
	{
		threshold = (m_other_modes->dither_alpha_en) ? (mame_rand(m_machine) & 0xff) : m_rdp->GetBlendColor()->i.a;
		if (alpha < threshold)
		{
			return false;
		}
		return true;
	}
	return true;
}

void Blender::DitherA(UINT8 *a, int dith)
{
	INT32 new_a = *a + dith;
	if(new_a & 0x100)
	{
		new_a = 0xff;
	}
	*a = (UINT8)new_a;
}

void Blender::DitherRGB(INT32 *r, INT32 *g, INT32 *b, int dith)
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
