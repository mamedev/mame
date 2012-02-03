#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

namespace RDP
{

void FramebufferT::Write(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b)
{
	switch(m_rdp->MiscState.FBSize)
	{
		case PIXEL_SIZE_16BIT:
			Write16Bit(curpixel, r, g, b);
			break;

		case PIXEL_SIZE_32BIT:
			Write32Bit(curpixel, r, g, b);
			break;

		default:
			fatalerror("Unsupported bit depth: %d\n", m_rdp->MiscState.FBSize);
			break;
	}
}

void FramebufferT::Write16Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b)
{
#undef CVG_DRAW
#ifdef CVG_DRAW
	int covdraw = (CurrentPixCvg - 1) << 5;
	r = covdraw;
	g = covdraw;
	b = covdraw;
#endif

	UINT32 fb = (m_rdp->MiscState.FBAddress >> 1) + curpixel;
	UINT32 hb = fb;

#if 0
	if (m_rdp->MiscState.CurrentPixCvg > 8 && m_rdp->OtherModes.z_mode != 1)
	{
		stricterror("FBWRITE_16: CurrentPixCvg %d", m_rdp->MiscState.CurrentPixCvg);
	}
#endif

	UINT16 finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);
	UINT32 finalcvg = 0;

	if (m_rdp->OtherModes.color_on_cvg && !m_pre_wrap)
	{
		finalcolor = RREADIDX16(fb) & 0xfffe;
	}

	switch(m_rdp->OtherModes.cvg_dest)
	{
	case 0:
		if (!m_rdp->Blender.BlendEnable)
		{
			finalcvg = (m_rdp->MiscState.CurrentPixCvg - 1) & 7;
			RWRITEIDX16(fb, finalcolor | ((finalcvg >> 2) & 1));
			HWRITEADDR8(hb, finalcvg & 3);
		}
		else
		{
			finalcvg = m_rdp->MiscState.CurrentPixCvg + m_rdp->MiscState.CurrentMemCvg;
			if (finalcvg & 8)
			{
				finalcvg = 7;
			}
			RWRITEIDX16(fb, finalcolor | ((finalcvg >> 2) & 1));
			HWRITEADDR8(hb, finalcvg & 3);
		}
		break;
	case 1:
		finalcvg = (m_rdp->MiscState.CurrentPixCvg + m_rdp->MiscState.CurrentMemCvg) & 7;
		RWRITEIDX16(fb, finalcolor | ((finalcvg >> 2) & 1));
		HWRITEADDR8(hb, finalcvg & 3);
		break;
	case 2:
		RWRITEIDX16(fb, finalcolor | 1);
		HWRITEADDR8(hb, 3);
		break;
	case 3:
		RWRITEIDX16(fb, finalcolor | ((m_rdp->MiscState.CurrentMemCvg >> 2) & 1));
		HWRITEADDR8(hb, m_rdp->MiscState.CurrentMemCvg & 3);
		break;
	}
}

void FramebufferT::Write32Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b)
{
	UINT32 fb = (m_rdp->MiscState.FBAddress >> 2) + curpixel;
	UINT32 finalcolor = (r << 24) | (g << 16) | (b << 8);//cvg as 3 MSBs of alpha channel;
	UINT32 finalcvg = 0;

#if 0
	if (CurrentPixCvg > 8 && m_rdp->OtherModes.z_mode != 1)
	{
		stricterror("FBWRITE_16: CurrentPixCvg %d", CurrentPixCvg);
	}
#endif

	if (m_rdp->OtherModes.color_on_cvg && !m_pre_wrap)
	{
		finalcolor = RREADIDX32(fb) & 0xffffff00;
	}

	switch(m_rdp->OtherModes.cvg_dest)
	{
	case 0: //normal
		if (!m_rdp->Blender.BlendEnable)
		{
			finalcvg = (m_rdp->MiscState.CurrentPixCvg - 1) & 7;
			finalcolor |= (finalcvg << 5);
			RWRITEIDX32(fb, finalcolor);
		}
		else
		{
			finalcvg = m_rdp->MiscState.CurrentPixCvg + m_rdp->MiscState.CurrentMemCvg;
			if (finalcvg & 8)
			{
				finalcvg = 7;
			}
			finalcolor |= (finalcvg << 5);
			RWRITEIDX32(fb, finalcolor);
		}
		break;
	case 1:
		finalcvg = (m_rdp->MiscState.CurrentPixCvg + m_rdp->MiscState.CurrentMemCvg) & 7;
		finalcolor |= (finalcvg << 5);
		RWRITEIDX32(fb, finalcolor);
		break;
	case 2:
		RWRITEIDX32(fb, finalcolor | 0xE0);
		break;
	case 3:
		finalcolor |= (m_rdp->MiscState.CurrentMemCvg << 5);
		RWRITEIDX32(fb, finalcolor);
		break;
	}
}

void FramebufferT::Read(UINT32 curpixel)
{
	switch(m_rdp->MiscState.FBSize)
	{
		case PIXEL_SIZE_16BIT:
			Read16Bit(curpixel);
			break;

		case PIXEL_SIZE_32BIT:
			Read32Bit(curpixel);
			break;

		default:
			fatalerror("Unsupported bit depth: %d\n", m_rdp->MiscState.FBSize);
			break;
	}
}

void FramebufferT::Read16Bit(UINT32 curpixel)
{
	UINT16 fword = RREADIDX16((m_rdp->MiscState.FBAddress >> 1) + curpixel);
	UINT8 hbyte = HREADADDR8((m_rdp->MiscState.FBAddress >> 1) + curpixel);
	m_rdp->MemoryColor.i.r = GETHICOL(fword);
	m_rdp->MemoryColor.i.g = GETMEDCOL(fword);
	m_rdp->MemoryColor.i.b = GETLOWCOL(fword);
	if (m_rdp->OtherModes.image_read_en)
	{
		m_rdp->MiscState.CurrentMemCvg = ((fword & 1) << 2) | (hbyte & 3);
		m_rdp->MemoryColor.i.a = m_rdp->MiscState.CurrentMemCvg << 5;
	}
	else
	{
		m_rdp->MiscState.CurrentMemCvg = 7;
		m_rdp->MemoryColor.i.a = 0xff;
	}
}

void FramebufferT::Read32Bit(UINT32 curpixel)
{
	UINT32 mem = RREADIDX32((m_rdp->MiscState.FBAddress >> 2) + curpixel);
	m_rdp->MemoryColor.i.r = (mem >> 24) & 0xff;
	m_rdp->MemoryColor.i.g = (mem >> 16) & 0xff;
	m_rdp->MemoryColor.i.b = (mem >> 8) & 0xff;
	if (m_rdp->OtherModes.image_read_en)
	{
		m_rdp->MiscState.CurrentMemCvg = (mem >> 5) & 7;
		m_rdp->MemoryColor.i.a = (mem) & 0xff;
	}
	else
	{
		m_rdp->MiscState.CurrentMemCvg = 7;
		m_rdp->MemoryColor.i.a = 0xff;
	}
}

void FramebufferT::Copy(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b)
{
	switch(m_rdp->MiscState.FBSize)
	{
		case PIXEL_SIZE_16BIT:
			Copy16Bit(curpixel, r, g, b);
			break;

		case PIXEL_SIZE_32BIT:
			Copy32Bit(curpixel, r, g, b);
			break;

		default:
			fatalerror("Unsupported bit depth: %d\n", m_rdp->MiscState.FBSize);
			break;
	}
}

void FramebufferT::Copy16Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b)
{
	UINT16 val = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | ((m_rdp->MiscState.CurrentPixCvg >> 2) & 1);
	RWRITEIDX16((m_rdp->MiscState.FBAddress >> 1) + curpixel, val);
	HWRITEADDR8((m_rdp->MiscState.FBAddress >> 1) + curpixel, m_rdp->MiscState.CurrentPixCvg & 3);
}

void FramebufferT::Copy32Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b)
{
	UINT32 val = (r << 24) | (g << 16) | (b << 8) | (m_rdp->MiscState.CurrentPixCvg << 5);
	RWRITEIDX32((m_rdp->MiscState.FBAddress >> 2) + curpixel, val);
}

void FramebufferT::Fill(UINT32 curpixel)
{
	switch(m_rdp->MiscState.FBSize)
	{
		case PIXEL_SIZE_16BIT:
			Fill16Bit(curpixel);
			break;

		case PIXEL_SIZE_32BIT:
			Fill32Bit(curpixel);
			break;

		default:
			fatalerror("Unsupported bit depth: %d\n", m_rdp->MiscState.FBSize);
			break;
	}
}

void FramebufferT::Fill16Bit(UINT32 curpixel)
{
	UINT16 val;
	if (curpixel & 1)
	{
		val = m_rdp->FillColor & 0xffff;
	}
	else
	{
		val = (m_rdp->FillColor >> 16) & 0xffff;
	}
	RWRITEIDX16((m_rdp->MiscState.FBAddress >> 1) + curpixel, val);
	HWRITEADDR8((m_rdp->MiscState.FBAddress >> 1) + curpixel, ((val & 1) << 1) | (val & 1));
}

void FramebufferT::Fill32Bit(UINT32 curpixel)
{
	UINT32 FillColor = m_rdp->FillColor;
	RWRITEIDX32((m_rdp->MiscState.FBAddress >> 2) + curpixel, FillColor);
	HWRITEADDR8((m_rdp->MiscState.FBAddress >> 1) + (curpixel << 1), (FillColor & 0x10000) ? 3 : 0);
	HWRITEADDR8((m_rdp->MiscState.FBAddress >> 1) + (curpixel << 1) + 1, (FillColor & 0x1) ? 3 : 0);
}

} // namespace RDP

} // namespace N64
