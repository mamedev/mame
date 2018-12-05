// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    P&P Marketing Police Trainer hardware

***************************************************************************/

#include "emu.h"
#include "cpu/mips/mips1.h"
#include "includes/policetr.h"


/*************************************
 *
 *  Video system start
 *
 *************************************/

void policetr_state::video_start()
{
	/* compute the height */
	m_srcbitmap_height_mask = (m_srcbitmap.bytes() / SRCBITMAP_WIDTH) - 1;

	/* the destination bitmap is not directly accessible to the CPU */
	m_dstbitmap = std::make_unique<bitmap_ind8>(+DSTBITMAP_WIDTH, +DSTBITMAP_HEIGHT);

	save_item(NAME(m_palette_offset));
	save_item(NAME(m_palette_index));
	save_item(NAME(m_palette_data));
	save_item(NAME(m_src_xoffs));
	save_item(NAME(m_src_yoffs));
	save_item(NAME(m_dst_xoffs));
	save_item(NAME(m_dst_yoffs));
	save_item(NAME(m_video_latch));
}



/*************************************
 *
 *  Display list processor
 *
 *************************************/

void policetr_state::render_display_list(offs_t offset)
{
	/* mask against the R3000 address space */
	offset &= 0x1fffffff;

	/* loop over all items */
	while (offset != 0x1fffffff)
	{
		const uint32_t *entry = &m_rambase[offset / 4];
		uint32_t srcx = entry[0] & 0xfffffff;
		uint32_t srcy = entry[1] & ((m_srcbitmap_height_mask << 16) | 0xffff);
		const uint32_t srcxstep = entry[2];
		const uint32_t srcystep = entry[3];
		int dstw = (entry[4] & 0x1ff) + 1;
		int dsth = ((entry[4] >> 12) & 0x1ff) + 1;
		int dstx = entry[5] & 0x1ff;
		int dsty = (entry[5] >> 12) & 0x1ff;
		const uint8_t mask = ~entry[6] >> 16;
		const uint8_t color = (entry[6] >> 24) & ~mask;

		if (dstx > m_render_clip.max_x)
		{
			dstw -= (512 - dstx);
			dstx = 0;
		}
		/* apply X clipping */
		if (dstx < m_render_clip.min_x)
		{
			srcx += srcxstep * (m_render_clip.min_x - dstx);
			dstw -= m_render_clip.min_x - dstx;
			dstx = m_render_clip.min_x;
		}
		if (dstx + dstw > m_render_clip.max_x)
			dstw = m_render_clip.max_x - dstx + 1;

		/* apply Y clipping */
		if (dsty < m_render_clip.min_y)
		{
			srcy += srcystep * (m_render_clip.min_y - dsty);
			dsth -= m_render_clip.min_y - dsty;
			dsty = m_render_clip.min_y;
		}
		if (dsty + dsth > m_render_clip.max_y)
			dsth = m_render_clip.max_y - dsty + 1;

		/* special case for fills */
		if (srcxstep == 0 && srcystep == 0)
		{
			/* prefetch the pixel */
			uint8_t pixel = m_srcbitmap[(((srcy >> 16) & m_srcbitmap_height_mask) * SRCBITMAP_WIDTH) | ((srcx >> 16) & SRCBITMAP_WIDTH_MASK)];
			pixel = color | (pixel & mask);

			/* loop over rows and columns */
			if (dstw > 0)
			{
				for (int y = 0; y < dsth; y++)
				{
					uint8_t *dst = &m_dstbitmap->pix8(dsty + y,dstx);
					memset(dst, pixel, dstw);
				}
			}
		}

		/* otherwise, standard render */
		else
		{
			/* loop over rows */
			uint32_t cury = srcy;
			for (int y = 0; y < dsth; y++, cury += srcystep)
			{
				const uint8_t *src = &m_srcbitmap[((cury >> 16) & m_srcbitmap_height_mask) * SRCBITMAP_WIDTH];
				uint8_t *dst = &m_dstbitmap->pix8((dsty + y), dstx);

				/* loop over columns */
				for (int x = 0, curx = srcx; x < dstw; x++, curx += srcxstep)
				{
					const uint8_t pixel = src[(curx >> 16) & SRCBITMAP_WIDTH_MASK];
					if (pixel)
						dst[x] = color | (pixel & mask);
				}
			}
		}

		/* advance to the next link */
		offset = entry[7] & 0x1fffffff;
	}
}



/*************************************
 *
 *  Video controller writes
 *
 *************************************/

WRITE32_MEMBER(policetr_state::video_w)
{
	/* 4 offsets */
	switch (offset)
	{
		/* offset 0 specifies the start address of a display list */
		case 0:
			render_display_list(data);
			break;

		/* offset 1 specifies a latch value in the upper 8 bits */
		case 1:
			m_video_latch = data >> 24;
			break;

		/* offset 2 has various meanings based on the latch */
		case 2:
		{
			switch (m_video_latch)
			{
				/* latch 0x04 specifies the source X offset for a source bitmap pixel read */
				case 0x04:
					m_src_xoffs = data >> 16;
					break;

				/* latch 0x14 specifies the source Y offset for a source bitmap pixel read */
				case 0x14:
					m_src_yoffs = data >> 16;
					break;

				/* latch 0x20 specifies the top/left corners of the render cliprect */
				case 0x20:
					m_render_clip.min_y = (data >> 12) & 0xfff;
					m_render_clip.min_x = data & 0xfff;
					break;

				/* latch 0x30 specifies the bottom/right corners of the render cliprect */
				case 0x30:
					m_render_clip.max_y = (data >> 12) & 0xfff;
					m_render_clip.max_x = data & 0xfff;
					break;

				/* latch 0x50 allows a direct write to the destination bitmap */
				case 0x50:
					if (ACCESSING_BITS_24_31 && m_dst_xoffs < DSTBITMAP_WIDTH && m_dst_yoffs < DSTBITMAP_HEIGHT)
						m_dstbitmap->pix8(m_dst_yoffs,m_dst_xoffs) = data >> 24;
					break;

				/* log anything else */
				default:
					logerror("%s: video_w(2) = %08X & %08X with latch %02X\n", machine().describe_context(), data, mem_mask, m_video_latch);
					break;
			}
			break;
		}

		/* offset 3 has various meanings based on the latch */
		case 3:
		{
			switch (m_video_latch)
			{
				/* latch 0x00 is unknown; 0, 1, and 2 get written into the upper 12 bits before rendering */
				case 0x00:
					if (data != 0 && data != (1 << 20) && data != (2 << 20))
						logerror("%s: video_w(3) = %08X & %08X with latch %02X\n", machine().describe_context(), data, mem_mask, m_video_latch);
					break;

				/* latch 0x10 specifies destination bitmap X and Y offsets */
				case 0x10:
					m_dst_yoffs = (data >> 12) & 0xfff;
					m_dst_xoffs = data & 0xfff;
					break;

				/* latch 0x20 is unknown; either 0xef or 0x100 is written every IRQ4 */
				case 0x20:
					if (data != (0x100 << 12) && data != (0xef << 12))
						logerror("%s: video_w(3) = %08X & %08X with latch %02X\n", machine().describe_context(), data, mem_mask, m_video_latch);
					break;

				/* latch 0x40 is unknown; a 0 is written every IRQ4 */
				case 0x40:
					if (data != 0)
						logerror("%s: video_w(3) = %08X & %08X with latch %02X\n", machine().describe_context(), data, mem_mask, m_video_latch);
					break;

				/* latch 0x50 clears IRQ4 */
				case 0x50:
					m_maincpu->set_input_line(INPUT_LINE_IRQ4, CLEAR_LINE);
					break;

				/* latch 0x60 clears IRQ5 */
				case 0x60:
					m_maincpu->set_input_line(INPUT_LINE_IRQ5, CLEAR_LINE);
					break;

				/* log anything else */
				default:
					logerror("%s: video_w(3) = %08X & %08X with latch %02X\n", machine().describe_context(), data, mem_mask, m_video_latch);
					break;
			}
			break;
		}
	}
}



/*************************************
 *
 *  Video controller reads
 *
 *************************************/

READ32_MEMBER(policetr_state::video_r)
{
	int inputval;
	int width = m_screen->width();
	int height = m_screen->height();

	/* the value read is based on the latch */
	switch (m_video_latch)
	{
		/* latch 0x00 is player 1's gun X coordinate */
		case 0x00:
			inputval = ((m_gun_x_io[0]->read() & 0xff) * width) >> 8;
			inputval += 0x50;
			return (inputval << 20) | 0x20000000;

		/* latch 0x01 is player 1's gun Y coordinate */
		case 0x01:
			inputval = ((m_gun_y_io[0]->read() & 0xff) * height) >> 8;
			inputval += 0x17;
			return (inputval << 20);

		/* latch 0x02 is player 2's gun X coordinate */
		case 0x02:
			inputval = ((m_gun_x_io[1]->read() & 0xff) * width) >> 8;
			inputval += 0x50;
			return (inputval << 20) | 0x20000000;

		/* latch 0x03 is player 2's gun Y coordinate */
		case 0x03:
			inputval = ((m_gun_y_io[1]->read() & 0xff) * height) >> 8;
			inputval += 0x17;
			return (inputval << 20);

		/* latch 0x04 is the pixel value in the ROM at the specified address */
		case 0x04:
			return m_srcbitmap[((m_src_yoffs & m_srcbitmap_height_mask) << 12) | (m_src_xoffs & 0xfff)] << 24;

		/* latch 0x50 is read at IRQ 4; the top 2 bits are checked. If they're not 0,
		    they skip the rest of the interrupt processing */
		case 0x50:
			return 0;
	}

	/* log anything else */
	logerror("%s: video_r with latch %02X\n", machine().describe_context(), m_video_latch);
	return 0;
}




/*************************************
 *
 *  Palette access
 *
 *************************************/

WRITE8_MEMBER(policetr_state::palette_offset_w)
{
	m_palette_offset = data;
	m_palette_index = 0;
}


WRITE8_MEMBER(policetr_state::palette_data_w)
{
	m_palette_data[m_palette_index] = data;
	if (++m_palette_index == 3)
	{
		m_palette->set_pen_color(m_palette_offset, rgb_t(m_palette_data[0], m_palette_data[1], m_palette_data[2]));
		m_palette_index = 0;
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t policetr_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const int width = cliprect.width();
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	/* render all the scanlines from the dstbitmap to MAME's bitmap */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const uint8_t *src = &m_dstbitmap->pix8(y,cliprect.min_x);
		uint32_t *dst = &bitmap.pix32(y,cliprect.min_x);
		//draw_scanline8(bitmap, cliprect.min_x, y, width, src, nullptr);
		for (int x = 0; x < width; x++, dst++, src++)
			*dst = palette[*src];
	}

	return 0;
}
