// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
/*****************************************************************************

    SunPlus GCM394-series SoC peripheral emulation (Video)

**********************************************************************/

#include "emu.h"
#include "sunplus_gcm394_video.h"

DEFINE_DEVICE_TYPE(GCM394_VIDEO, gcm394_video_device, "gcm394_video", "GCM394-series System-on-a-Chip (Video)")

#define LOG_IRQS            (1U << 4)
#define LOG_VLINES          (1U << 5)
#define LOG_DMA             (1U << 9)
#define LOG_PPU_READS       (1U << 22)
#define LOG_PPU_WRITES      (1U << 23)
#define LOG_UNKNOWN_PPU     (1U << 24)
#define LOG_PPU             (LOG_PPU_READS | LOG_PPU_WRITES | LOG_UNKNOWN_PPU)
#define LOG_ALL             (LOG_IRQS | LOG_PPU | LOG_VLINES | LOG_DMA )

#define VERBOSE             (0)
#include "logmacro.h"


gcm394_base_video_device::gcm394_base_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
//	, m_scrollram(*this, "scrollram")
	, m_paletteram(*this, "paletteram")
//	, m_spriteram(*this, "spriteram")
	, m_video_irq_cb(*this)
{
}

gcm394_video_device::gcm394_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gcm394_base_video_device(mconfig, GCM394_VIDEO, tag, owner, clock)
{
}

void gcm394_base_video_device::device_start()
{
	for (uint8_t i = 0; i < 32; i++)
	{
		m_rgb5_to_rgb8[i] = (i << 3) | (i >> 2);
	}
	for (uint16_t i = 0; i < 0x8000; i++)
	{
		m_rgb555_to_rgb888[i] = (m_rgb5_to_rgb8[(i >> 10) & 0x1f] << 16) |
								(m_rgb5_to_rgb8[(i >>  5) & 0x1f] <<  8) |
								(m_rgb5_to_rgb8[(i >>  0) & 0x1f] <<  0);
	}

	save_item(NAME(m_video_regs));

	m_video_irq_cb.resolve();

}

void gcm394_base_video_device::device_reset()
{
	memset(m_video_regs, 0, 0x100 * sizeof(uint16_t));
}

/*************************
*     Video Hardware     *
*************************/


template<gcm394_base_video_device::blend_enable_t Blend, gcm394_base_video_device::rowscroll_enable_t RowScroll, gcm394_base_video_device::flipx_t FlipX>
void gcm394_base_video_device::draw(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t bitmap_addr, uint16_t tile, int32_t h, int32_t w, uint8_t bpp, uint32_t yflipmask, uint32_t palette_offset)
{
	address_space &space = m_cpu->space(AS_PROGRAM);

	uint32_t nc_bpp = ((bpp) + 1) << 1;

	palette_offset >>= nc_bpp;
	palette_offset <<= nc_bpp;

	uint32_t bits_per_row = nc_bpp * w / 16;
	uint32_t words_per_tile = bits_per_row * h;
	uint32_t m = bitmap_addr + words_per_tile * tile + bits_per_row * (line ^ yflipmask);
	uint32_t bits = 0;
	uint32_t nbits = 0;
	uint32_t y = line;

	int yy = (yoff + y) & 0x1ff;
	if (yy >= 0x01c0)
		yy -= 0x0200;

	if (yy > 240 || yy < 0)
		return;

	int y_index = yy * 320;

	for (int32_t x = FlipX ? (w - 1) : 0; FlipX ? x >= 0 : x < w; FlipX ? x-- : x++)
	{
		int xx = xoff + x;

		bits <<= nc_bpp;

		if (nbits < nc_bpp)
		{
			uint16_t b = space.read_word(m++ & 0x3fffff);
			b = (b << 8) | (b >> 8);
			bits |= b << (nc_bpp - nbits);
			nbits += 16;
		}
		nbits -= nc_bpp;

		uint32_t pal = palette_offset + (bits >> 16);
		bits &= 0xffff;

		if (RowScroll)
			xx -= 0;// (int16_t)m_scrollram[yy & 0x1ff];

		xx &= 0x01ff;
		if (xx >= 0x01c0)
			xx -= 0x0200;

		if (xx >= 0 && xx < 320)
		{
			int pix_index = xx + y_index;

			uint16_t rgb = m_paletteram[pal];

			if (!(rgb & 0x8000))
			{
				if (Blend)
				{
					/*
					m_screenbuf[pix_index] = (mix_channel((uint8_t)(m_screenbuf[pix_index] >> 16), m_rgb5_to_rgb8[(rgb >> 10) & 0x1f]) << 16) |
											 (mix_channel((uint8_t)(m_screenbuf[pix_index] >>  8), m_rgb5_to_rgb8[(rgb >> 5) & 0x1f]) << 8) |
											 (mix_channel((uint8_t)(m_screenbuf[pix_index] >>  0), m_rgb5_to_rgb8[rgb & 0x1f]));
					*/
					m_screenbuf[pix_index] = m_rgb555_to_rgb888[rgb];
				}
				else
				{
					m_screenbuf[pix_index] = m_rgb555_to_rgb888[rgb];
				}
			}
		}
	}
}

void gcm394_base_video_device::draw_page(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t bitmap_addr, uint16_t *regs)
{
	uint32_t xscroll = regs[0];
	uint32_t yscroll = regs[1];
	uint32_t attr = regs[2];
	uint32_t ctrl = regs[3];
	uint32_t tilemap = regs[4];
	uint32_t palette_map = regs[5];
	address_space &space = m_cpu->space(AS_PROGRAM);

	if (!(ctrl & PAGE_ENABLE_MASK))
	{
		return;
	}

	if (((attr & PAGE_PRIORITY_FLAG_MASK) >> PAGE_PRIORITY_FLAG_SHIFT) != priority)
	{
		return;
	}

	uint32_t tile_h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	uint32_t tile_w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	uint32_t tile_count_x = 512 / tile_w;

	uint32_t bitmap_y = (scanline + yscroll) & 0xff;
	uint32_t y0 = bitmap_y / tile_h;
	uint32_t tile_scanline = bitmap_y % tile_h;
	uint32_t tile_address = tile_count_x * y0;

	for (uint32_t x0 = 0; x0 < tile_count_x; x0++, tile_address++)
	{
		uint32_t yy = ((tile_h * y0 - yscroll + 0x10) & 0xff) - 0x10;
		uint32_t xx = (tile_w * x0 - xscroll) & 0x1ff;
		uint16_t tile = (ctrl & PAGE_WALLPAPER_MASK) ? space.read_word(tilemap) : space.read_word(tilemap + tile_address);
		uint16_t palette = 0;

		if (!tile)
			continue;

		palette = (ctrl & PAGE_WALLPAPER_MASK) ? space.read_word(palette_map) : space.read_word(palette_map + tile_address / 2);
		if (x0 & 1)
			palette >>= 8;

		uint32_t tileattr = attr;
		uint32_t tilectrl = ctrl;
		if ((ctrl & 2) == 0)
		{   // -(1) bld(1) flip(2) pal(4)
			tileattr &= ~0x000c;
			tileattr |= (palette >> 2) & 0x000c;    // flip

			tileattr &= ~0x0f00;
			tileattr |= (palette << 8) & 0x0f00;    // palette

			tilectrl &= ~0x0100;
			tilectrl |= (palette << 2) & 0x0100;    // blend
		}

		const bool blend = (tileattr & 0x4000 || tilectrl & 0x0100);
		const bool row_scroll = (tilectrl & 0x0010);
		const bool flip_x = (tileattr & TILE_X_FLIP);
		const uint32_t yflipmask = tileattr & TILE_Y_FLIP ? tile_h - 1 : 0;
		const uint32_t palette_offset = (tileattr & 0x0f00) >> 4;

		const uint8_t bpp = tileattr & 0x0003;

		if (blend)
		{
			if (row_scroll)
			{
				if (flip_x)
					draw<BlendOn, RowScrollOn, FlipXOn>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset);
				else
					draw<BlendOn, RowScrollOn, FlipXOff>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset);
			}
			else
			{
				if (flip_x)
					draw<BlendOn, RowScrollOff, FlipXOn>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset);
				else
					draw<BlendOn, RowScrollOff, FlipXOff>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset);
			}
		}
		else
		{
			if (row_scroll)
			{
				if (flip_x)
					draw<BlendOff, RowScrollOn, FlipXOn>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset);
				else
					draw<BlendOff, RowScrollOn, FlipXOff>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset);
			}
			else
			{
				if (flip_x)
					draw<BlendOff, RowScrollOff, FlipXOn>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset);
				else
					draw<BlendOff, RowScrollOff, FlipXOff>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset);
			}
		}
	}
}

uint32_t gcm394_base_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	memset(&m_screenbuf[320 * cliprect.min_y], 0, 4 * 320 * ((cliprect.max_y - cliprect.min_y) + 1));

	const uint32_t page1_addr = 0x40 * m_page1_addr;// m_video_regs[0x20];
	const uint32_t page2_addr = 0x40 * m_page2_addr;// m_video_regs[0x21];
	uint16_t *page1_regs = m_video_regs + 0x10;
	uint16_t *page2_regs = m_video_regs + 0x16;

	for (uint32_t scanline = (uint32_t)cliprect.min_y; scanline <= (uint32_t)cliprect.max_y; scanline++)
	{
		for (int i = 0; i < 4; i++)
		{
			draw_page(cliprect, scanline, i, page1_addr, page1_regs);
			draw_page(cliprect, scanline, i, page2_addr, page2_regs);
		}
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *dest = &bitmap.pix32(y, cliprect.min_x);
		uint32_t *src = &m_screenbuf[cliprect.min_x + 320 * y];
		memcpy(dest, src, sizeof(uint32_t) * ((cliprect.max_x - cliprect.min_x) + 1));
	}

	return 0;
}

READ16_MEMBER(gcm394_base_video_device::video_r)
{
	return m_video_regs[offset];
}

WRITE16_MEMBER(gcm394_base_video_device::video_w)
{
	switch (offset)
	{
	case 0x10: // Page 1 X scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 X Scroll = %04x\n", data & 0x01ff);
		m_video_regs[offset] = data & 0x01ff;
		break;

	case 0x11: // Page 1 Y scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Y Scroll = %04x\n", data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x12: // Page 1 Attributes
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Attributes = %04x (Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n", data
			, (data >> 12) & 3, (data >> 8) & 15, 8 << ((data >> 6) & 3), 8 << ((data >> 4) & 3), BIT(data, 3), BIT(data, 2), 2 * ((data & 3) + 1));
		m_video_regs[offset] = data;
		break;

	case 0x13: // Page 1 Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Control = %04x (Blend:%d, HiColor:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n", data
			, BIT(data, 8), BIT(data, 7), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));
		m_video_regs[offset] = data;
		break;

	case 0x14: // Page 1 Tile Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Tile Address = %04x\n", data & 0x1fff);
		m_video_regs[offset] = data;
		break;

	case 0x15: // Page 1 Attribute Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Attribute Address = %04x\n", data & 0x1fff);
		m_video_regs[offset] = data;
		break;

//

	case 0x16: // Page 2 X scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 X Scroll = %04x\n", data & 0x01ff);
		m_video_regs[offset] = data & 0x01ff;
		break;

	case 0x17: // Page 2 Y scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Y Scroll: %04x = %04x\n", 0x2800 | offset, data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x18: // Page 2 Attributes
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Attributes = %04x (Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n", data
			, (data >> 12) & 3, (data >> 8) & 15, 8 << ((data >> 6) & 3), 8 << ((data >> 4) & 3), BIT(data, 3), BIT(data, 2), 2 * ((data & 3) + 1));
		m_video_regs[offset] = data;
		break;

	case 0x19: // Page 2 Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Control = %04x (Blend:%d, HiColor:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n", data
			, BIT(data, 8), BIT(data, 7), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));
		m_video_regs[offset] = data;
		break;

	case 0x1a: // Page 2 Tile Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Tile Address = %04x\n", data & 0x1fff);
		m_video_regs[offset] = data;
		break;

	case 0x1b: // Page 2 Attribute Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Attribute Address = %04x\n", data & 0x1fff);
		m_video_regs[offset] = data;
		break;


//

	case 0x20: // Page 1 Segment Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Segment Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x21: // Page 2 Segment Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Segment Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	default:
		LOGMASKED(LOG_UNKNOWN_PPU, "video_w: Unknown register %04x = %04x\n", 0x2800 + offset, data);
		m_video_regs[offset] = data;
		break;
	}
}

WRITE_LINE_MEMBER(gcm394_base_video_device::vblank)
{
/*
	if (!state)
	{
		VIDEO_IRQ_STATUS &= ~1;
		LOGMASKED(LOG_IRQS, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
		check_video_irq();
		return;
	}

	if (VIDEO_IRQ_ENABLE & 1)
	{
		VIDEO_IRQ_STATUS |= 1;
		LOGMASKED(LOG_IRQS, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
		check_video_irq();
	}
*/
}

void gcm394_base_video_device::check_video_irq()
{
/*
	LOGMASKED(LOG_IRQS, "%ssserting IRQ0 (%04x, %04x)\n", (VIDEO_IRQ_STATUS & VIDEO_IRQ_ENABLE) ? "A" : "Dea", VIDEO_IRQ_STATUS, VIDEO_IRQ_ENABLE);
	m_video_irq_cb((VIDEO_IRQ_STATUS & VIDEO_IRQ_ENABLE) ? ASSERT_LINE : CLEAR_LINE);
*/
}

