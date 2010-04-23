#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

bool RDP::Framebuffer::Write(void *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
{
	switch(m_misc_state->m_fb_size)
	{
		case PIXEL_SIZE_16BIT:
			return Write16Bit((UINT16*)fb, hb, r, g, b);

		case PIXEL_SIZE_32BIT:
			return Write32Bit((UINT32*)fb, r, g, b);

		default:
			fatalerror("Unsupported bit depth: %d\n", m_misc_state->m_fb_size);
			break;
	}

	return false;
}

bool RDP::Framebuffer::Write16Bit(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
{
#undef CVG_DRAW
//#define CVG_DRAW
#ifdef CVG_DRAW
	int covdraw;
	if (m_misc_state->m_curpixel_cvg == 8)
	{
		covdraw = 255;
	}
	else
	{
		covdraw = m_misc_state->m_curpixel_cvg << 5;
	}
	r = covdraw;
	g = covdraw;
	b = covdraw;
#endif

	if (!m_other_modes->z_compare_en)
	{
		m_misc_state->m_curpixel_overlap = 0;
	}

	UINT32 memory_cvg = 8;
	if (m_other_modes->image_read_en)
	{
		memory_cvg = ((*fb & 1) << 2) + (*hb & 3) + 1;
	}

	UINT32 newcvg = m_misc_state->m_curpixel_cvg + memory_cvg;
	bool wrapped = newcvg > 8;

	UINT16 finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	UINT32 clamped_cvg = wrapped ? 8 : newcvg;
	newcvg = wrapped ? (newcvg - 8) : newcvg;

	m_misc_state->m_curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clamped_cvg--;

	if (m_other_modes->color_on_cvg && !wrapped)
	{
		*fb &= 0xfffe;
		*fb |= ((newcvg >> 2) & 1);
		*hb = (newcvg & 3);
		return false;
	}

	switch(m_other_modes->cvg_dest)
	{
		case 0:
			if (!m_other_modes->force_blend && !m_misc_state->m_curpixel_overlap)
			{
				*fb = finalcolor | ((m_misc_state->m_curpixel_cvg >> 2) & 1);
				*hb = (m_misc_state->m_curpixel_cvg & 3);
			}
			else
			{
				*fb = finalcolor | ((clamped_cvg >> 2) & 1);
				*hb = (clamped_cvg & 3);
			}
			break;

		case 1:
			*fb = finalcolor | ((newcvg >> 2) & 1);
			*hb = (newcvg & 3);
			break;

		case 2:
			*fb = finalcolor | 1;
			*hb = 3;
			break;

		case 3:
			*fb = finalcolor | ((memory_cvg >> 2) & 1);
			*hb = (memory_cvg & 3);
			break;
	}

	return true;
}

bool RDP::Framebuffer::Write32Bit(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
{
	UINT32 finalcolor = (r << 24) | (g << 16) | (b << 8);
	UINT32 memory_alphachannel = *fb & 0xff;

	UINT32 memory_cvg = 8;
	if (m_other_modes->image_read_en)
	{
		memory_cvg = ((*fb >>5) & 7) + 1;
	}

	UINT32 newcvg = m_misc_state->m_curpixel_cvg + memory_cvg;
	bool wrapped = (newcvg > 8);
	UINT32 clamped_cvg = wrapped ? 8 : newcvg;
	newcvg = (wrapped) ? (newcvg - 8) : newcvg;

	m_misc_state->m_curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clamped_cvg--;

	if (m_other_modes->color_on_cvg && !wrapped)
	{
		*fb &= 0xffffff00;
		*fb |= ((newcvg << 5) & 0xff);
		return 0;
	}

	switch(m_other_modes->cvg_dest)
	{
		case 0:
			if (!m_other_modes->force_blend && !m_misc_state->m_curpixel_overlap)
			{
				*fb = finalcolor|(m_misc_state->m_curpixel_cvg << 5);
			}
			else
			{
				*fb = finalcolor|(clamped_cvg << 5);
			}
			break;
		case 1:
			*fb = finalcolor | (newcvg << 5);
			break;
		case 2:
			*fb = finalcolor | 0xE0;
			break;
		case 3:
			*fb = finalcolor | memory_alphachannel;
			break;
	}

	return true;
}

} // namespace N64
