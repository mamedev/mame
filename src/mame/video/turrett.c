// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*************************************************************************

    Turrett Tower video hardware

*************************************************************************/

#include "emu.h"
#include "machine/idectrl.h"
#include "includes/turrett.h"



inline UINT8 clamp_5bit(INT8 val)
{
	if (val < 0)
		return 0;

	if (val > 31)
		return 31;

	return val;
}


UINT32 turrett_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int page = (m_video_ctrl & 1) ^ 1;

	const UINT16 *vram = m_video_ram[page];

	INT8 fade_b = m_video_fade & 0x1f;
	INT8 fade_g = (m_video_fade >> 5) & 0x1f;
	INT8 fade_r = (m_video_fade >> 10) & 0x1f;

	if (m_video_fade & 0x8000)
	{
		fade_b = -fade_b;
		fade_g = -fade_g;
		fade_r = -fade_r;
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		const UINT16 *src = &vram[y * X_VISIBLE + cliprect.min_x];
		UINT16 *dest = &bitmap.pix16(y, cliprect.min_x);

		if (m_video_fade != 0)
		{
			for (int x = cliprect.min_x; x <= cliprect.max_x; ++x)
			{
				UINT16 srcpix = *src++;

				UINT8 src_b = srcpix & 0x1f;
				UINT8 src_g = (srcpix >> 5) & 0x1f;
				UINT8 src_r = (srcpix >> 10) & 0x1f;

				UINT8 dst_b = clamp_5bit(src_b + fade_b);
				UINT8 dst_g = clamp_5bit(src_g + fade_g);
				UINT8 dst_r = clamp_5bit(src_r + fade_r);

				*dest++ = (dst_r << 10) | (dst_g << 5) | dst_b;
			}
		}
		else
		{
			for (int x = cliprect.min_x; x <= cliprect.max_x; ++x)
			{
				*dest++ = *src++ & 0x7fff;
			}
		}
	}

	return 0;
}


UINT32 turrett_state::write_video_ram(UINT16 data)
{
	UINT32 clocks = 1;

	if (!m_skip_x && !m_skip_y)
	{
		// Handle hot spot test
		if (m_x_pos == m_hotspot_x
		&&  m_y_pos == (m_hotspot_y & 0xfff))
		{
			m_hotspot_y |= 0x8000;
		}

		if (m_x_pos >= 0 && m_x_pos < X_VISIBLE
		&&  m_y_pos >= 0 && m_y_pos < Y_VISIBLE)
		{
			int address = m_y_pos * X_VISIBLE + m_x_pos;

			UINT16 *vramptr = &m_video_ram[m_video_ctrl & 1][address];
			UINT16 srcpix = data;
			UINT16 dstpix = data;

			// Blending enabled?
			if (data & 0x8000)
			{
				dstpix = *vramptr;

				UINT8 src_b = srcpix & 0x1f;
				UINT8 src_g = (srcpix >> 5) & 0x1f;
				UINT8 src_r = (srcpix >> 10) & 0x1f;

				UINT8 dst_b = dstpix & 0x1f;
				UINT8 dst_g = (dstpix >> 5) & 0x1f;
				UINT8 dst_r = (dstpix >> 10) & 0x1f;

				// Additive
				if (m_video_ctrl & 2)
				{
					dst_b = clamp_5bit(src_b + dst_b);
					dst_g = clamp_5bit(src_g + dst_g);
					dst_r = clamp_5bit(src_r + dst_r);
				}
				else
				{
					// R always seems to be 0 for blended pixels
					if ((src_g & 1) && (src_b & 1))
					{
						dst_b = clamp_5bit(dst_b - src_b);
						dst_g = clamp_5bit(dst_g - src_g);
						dst_r = clamp_5bit(dst_r - src_r);
					}
					else
					{
						// 75% source, 25% destination?
						dst_b = (src_b - (src_b >> 2)) + (dst_b >> 2);
						dst_g = (src_g - (src_g >> 2)) + (dst_g >> 2);
						dst_r = (src_r - (src_r >> 2)) + (dst_r >> 2);
					}
				}
				clocks += 2;
				*vramptr = (dst_r << 10) | (dst_g << 5) | dst_b;
			}
			else
			{
				clocks += 2;
				*vramptr = srcpix;
			}
		}
	}
	update_video_addr();

	return clocks;
}


void turrett_state::update_video_addr(void)
{
	// Handle auto-increment
	if (m_dx == m_x_mod)
	{
		m_dx = 0;
		m_scale_cnt_y += m_scale;

		if (m_scale_cnt_y & 0x800)
		{
			m_skip_y = false;
			m_scale_cnt_y &= 0x7ff;
			--m_y_pos;
			m_x_pos = m_x_start;
		}
		else
		{
			m_skip_y = true;
		}
	}
	else
	{
		++m_dx;
		m_scale_cnt_x += m_scale;

		if (m_scale_cnt_x & 0x800)
		{
			m_scale_cnt_x &= 0x7ff;
			m_skip_x = false;
			++m_x_pos;
		}
		else
		{
			m_skip_x = true;
		}
	}
}


READ32_MEMBER( turrett_state::video_r )
{
	UINT32 ret = 0;

	if (offset == 3 && mem_mask == 0x0000ffff)
	{
		// Collision detection flag
		ret = m_hotspot_y & 0x8000;
	}
	else
	{
		fatalerror("Unhandled video read (%x %x)!", offset, mem_mask);
	}

	return ret;
}


WRITE32_MEMBER( turrett_state::video_w )
{
	switch (offset)
	{
		case 0:
		{
			if (mem_mask == 0xffff0000)
			{
				data >>= 16;

				// TODO: Merge with DMA code?
				if ((data & 0xc400) == 0xc400)
				{
					// RLE word
					int count = (data & 0x3ff) + 1;

					// TODO: Cycle stalling
					while (count--)
						write_video_ram(m_last_pixel);
				}
				else
				{
					write_video_ram(data);

					// Store current pixel
					m_last_pixel = data;
				}
			}
			else
			{
				m_video_ctrl = data & 3;
			}
			break;
		}
		case 1:
		{
			m_x_mod = data & 0xffff;
			m_scale = 0x800 - (data >> 16);
			break;
		}
		case 2:
		{
			m_y_pos = data & 0xffff;
			m_x_pos = data >> 16;

			// Seems the logical place to set these
			m_x_start = m_x_pos;
			m_skip_x = false;
			m_skip_y = false;
			m_dx = 0;
			break;
		}
		case 3:
		{
			if (mem_mask == 0xffff0000)
			{
				m_video_fade = data >> 16;
			}
			else if (mem_mask == 0x0000ffff)
			{
				if (data & 0x4000)
					m_hotspot_y = data;
				else
					m_hotspot_x = data;
			}
			else
			{
				fatalerror("Unhandled");
			}
			break;
		}
		default:
			fatalerror("Unhandled video write: %x %x\n", offset, data);
	}
}


TIMER_CALLBACK_MEMBER( turrett_state::dma_complete )
{
	m_dma_idle = true;
}


WRITE32_MEMBER( turrett_state::dma_w )
{
	int bank = ((offset & 2) >> 1) ^ 1;

	if ((offset & 1) == 0)
	{
		m_dma_addr[bank] = data;
	}
	else
	{
		UINT32 clocks = 0;
		UINT32 words = data & 0x0fffffff;

		// IDE to DRAM
		if (data & 0x10000000)
		{
			UINT32 addr = m_dma_addr[bank];
			UINT16 *ram = bank ? m_bank_b : m_bank_a;

			while (words--)
			{
				ram[addr & DIMM_BANK_MASK] = m_ata->read_cs0(space, 0, 0xffff);
				++addr;
			}

			clocks = 500; // TODO: May be too high
			m_dma_addr[bank] = addr;
		}
		// IDE to video RAM
		else if (data & 0x40000000)
		{
			while (words--)
			{
				UINT16 data = m_ata->read_cs0(space, 0, 0xffff);

				// TODO: Verify if this is correct
				if ((data & 0xc400) == 0xc400)
				{
					fatalerror("IDE RLE detected");

					// RLE word
					int count = (data & 0x3ff) + 1;

					while (count--)
						write_video_ram(m_last_pixel);
				}
				else
				{
					write_video_ram(data);

					// Store current pixel
					m_last_pixel = data;
				}
			}

			clocks = 500; // TODO
		}
		// RAM to video RAM
		else if (data & 0x80000000)
		{
			UINT32 addr = m_dma_addr[bank];
			UINT16 *ram = bank ? m_bank_b : m_bank_a;

			//bool first = true; // Does it matter?

			while (words--)
			{
				UINT16 val = ram[addr++];
				//++clocks;

				switch (val & 0xc400)
				{
					// Transparent run
					case 0x8400:
					{
						int run = (((val & 0x3800) >> 1) | (val & 0x03ff)) + 1;

						while (run--)
						{
							update_video_addr();
							//++clocks;
						}

						break;
					}

					case 0xc400:
					{
						int run = (((val & 0x3800) >> 1) | (val & 0x03ff)) + 1;

						while (run--)
							clocks += write_video_ram(m_last_pixel);

						break;
					}

					default:
					{
						m_last_pixel = val;
						clocks += write_video_ram(val);
						break;
					}
				}
				//first = false;
			}

		//   clocks =1;///= 2;
		}
		else
		{
			popmessage("Unhandled DMA case: %.8x, contact MAMEdev!\n", data);
		}

		// Set the DMA completion timer
		m_dma_idle = false;
		m_dma_timer->adjust(attotime::from_nsec(10) * clocks, 0);
	}
}
