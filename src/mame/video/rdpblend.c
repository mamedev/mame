#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

bool RDP::Blender::Blend(void* in_fb, UINT8* hb, RDP::Color c1, RDP::Color c2, int dith)
{
	switch(m_misc_state->m_fb_size)
	{
		case PIXEL_SIZE_16BIT:
			return Blend16Bit((UINT16*)in_fb, hb, c1, c2, dith);

		case PIXEL_SIZE_32BIT:
			return Blend32Bit((UINT32*)in_fb, c1, c2);

		default:
			fatalerror("Unsupported bit depth: %d\n", m_misc_state->m_fb_size);
			break;
	}

	return false;
}

bool RDP::Blender::Blend16Bit(UINT16* fb, UINT8* hb, RDP::Color c1, RDP::Color c2, int dith)
{
	switch(m_other_modes->cycle_type)
	{
		case CYCLE_TYPE_1:
			return Blend16Bit1Cycle(fb, hb, c1, dith);

		case CYCLE_TYPE_2:
			return Blend16Bit2Cycle(fb, hb, c1, c2, dith);

		default:
			fatalerror("Unsupported cycle type for Blend16Bit: %d\n", m_other_modes->cycle_type);
			break;
	}

	return false;
}

bool RDP::Blender::Blend16Bit1Cycle(UINT16* fb, UINT8* hb, RDP::Color c, int dith)
{
	UINT16 mem = *fb;
	UINT32 memory_cvg = 7;
	if(m_other_modes->image_read_en)
	{
		memory_cvg = ((mem & 1) << 2) + (*hb & 3);
	}

	// Alpha compare
	if (!AlphaCompare(c.i.a))
	{
		return false;
	}

	if (!m_misc_state->m_curpixel_cvg) // New coverage is zero, so abort
	{
		return false;
	}

	int special_bsel = 0;
	if (m_rdp->GetColorInputs()->blender2b_a[0] == &m_rdp->GetMemoryColor()->i.a)
	{
		special_bsel = 1;
	}

	m_rdp->GetPixelColor()->c = c.c;

	if(!m_other_modes->z_compare_en)
	{
		m_misc_state->m_curpixel_overlap = 0;
	}

	m_rdp->GetMemoryColor()->c = m_rdp->LookUp16To32(mem);
	m_rdp->GetMemoryColor()->i.a = (memory_cvg << 5) & 0xe0;

	int r;
	int g;
	int b;

	if(m_other_modes->z_compare_en)
	{
		if(m_other_modes->force_blend)
		{
			m_rdp->GetInvPixelColor()->i.a = 0xff - *m_rdp->GetColorInputs()->blender1b_a[0];
			BlendEquation0Force(&r, &g, &b, special_bsel);
		}
		else
		{
			if (!m_misc_state->m_curpixel_overlap)
			{
				r = *m_rdp->GetColorInputs()->blender1a_r[0];
				g = *m_rdp->GetColorInputs()->blender1a_g[0];
				b = *m_rdp->GetColorInputs()->blender1a_b[0];
			}
			else
			{
				m_rdp->GetInvPixelColor()->i.a = 0xff - *m_rdp->GetColorInputs()->blender1b_a[0];

				BlendEquation0NoForce(&r, &g, &b, special_bsel);
			}
		}
	}
	else
	{
		if(m_other_modes->force_blend)
		{
			m_rdp->GetInvPixelColor()->i.a = 0xff - *m_rdp->GetColorInputs()->blender1b_a[0];

			BlendEquation0Force(&r, &g, &b, special_bsel);
		}
		else
		{
			r = *m_rdp->GetColorInputs()->blender1a_r[0];
			g = *m_rdp->GetColorInputs()->blender1a_g[0];
			b = *m_rdp->GetColorInputs()->blender1a_b[0];
		}
	}

	if(!(m_other_modes->rgb_dither_sel & 2))
	{
		// Hack to prevent "double-dithering" artifacts
		if (!(((r & 0xf8)==(m_rdp->GetMemoryColor()->i.r&0xf8) && (g & 0xf8) == (m_rdp->GetMemoryColor()->i.g & 0xf8) &&(b&0xf8)==(m_rdp->GetMemoryColor()->i.b&0xf8))))
		{
			DitherRGB(&r, &g, &b, dith);
		}
	}

    return m_rdp->GetFramebuffer()->Write(fb, hb, r, g, b);
}

bool RDP::Blender::Blend16Bit2Cycle(UINT16* fb, UINT8* hb, RDP::Color c1, RDP::Color c2, int dith)
{
	UINT16 mem = *fb;
	UINT32 memory_cvg = 7;
	if(m_other_modes->image_read_en)
	{
		memory_cvg = ((mem & 1) << 2) + (*hb & 3);
	}

	// Alpha compare
	if (!AlphaCompare(c2.i.a))
	{
		return false;
	}

	if (!m_misc_state->m_curpixel_cvg)
	{
		return false;
	}

	int special_bsel = 0;
	if (m_rdp->GetColorInputs()->blender2b_a[0] == &m_rdp->GetMemoryColor()->i.a)
	{
		special_bsel = 1;
	}

	m_rdp->GetPixelColor()->c = c2.c;

	if(m_other_modes->z_compare_en)
	{
		m_misc_state->m_curpixel_overlap = 0;
	}

	m_rdp->GetMemoryColor()->c = m_rdp->LookUp16To32(mem);
	m_rdp->GetMemoryColor()->i.a = (memory_cvg << 5) & 0xe0;

	m_rdp->GetInvPixelColor()->i.a = 0xff - *m_rdp->GetColorInputs()->blender1b_a[0];

	int r, g, b;
	if(m_other_modes->force_blend)
	{
		BlendEquation0Force(&r, &g, &b, special_bsel);
	}
	else
	{
		BlendEquation1Force(&r, &g, &b, special_bsel);
	}

	m_rdp->GetBlendedColor()->i.r = r;
	m_rdp->GetBlendedColor()->i.g = g;
	m_rdp->GetBlendedColor()->i.b = b;
	m_rdp->GetBlendedColor()->i.a = m_rdp->GetPixelColor()->i.a;

	m_rdp->GetPixelColor()->i.r = r;
	m_rdp->GetPixelColor()->i.g = g;
	m_rdp->GetPixelColor()->i.b = b;

	m_rdp->GetInvPixelColor()->i.a = 0xff - *m_rdp->GetColorInputs()->blender1b_a[1];

	if(m_other_modes->force_blend)
	{
		if (m_rdp->GetColorInputs()->blender2b_a[1] == &m_rdp->GetMemoryColor()->i.a)
		{
			special_bsel = 1;
		}
		else
		{
			special_bsel = 0;
		}

		BlendEquation1Force(&r, &g, &b, special_bsel);
	}
	else
	{
		if (!m_misc_state->m_curpixel_overlap)
		{
			r = *m_rdp->GetColorInputs()->blender1a_r[1];
			g = *m_rdp->GetColorInputs()->blender1a_g[1];
			b = *m_rdp->GetColorInputs()->blender1a_b[1];
		}
		else
		{
			if (m_rdp->GetColorInputs()->blender2b_a[1] == &m_rdp->GetMemoryColor()->i.a)
			{
				special_bsel = 1;
			}
			else
			{
				special_bsel = 0;
			}

			BlendEquation1NoForce(&r, &g, &b, special_bsel);
		}
	}

	if(!(m_other_modes->rgb_dither_sel & 2))
	{
		// Hack to prevent "double-dithering" artifacts
		if (!(((r & 0xf8)==(m_rdp->GetMemoryColor()->i.r&0xf8) && (g & 0xf8) == (m_rdp->GetMemoryColor()->i.g & 0xf8) &&(b&0xf8)==(m_rdp->GetMemoryColor()->i.b&0xf8))))
		{
			DitherRGB(&r, &g, &b, dith);
		}
	}

    return m_rdp->GetFramebuffer()->Write(fb, hb, r, g, b);
}

bool RDP::Blender::Blend32Bit(UINT32* fb, RDP::Color c1, RDP::Color c2)
{
	switch(m_other_modes->cycle_type)
	{
		case CYCLE_TYPE_1:
			return Blend32Bit1Cycle(fb, c1);

		case CYCLE_TYPE_2:
			return Blend32Bit2Cycle(fb, c1, c2);

		default:
			fatalerror("Unsupported cycle type for Blend16Bit: %d\n", m_other_modes->cycle_type);
			break;
	}

	return false;
}

bool RDP::Blender::Blend32Bit1Cycle(UINT32* fb, RDP::Color c)
{
	UINT32 mem = *fb;
	int r, g, b;

	// Alpha compare
	if (!AlphaCompare(c.i.a))
	{
		return 0;
	}

	if (!m_misc_state->m_curpixel_cvg)
	{
		return 0;
	}

	m_rdp->GetPixelColor()->c = c.c;
	if (!m_other_modes->z_compare_en)
	{
		m_misc_state->m_curpixel_overlap = 0;
	}

	m_rdp->GetMemoryColor()->i.r = (mem >> 24) & 0xff;
	m_rdp->GetMemoryColor()->i.g = (mem >> 16) & 0xff;
	m_rdp->GetMemoryColor()->i.b = (mem >> 8) & 0xff;

	if (m_other_modes->image_read_en)
	{
		m_rdp->GetMemoryColor()->i.a = mem & 0xe0;
	}
	else
	{
		m_rdp->GetMemoryColor()->i.a = 0xe0;
	}

	if (!m_misc_state->m_curpixel_overlap && !m_other_modes->force_blend)
	{
		r = *m_rdp->GetColorInputs()->blender1a_r[0];
		g = *m_rdp->GetColorInputs()->blender1a_g[0];
		b = *m_rdp->GetColorInputs()->blender1a_b[0];
	}
	else
	{
		m_rdp->GetInvPixelColor()->i.a = 0xff - *m_rdp->GetColorInputs()->blender1b_a[0];

		if(m_other_modes->force_blend)
		{
			BlendEquation0Force(&r, &g, &b, m_misc_state->m_special_bsel0);
		}
		else
		{
			BlendEquation0NoForce(&r, &g, &b, m_misc_state->m_special_bsel0);
		}
	}

    return m_rdp->GetFramebuffer()->Write(fb, NULL, r, g, b);
}

bool RDP::Blender::Blend32Bit2Cycle(UINT32* fb, RDP::Color c1, RDP::Color c2)
{
	UINT32 mem = *fb;
	int r, g, b;

	// Alpha compare
	if (!AlphaCompare(c2.i.a))
	{
		return 0;
	}

	if (!m_misc_state->m_curpixel_cvg)
	{
		return 0;
	}

	m_rdp->GetPixelColor()->c = c2.c;
	if (!m_other_modes->z_compare_en)
	{
		m_misc_state->m_curpixel_overlap = 0;
	}

	m_rdp->GetMemoryColor()->i.r = (mem >>24) & 0xff;
	m_rdp->GetMemoryColor()->i.g = (mem >> 16) & 0xff;
	m_rdp->GetMemoryColor()->i.b = (mem >> 8) & 0xff;

	if (m_other_modes->image_read_en)
	{
		m_rdp->GetMemoryColor()->i.a = (mem & 0xe0);
	}
	else
	{
		m_rdp->GetMemoryColor()->i.a = 0xe0;
	}

	m_rdp->GetInvPixelColor()->i.a = 0xff - *m_rdp->GetColorInputs()->blender1b_a[0];

	if(m_other_modes->force_blend)
	{
		BlendEquation0Force(&r, &g, &b, m_misc_state->m_special_bsel0);
	}
	else
	{
		BlendEquation0NoForce(&r, &g, &b, m_misc_state->m_special_bsel0);
	}

	m_rdp->GetBlendedColor()->i.r = r;
	m_rdp->GetBlendedColor()->i.g = g;
	m_rdp->GetBlendedColor()->i.b = b;

	m_rdp->GetPixelColor()->i.r = r;
	m_rdp->GetPixelColor()->i.g = g;
	m_rdp->GetPixelColor()->i.b = b;

	if (!m_misc_state->m_curpixel_overlap && !m_other_modes->force_blend)
	{
		r = *m_rdp->GetColorInputs()->blender1a_r[1];
		g = *m_rdp->GetColorInputs()->blender1a_g[1];
		b = *m_rdp->GetColorInputs()->blender1a_b[1];
	}
	else
	{
		m_rdp->GetInvPixelColor()->i.a = 0xff - *m_rdp->GetColorInputs()->blender1b_a[1];

		if(m_other_modes->force_blend)
		{
			BlendEquation1Force(&r, &g, &b, m_misc_state->m_special_bsel1);
		}
		else
		{
			BlendEquation1NoForce(&r, &g, &b, m_misc_state->m_special_bsel1);
		}
	}

    return m_rdp->GetFramebuffer()->Write(fb, NULL, r, g, b);
}

void RDP::Blender::BlendEquation0Force(INT32* r, INT32* g, INT32* b, int bsel_special)
{
	int blend1a = *m_rdp->GetColorInputs()->blender1b_a[0];
	if (bsel_special)
	{
		blend1a &= 0xe0;
	}

	INT32 tr = ((int)(*m_rdp->GetColorInputs()->blender1a_r[0]) * blend1a + (int)(*m_rdp->GetColorInputs()->blender2a_r[0]) * *m_rdp->GetColorInputs()->blender2b_a[0] + ((int)(*m_rdp->GetColorInputs()->blender2a_r[0]) << (3 + bsel_special))) >> 8;
	INT32 tg = ((int)(*m_rdp->GetColorInputs()->blender1a_g[0]) * blend1a + (int)(*m_rdp->GetColorInputs()->blender2a_g[0]) * *m_rdp->GetColorInputs()->blender2b_a[0] + ((int)(*m_rdp->GetColorInputs()->blender2a_g[0]) << (3 + bsel_special))) >> 8;
	INT32 tb = ((int)(*m_rdp->GetColorInputs()->blender1a_b[0]) * blend1a + (int)(*m_rdp->GetColorInputs()->blender2a_b[0]) * *m_rdp->GetColorInputs()->blender2b_a[0] + ((int)(*m_rdp->GetColorInputs()->blender2a_b[0]) << (3 + bsel_special))) >> 8;

	if (tr > 255) *r = 255; else *r = tr;
	if (tg > 255) *g = 255; else *g = tg;
	if (tb > 255) *b = 255; else *b = tb;
}

void RDP::Blender::BlendEquation0NoForce(INT32* r, INT32* g, INT32* b, int bsel_special)
{
	UINT8 blend1a = *m_rdp->GetColorInputs()->blender1b_a[0];
	UINT8 blend2a = *m_rdp->GetColorInputs()->blender2b_a[0];
	if (bsel_special)
	{
		blend1a &= 0xe0;
	}

	UINT32 sum = (((blend1a >> 5) + (blend2a >> 5) + 1) & 0xf) << 5;

	INT32 tr = (((int)(*m_rdp->GetColorInputs()->blender1a_r[0]) * (int)(blend1a))) +
		(((int)(*m_rdp->GetColorInputs()->blender2a_r[0]) * (int)(blend2a)));
	tr += (bsel_special) ? (((int)(*m_rdp->GetColorInputs()->blender2a_r[0])) << 5) : (((int)(*m_rdp->GetColorInputs()->blender2a_r[0])) << 3);

	INT32 tg = (((int)(*m_rdp->GetColorInputs()->blender1a_g[0]) * (int)(blend1a))) +
		(((int)(*m_rdp->GetColorInputs()->blender2a_g[0]) * (int)(blend2a)));
	tg += (bsel_special) ? ((int)((*m_rdp->GetColorInputs()->blender2a_g[0])) << 5) : (((int)(*m_rdp->GetColorInputs()->blender2a_g[0])) << 3);

	INT32 tb = (((int)(*m_rdp->GetColorInputs()->blender1a_b[0]) * (int)(blend1a))) +
		(((int)(*m_rdp->GetColorInputs()->blender2a_b[0]) * (int)(blend2a)));
	tb += (bsel_special) ? (((int)(*m_rdp->GetColorInputs()->blender2a_b[0])) << 5) : (((int)(*m_rdp->GetColorInputs()->blender2a_b[0])) << 3);

	if (sum)
	{
		tr /= sum;
		tg /= sum;
		tb /= sum;
	}
	else
	{
		*r = *g = *b = 0xff;
		return;
	}

	if (tr > 255) *r = 255; else *r = tr;
	if (tg > 255) *g = 255; else *g = tg;
	if (tb > 255) *b = 255; else *b = tb;
}

void RDP::Blender::BlendEquation1Force(INT32* r, INT32* g, INT32* b, int bsel_special)
{
	UINT8 blend1a = *m_rdp->GetColorInputs()->blender1b_a[1];
	UINT8 blend2a = *m_rdp->GetColorInputs()->blender2b_a[1];
	if (bsel_special)
	{
		blend1a &= 0xe0;
	}

	INT32 tr = (((int)(*m_rdp->GetColorInputs()->blender1a_r[1]) * (int)(blend1a))) +
		(((int)(*m_rdp->GetColorInputs()->blender2a_r[1]) * (int)(blend2a)));
	tr += (bsel_special) ? (((int)(*m_rdp->GetColorInputs()->blender2a_r[1])) << 5) : (((int)(*m_rdp->GetColorInputs()->blender2a_r[1])) << 3);

	INT32 tg = (((int)(*m_rdp->GetColorInputs()->blender1a_g[1]) * (int)(blend1a))) +
		(((int)(*m_rdp->GetColorInputs()->blender2a_g[1]) * (int)(blend2a)));
	tg += (bsel_special) ? ((int)((*m_rdp->GetColorInputs()->blender2a_g[1])) << 5) : (((int)(*m_rdp->GetColorInputs()->blender2a_g[1])) << 3);

	INT32 tb = (((int)(*m_rdp->GetColorInputs()->blender1a_b[1]) * (int)(blend1a))) +
		(((int)(*m_rdp->GetColorInputs()->blender2a_b[1]) * (int)(blend2a)));
	tb += (bsel_special) ? (((int)(*m_rdp->GetColorInputs()->blender2a_b[1])) << 5) : (((int)(*m_rdp->GetColorInputs()->blender2a_b[1])) << 3);

	tr >>= 8;
	tg >>= 8;
	tb >>= 8;

	if (tr > 255) *r = 255; else *r = tr;
	if (tg > 255) *g = 255; else *g = tg;
	if (tb > 255) *b = 255; else *b = tb;
}

void RDP::Blender::BlendEquation1NoForce(INT32* r, INT32* g, INT32* b, int bsel_special)
{
	UINT8 blend1a = *m_rdp->GetColorInputs()->blender1b_a[1];
	UINT8 blend2a = *m_rdp->GetColorInputs()->blender2b_a[1];
	if (bsel_special)
	{
		blend1a &= 0xe0;
	}

	UINT32 sum = (((blend1a >> 5) + (blend2a >> 5) + 1) & 0xf) << 5;

	INT32 tr = (((int)(*m_rdp->GetColorInputs()->blender1a_r[1]) * (int)(blend1a))) +
		(((int)(*m_rdp->GetColorInputs()->blender2a_r[1]) * (int)(blend2a)));
	tr += (bsel_special) ? (((int)(*m_rdp->GetColorInputs()->blender2a_r[1])) << 5) : (((int)(*m_rdp->GetColorInputs()->blender2a_r[1])) << 3);

	INT32 tg = (((int)(*m_rdp->GetColorInputs()->blender1a_g[1]) * (int)(blend1a))) +
		(((int)(*m_rdp->GetColorInputs()->blender2a_g[1]) * (int)(blend2a)));
	tg += (bsel_special) ? ((int)((*m_rdp->GetColorInputs()->blender2a_g[1])) << 5) : (((int)(*m_rdp->GetColorInputs()->blender2a_g[1])) << 3);

	INT32 tb = (((int)(*m_rdp->GetColorInputs()->blender1a_b[1]) * (int)(blend1a))) +
		(((int)(*m_rdp->GetColorInputs()->blender2a_b[1]) * (int)(blend2a)));
	tb += (bsel_special) ? (((int)(*m_rdp->GetColorInputs()->blender2a_b[1])) << 5) : (((int)(*m_rdp->GetColorInputs()->blender2a_b[1])) << 3);

	if (sum)
	{
		tr /= sum;
		tg /= sum;
		tb /= sum;
	}
	else
	{
		*r = *g = *b = 0xff;
		return;
	}

	if (tr > 255) *r = 255; else *r = tr;
	if (tg > 255) *g = 255; else *g = tg;
	if (tb > 255) *b = 255; else *b = tb;
}

bool RDP::Blender::AlphaCompare(UINT8 alpha)
{
	if(m_other_modes->alpha_compare_en)
	{
		if(m_other_modes->dither_alpha_en)
		{
			return (alpha > m_rdp->GetRandom());
		}
		else
		{
			return (alpha > m_rdp->GetBlendColor()->i.a);
		}
	}
	return true;
}

void RDP::Blender::DitherRGB(INT32 *r, INT32 *g, INT32 *b, int dith)
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

} // namespace N64