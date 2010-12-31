#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

namespace RDP
{

void Framebuffer::Write(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b)
{
	switch(m_misc_state->m_fb_size)
	{
		case PIXEL_SIZE_16BIT:
			Write16Bit(curpixel, r, g, b);
			break;

		case PIXEL_SIZE_32BIT:
			Write32Bit(curpixel, r, g, b);
			break;

		default:
			fatalerror("Unsupported bit depth: %d\n", m_misc_state->m_fb_size);
			break;
	}
}

void Framebuffer::Write16Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b)
{
#undef CVG_DRAW
#ifdef CVG_DRAW
	int covdraw = (curpixel_cvg - 1) << 5;
	r = covdraw;
	g = covdraw;
	b = covdraw;
#endif

	UINT32 fb = (m_misc_state->m_fb_address >> 1) + curpixel;
	UINT32 hb = fb;

#if 0
	if (m_misc_state->m_curpixel_cvg > 8 && m_other_modes->z_mode != 1)
	{
		stricterror("FBWRITE_16: curpixel_cvg %d", m_misc_state->m_curpixel_cvg);
	}
#endif

	UINT16 finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);
	UINT32 finalcvg = 0;

	if (m_other_modes->color_on_cvg && !m_pre_wrap)
	{
		finalcolor = RREADIDX16(fb) & 0xfffe;
	}

	switch(m_other_modes->cvg_dest)
	{
	case 0:
		if (!m_rdp->GetBlender()->GetBlendEnable())
		{
			finalcvg = (m_misc_state->m_curpixel_cvg - 1) & 7;
			RWRITEIDX16(fb, finalcolor | ((finalcvg >> 2) & 1));
			HWRITEADDR8(hb, finalcvg & 3);
		}
		else
		{
			finalcvg = m_misc_state->m_curpixel_cvg + m_misc_state->m_curpixel_memcvg;
			if (finalcvg & 8)
			{
				finalcvg = 7;
			}
			RWRITEIDX16(fb, finalcolor | ((finalcvg >> 2) & 1));
			HWRITEADDR8(hb, finalcvg & 3);
		}
		break;
	case 1:
		finalcvg = (m_misc_state->m_curpixel_cvg + m_misc_state->m_curpixel_memcvg) & 7;
		RWRITEIDX16(fb, finalcolor | ((finalcvg >> 2) & 1));
		HWRITEADDR8(hb, finalcvg & 3);
		break;
	case 2:
		RWRITEIDX16(fb, finalcolor | 1);
		HWRITEADDR8(hb, 3);
		break;
	case 3:
		RWRITEIDX16(fb, finalcolor | ((m_misc_state->m_curpixel_memcvg >> 2) & 1));
		HWRITEADDR8(hb, m_misc_state->m_curpixel_memcvg & 3);
		break;
	}
}

void Framebuffer::Write32Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b)
{
	UINT32 fb = (m_misc_state->m_fb_address >> 2) + curpixel;
	UINT32 finalcolor = (r << 24) | (g << 16) | (b << 8);//cvg as 3 MSBs of alpha channel;
	UINT32 finalcvg = 0;

#if 0
	if (curpixel_cvg > 8 && m_other_modes->z_mode != 1)
	{
		stricterror("FBWRITE_16: curpixel_cvg %d", curpixel_cvg);
	}
#endif

	if (m_other_modes->color_on_cvg && !m_pre_wrap)
	{
		finalcolor = RREADIDX32(fb) & 0xffffff00;
	}

	switch(m_other_modes->cvg_dest)
	{
	case 0: //normal
		if (!m_rdp->GetBlender()->GetBlendEnable())
		{
			finalcvg = (m_misc_state->m_curpixel_cvg - 1) & 7;
			finalcolor |= (finalcvg << 5);
			RWRITEIDX32(fb, finalcolor);
		}
		else
		{
			finalcvg = m_misc_state->m_curpixel_cvg + m_misc_state->m_curpixel_memcvg;
			if (finalcvg & 8)
			{
				finalcvg = 7;
			}
			finalcolor |= (finalcvg << 5);
			RWRITEIDX32(fb, finalcolor);
		}
		break;
	case 1:
		finalcvg = (m_misc_state->m_curpixel_cvg + m_misc_state->m_curpixel_memcvg) & 7;
		finalcolor |= (finalcvg << 5);
		RWRITEIDX32(fb, finalcolor);
		break;
	case 2:
		RWRITEIDX32(fb, finalcolor | 0xE0);
		break;
	case 3:
		finalcolor |= (m_misc_state->m_curpixel_memcvg << 5);
		RWRITEIDX32(fb, finalcolor);
		break;
	}
}

void Framebuffer::Read(UINT32 curpixel)
{
	switch(m_misc_state->m_fb_size)
	{
		case PIXEL_SIZE_16BIT:
			Read16Bit(curpixel);
			break;

		case PIXEL_SIZE_32BIT:
			Read32Bit(curpixel);
			break;

		default:
			fatalerror("Unsupported bit depth: %d\n", m_misc_state->m_fb_size);
			break;
	}
}

void Framebuffer::Read16Bit(UINT32 curpixel)
{
	UINT16 fword = RREADIDX16((m_misc_state->m_fb_address >> 1) + curpixel);
	UINT8 hbyte = HREADADDR8((m_misc_state->m_fb_address >> 1) + curpixel);
	m_rdp->GetMemoryColor()->i.r = GETHICOL(fword);
	m_rdp->GetMemoryColor()->i.g = GETMEDCOL(fword);
	m_rdp->GetMemoryColor()->i.b = GETLOWCOL(fword);
	if (m_other_modes->image_read_en)
	{
		m_misc_state->m_curpixel_memcvg = ((fword & 1) << 2) | (hbyte & 3);
		m_rdp->GetMemoryColor()->i.a = m_misc_state->m_curpixel_memcvg << 5;
	}
	else
	{
		m_misc_state->m_curpixel_memcvg = 7;
		m_rdp->GetMemoryColor()->i.a = 0xff;
	}
}

void Framebuffer::Read32Bit(UINT32 curpixel)
{
	UINT32 mem = RREADIDX32((m_misc_state->m_fb_address >> 2) + curpixel);
	m_rdp->GetMemoryColor()->i.r = (mem >> 24) & 0xff;
	m_rdp->GetMemoryColor()->i.g = (mem >> 16) & 0xff;
	m_rdp->GetMemoryColor()->i.b = (mem >> 8) & 0xff;
	if (m_other_modes->image_read_en)
	{
		m_misc_state->m_curpixel_memcvg = (mem >> 5) & 7;
		m_rdp->GetMemoryColor()->i.a = (mem) & 0xff;
	}
	else
	{
		m_misc_state->m_curpixel_memcvg = 7;
		m_rdp->GetMemoryColor()->i.a = 0xff;
	}
}

void Framebuffer::Copy(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b)
{
	switch(m_misc_state->m_fb_size)
	{
		case PIXEL_SIZE_16BIT:
			Copy16Bit(curpixel, r, g, b);
			break;

		case PIXEL_SIZE_32BIT:
			Copy32Bit(curpixel, r, g, b);
			break;

		default:
			fatalerror("Unsupported bit depth: %d\n", m_misc_state->m_fb_size);
			break;
	}
}

void Framebuffer::Copy16Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b)
{
	UINT16 val = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | ((m_misc_state->m_curpixel_cvg >> 2) & 1);
	RWRITEIDX16((m_misc_state->m_fb_address >> 1) + curpixel, val);
	HWRITEADDR8((m_misc_state->m_fb_address >> 1) + curpixel, m_misc_state->m_curpixel_cvg & 3);
}

void Framebuffer::Copy32Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b)
{
	UINT32 val = (r << 24) | (g << 16) | (b << 8) | (m_misc_state->m_curpixel_cvg << 5);
	RWRITEIDX32((m_misc_state->m_fb_address >> 2) + curpixel, val);
}

void Framebuffer::Fill(UINT32 curpixel)
{
	switch(m_misc_state->m_fb_size)
	{
		case PIXEL_SIZE_16BIT:
			Fill16Bit(curpixel);
			break;

		case PIXEL_SIZE_32BIT:
			Fill32Bit(curpixel);
			break;

		default:
			fatalerror("Unsupported bit depth: %d\n", m_misc_state->m_fb_size);
			break;
	}
}

void Framebuffer::Fill16Bit(UINT32 curpixel)
{
	UINT16 val;
	if (curpixel & 1)
	{
		val = m_rdp->GetFillColor32() & 0xffff;
	}
	else
	{
		val = (m_rdp->GetFillColor32() >> 16) & 0xffff;
	}
	RWRITEIDX16((m_misc_state->m_fb_address >> 1) + curpixel, val);
	HWRITEADDR8((m_misc_state->m_fb_address >> 1) + curpixel, ((val & 1) << 1) | (val & 1));
}

void Framebuffer::Fill32Bit(UINT32 curpixel)
{
	UINT32 fill_color = m_rdp->GetFillColor32();
	RWRITEIDX32((m_misc_state->m_fb_address >> 2) + curpixel, fill_color);
	HWRITEADDR8((m_misc_state->m_fb_address >> 1) + (curpixel << 1), (fill_color & 0x10000) ? 3 : 0);
	HWRITEADDR8((m_misc_state->m_fb_address >> 1) + (curpixel << 1) + 1, (fill_color & 0x1) ? 3 : 0);
}

} // namespace RDP

} // namespace N64
