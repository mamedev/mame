// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
/*****************************************************************************

    SunPlus GCM394-series SoC peripheral emulation (Video)

**********************************************************************/

#include "emu.h"
#include "sunplus_gcm394_video.h"

DEFINE_DEVICE_TYPE(GCM394_VIDEO, gcm394_video_device, "gcm394_video", "SunPlus GCM394 System-on-a-Chip (Video)")

#define LOG_GCM394_VIDEO_DMA      (1U << 3)
#define LOG_GCM394_TMAP           (1U << 2)
#define LOG_GCM394_VIDEO          (1U << 1)

#define VERBOSE             (LOG_GCM394_VIDEO_DMA | LOG_GCM394_VIDEO)

#include "logmacro.h"


gcm394_base_video_device::gcm394_base_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	//, device_gfx_interface(mconfig, *this, nullptr)
	, device_video_interface(mconfig, *this)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
//	, m_scrollram(*this, "scrollram")
	, m_spriteram(*this, "^spriteram")
	, m_video_irq_cb(*this)
	, m_palette(*this, "palette")
	, m_gfxdecode(*this, "gfxdecode")
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

	m_video_irq_cb.resolve();


	m_gfxregion = memregion(":maincpu")->base();
	m_gfxregionsize = memregion(":maincpu")->bytes();

	int gfxelement = 0;

	if (1)
	{
		gfx_layout obj_layout =
		{
			16,16,
			0,
			4,
			{ STEP4(0,1) },
			{ STEP16(0,4) },
			{ STEP16(0,4 * 16) },
			16 * 16 * 4
		};
		obj_layout.total = m_gfxregionsize / (16 * 16 * 4 / 8);
		m_gfxdecode->set_gfx(gfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, m_gfxregion, 0, 0x10, 0));
		gfxelement++;
	}

	if (1)
	{
		gfx_layout obj_layout =
		{
			32,16,
			0,
			4,
			{ STEP4(0,1) },
			{ STEP32(0,4) },
			{ STEP16(0,4 * 32) },
			16 * 32 * 4
		};
		obj_layout.total = m_gfxregionsize / (16 * 32 * 4 / 8);
		m_gfxdecode->set_gfx(gfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, m_gfxregion, 0, 0x10, 0));
		gfxelement++;
	}

	if (1)
	{
		gfx_layout obj_layout =
		{
			16,32,
			0,
			4,
			{ STEP4(0,1) },
			{ STEP16(0,4) },
			{ STEP32(0,4 * 16) },
			32 * 16 * 4
		};
		obj_layout.total = m_gfxregionsize / (32 * 16 * 4 / 8);
		m_gfxdecode->set_gfx(gfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, m_gfxregion, 0, 0x10, 0));
		gfxelement++;
	}

	if (1)
	{
		gfx_layout obj_layout =
		{
			32,32,
			0,
			4,
			{ STEP4(0,1) },
			{ STEP32(0,4) },
			{ STEP32(0,4 * 32) },
			32 * 32 * 4
		};
		obj_layout.total = m_gfxregionsize / (32 * 32 * 4 / 8);
		m_gfxdecode->set_gfx(gfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, m_gfxregion, 0, 0x10, 0));
		gfxelement++;
	}

	if (1)
	{
		gfx_layout obj_layout =
		{
			8,16,
			0,
			2,
			{ 0,1 },
			{ STEP8(0,2) },
			{ STEP16(0,2 * 8) },
			8 * 16 * 2
		};
		obj_layout.total = m_gfxregionsize / (8 * 16 * 2 / 8);
		m_gfxdecode->set_gfx(gfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, m_gfxregion, 0, 0x40, 0));
		gfxelement++;
	}

	if (1)
	{
		const uint32_t texlayout_xoffset[64] = { STEP64(0,2) };
		const uint32_t texlayout_yoffset[32] = { STEP32(0,2*64) };

		gfx_layout obj_layout =
		{
			64,32,
			0,
			2,
			{ 0,1 },
			EXTENDED_XOFFS,
			EXTENDED_YOFFS,
			32 * 64 * 2,
			texlayout_xoffset,
			texlayout_yoffset
		};
		obj_layout.total = m_gfxregionsize / (16 * 32 * 2 / 8);
		m_gfxdecode->set_gfx(gfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, m_gfxregion, 0, 0x40, 0));
		gfxelement++;
	}
	save_item(NAME(m_spriteextra));
}

void gcm394_base_video_device::device_reset()
{
	for (int i = 0; i < 6; i++)
	{
		m_tmap0_regs[i] = 0x0000;
		m_tmap1_regs[i] = 0x0000;
	}

	for (int i=0;i<0x100;i++)
		m_spriteextra[i] = 0x0000;

	for (int i=0;i<0x100;i++)
		m_paletteram[i] = machine().rand()&0x7fff;


	m_707f = 0x0000;
	m_703a = 0x0000;
	m_7062 = 0x0000;
	m_7063 = 0x0000;
	
	m_702a = 0x0000;
	m_7030 = 0x0000;
	m_703c = 0x0000;


	m_7080 = 0x0000;
	m_7081 = 0x0000;
	m_7082 = 0x0000;
	m_7083 = 0x0000;
	m_7084 = 0x0000;
	m_7085 = 0x0000;
	m_7086 = 0x0000;
	m_7087 = 0x0000;
	m_7088 = 0x0000;

	m_videodma_bank = 0x0000;
	m_videodma_size = 0x0000;
	m_videodma_dest = 0x0000;
	m_videodma_source = 0x0000;

	m_video_irq_status = 0x0000;

}

/*************************
*     Video Hardware     *
*************************/

inline uint16_t gcm394_base_video_device::read_data(uint32_t offset)
{
	uint16_t b = m_gfxregion[(offset * 2) & (m_gfxregionsize - 1)] | (m_gfxregion[(offset * 2 + 1) & (m_gfxregionsize - 1)] << 8);
	return b;
}


template<gcm394_base_video_device::blend_enable_t Blend, gcm394_base_video_device::rowscroll_enable_t RowScroll, gcm394_base_video_device::flipx_t FlipX>
void gcm394_base_video_device::draw(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t bitmap_addr, uint32_t tile, int32_t h, int32_t w, uint8_t bpp, uint32_t yflipmask, uint32_t palette_offset)
{
	uint32_t nc_bpp = ((bpp) + 1) << 1;

	palette_offset >>= nc_bpp;
	palette_offset <<= nc_bpp;

	uint32_t bits_per_row = nc_bpp * w / 16;
	//uint32_t words_per_tile = bits_per_row * h;

	uint32_t words_per_tile = 8; // seems to be correct for sprites regardless of size / bpp

	uint32_t m = bitmap_addr + words_per_tile * tile + bits_per_row * (line ^ yflipmask);

	m += (0x300000 / 2);

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
			uint16_t b = read_data((m++ & 0x3fffff));
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
		uint32_t palette_offset = (tileattr & 0x0f00) >> 4;

		palette_offset |= 0x0900;

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


void gcm394_base_video_device::draw_sprite(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t base_addr)
{
	uint32_t bitmap_addr = 0;// 0x40 * m_video_regs[0x22];
	uint32_t tile = m_spriteram[base_addr + 0];
	int16_t x = m_spriteram[base_addr + 1];
	int16_t y = m_spriteram[base_addr + 2];
	uint16_t attr = m_spriteram[base_addr + 3];

	tile |= m_spriteextra[base_addr / 4] << 16;

	if (!tile)
	{
		return;
	}

	if (((attr & PAGE_PRIORITY_FLAG_MASK) >> PAGE_PRIORITY_FLAG_SHIFT) != priority)
	{
		return;
	}

	const uint32_t h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	const uint32_t w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	/*
	if (!(m_video_regs[0x42] & SPRITE_COORD_TL_MASK))
	{
		x = (160 + x) - w / 2;
		y = (120 - y) - (h / 2) + 8;
	}
	*/

	x &= 0x01ff;
	y &= 0x01ff;

	uint32_t tile_line = ((scanline - y) + 0x200) % h;
	int16_t test_y = (y + tile_line) & 0x1ff;
	if (test_y >= 0x01c0)
		test_y -= 0x0200;

	if (test_y != scanline)
	{
		return;
	}

	bool blend = (attr & 0x4000);
	bool flip_x = (attr & TILE_X_FLIP);
	const uint8_t bpp = attr & 0x0003;
	const uint32_t yflipmask = attr & TILE_Y_FLIP ? h - 1 : 0;
	uint32_t palette_offset = (attr & 0x0f00) >> 4;

	palette_offset |= 0x0d00;

	if (blend)
	{
		if (flip_x)
			draw<BlendOn, RowScrollOff, FlipXOn>(cliprect, tile_line, x, y, bitmap_addr, tile, h, w, bpp, yflipmask, palette_offset);
		else
			draw<BlendOn, RowScrollOff, FlipXOff>(cliprect, tile_line, x, y, bitmap_addr, tile, h, w, bpp, yflipmask, palette_offset);
	}
	else
	{
		if (flip_x)
			draw<BlendOff, RowScrollOff, FlipXOn>(cliprect, tile_line, x, y, bitmap_addr, tile, h, w, bpp, yflipmask, palette_offset);
		else
			draw<BlendOff, RowScrollOff, FlipXOff>(cliprect, tile_line, x, y, bitmap_addr, tile, h, w, bpp, yflipmask, palette_offset);
	}
}

void gcm394_base_video_device::draw_sprites(const rectangle &cliprect, uint32_t scanline, int priority)
{
	for (uint32_t n = 0; n < 0x100; n++)
	{
		draw_sprite(cliprect, scanline, priority, 4 * n);
	}
}

uint32_t gcm394_base_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	memset(&m_screenbuf[320 * cliprect.min_y], 0, 4 * 320 * ((cliprect.max_y - cliprect.min_y) + 1));

	const uint32_t page1_addr = 0x40 * m_page1_addr;
	const uint32_t page2_addr = 0x40 * m_page2_addr;
	uint16_t* page1_regs = m_tmap0_regs;
	uint16_t* page2_regs = m_tmap1_regs;

	for (uint32_t scanline = (uint32_t)cliprect.min_y; scanline <= (uint32_t)cliprect.max_y; scanline++)
	{
		for (int i = 0; i < 4; i++)
		{
			if (1)
			{
				draw_page(cliprect, scanline, i, page1_addr, page1_regs);
				draw_page(cliprect, scanline, i, page2_addr, page2_regs);
			}
			draw_sprites(cliprect, scanline, i);
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


void gcm394_base_video_device::write_tmap_regs(int tmap, uint16_t* regs, int offset, uint16_t data)
{
	switch (offset)
	{
	case 0x0: // Page X scroll
		LOGMASKED(LOG_GCM394_TMAP, "write_tmap_regs: Page %d X Scroll = %04x\n", tmap, data & 0x01ff);
		regs[offset] = data & 0x01ff;
		break;

	case 0x1: // Page Y scroll
		LOGMASKED(LOG_GCM394_TMAP, "write_tmap_regs: Page %d Y Scroll = %04x\n", tmap, data & 0x00ff);
		regs[offset] = data & 0x00ff;
		break;

	case 0x2: // Page Attributes
		LOGMASKED(LOG_GCM394_TMAP, "write_tmap_regs: Page %d Attributes = %04x (Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n", tmap, data
			, (data >> 12) & 3, (data >> 8) & 15, 8 << ((data >> 6) & 3), 8 << ((data >> 4) & 3), BIT(data, 3), BIT(data, 2), 2 * ((data & 3) + 1));
		regs[offset] = data;
		break;

	case 0x3: // Page Control
		LOGMASKED(LOG_GCM394_TMAP, "write_tmap_regs: Page %d Control = %04x (Blend:%d, HiColor:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n", tmap, data
			, BIT(data, 8), BIT(data, 7), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));
		regs[offset] = data;
		break;

	case 0x4: // Page Tile Address
		LOGMASKED(LOG_GCM394_TMAP, "write_tmap_regs: Page %d Tile Address = %04x\n", tmap, data & 0x1fff);
		regs[offset] = data;
		break;

	case 0x5: // Page Attribute write_tmap_regs
		LOGMASKED(LOG_GCM394_TMAP, "write_tmap_regs: Page %d Attribute Address = %04x\n", tmap, data & 0x1fff);
		regs[offset] = data;
		break;
	}
}

// **************************************** TILEMAP 0 *************************************************

READ16_MEMBER(gcm394_base_video_device::tmap0_regs_r) { return m_tmap0_regs[offset]; }

WRITE16_MEMBER(gcm394_base_video_device::tmap0_regs_w)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap0_regs_w %01x %04x\n", machine().describe_context(), offset, data);
	write_tmap_regs(0, m_tmap0_regs, offset, data);
}

WRITE16_MEMBER(gcm394_base_video_device::tmap0_unk0_w)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap0_unk0_w %04x\n", machine().describe_context(), data);
	m_page1_addr = data;
}

WRITE16_MEMBER(gcm394_base_video_device::tmap0_unk1_w)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap0_unk1_w %04x\n", machine().describe_context(), data);
}

// **************************************** TILEMAP 1 *************************************************

READ16_MEMBER(gcm394_base_video_device::tmap1_regs_r) { return m_tmap1_regs[offset]; }

WRITE16_MEMBER(gcm394_base_video_device::tmap1_regs_w)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap1_regs_w %01x %04x\n", machine().describe_context(), offset, data);
	write_tmap_regs(1, m_tmap1_regs, offset, data);
}

WRITE16_MEMBER(gcm394_base_video_device::tmap1_unk0_w)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap1_unk0_w %04x\n", machine().describe_context(), data);
	m_page2_addr = data;
}

WRITE16_MEMBER(gcm394_base_video_device::tmap1_unk1_w)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap1_unk1_w %04x\n", machine().describe_context(), data);
}

// **************************************** unknown video device 0 (another tilemap? sprite layer?) *************************************************

WRITE16_MEMBER(gcm394_base_video_device::unknown_video_device0_regs_w)
{
	// offsets 0,1,4,5,6,7 used in main IRQ code
	// offsets 2,3 only cleared on startup

	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::unknown_video_device0_regs_w %01x %04x\n", machine().describe_context(), offset, data);
}

WRITE16_MEMBER(gcm394_base_video_device::unknown_video_device0_unk0_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::unknown_video_device0_unk0_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(gcm394_base_video_device::unknown_video_device0_unk1_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::unknown_video_device0_unk1_w %04x\n", machine().describe_context(), data);
}

// **************************************** unknown video device 1 (another tilemap? sprite layer?) *************************************************

WRITE16_MEMBER(gcm394_base_video_device::unknown_video_device1_regs_w)
{
	// offsets 0,1,4,5,6,7 used in main IRQ code
	// offsets 2,3 only cleared on startup

	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::unknown_video_device1_regs_w %01x %04x\n", machine().describe_context(), offset, data);
}

WRITE16_MEMBER(gcm394_base_video_device::unknown_video_device1_unk0_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::unknown_video_device1_unk0_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(gcm394_base_video_device::unknown_video_device1_unk1_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::unknown_video_device1_unk1_w %04x\n", machine().describe_context(), data);
}

// **************************************** unknown video device 2 (sprite control?) *************************************************

WRITE16_MEMBER(gcm394_base_video_device::unknown_video_device2_unk0_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::unknown_video_device2_unk0_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(gcm394_base_video_device::unknown_video_device2_unk1_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::unknown_video_device2_unk1_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(gcm394_base_video_device::unknown_video_device2_unk2_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::unknown_video_device2_unk2_w %04x\n", machine().describe_context(), data);
}

// **************************************** video DMA device *************************************************

WRITE16_MEMBER(gcm394_base_video_device::video_dma_source_w)
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_source_w %04x\n", machine().describe_context(), data);
	m_videodma_source = data;
}

WRITE16_MEMBER(gcm394_base_video_device::video_dma_dest_w)
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_dest_w %04x\n", machine().describe_context(), data);
	m_videodma_dest = data;
}

READ16_MEMBER(gcm394_base_video_device::video_dma_size_r)
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_size_r\n", machine().describe_context());
	return 0x0000;
}

WRITE16_MEMBER(gcm394_base_video_device::video_dma_size_w)
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_size_w %04x\n", machine().describe_context(), data);
	m_videodma_size = data;

	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s: doing sprite / video DMA source %04x dest %04x size %04x bank %04x\n", machine().describe_context(), m_videodma_source, m_videodma_dest, m_videodma_size, m_videodma_bank );


	if (m_videodma_dest == 0x7400)
	{
		if (m_videodma_bank == 0x0001) // transfers an additional word for each sprite with this bit set
		{
			m_videodma_size &= 0xff;

			for (int i = 0; i <= m_videodma_size; i++)
			{
				uint16_t dat = space.read_word(m_videodma_source+i);
				m_spriteextra[i] = dat;
			}

		}
		else if (m_videodma_bank == 0x0000)
		{
			m_videodma_size &= 0x3ff;

			for (int i = 0; i <= m_videodma_size; i++)
			{
				uint16_t dat = space.read_word(m_videodma_source+i);
				m_spriteram[i] = dat;
			}

		}
		else
		{
			logerror("unhandled: m_videodma_bank is %04x\n", m_videodma_bank);
		}
	}
	else
	{
		logerror("unhandled: m_videodma_dest is %04x\n", m_videodma_dest);
	}

	m_videodma_size = 0x0000;

}

WRITE16_MEMBER(gcm394_base_video_device::video_dma_unk_w)
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_unk_w %04x\n", machine().describe_context(), data);
	m_videodma_bank = data;
}




READ16_MEMBER(gcm394_base_video_device::video_707f_r) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_707f_r\n", machine().describe_context()); return m_707f; }
WRITE16_MEMBER(gcm394_base_video_device::video_707f_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_707f_w %04x\n", machine().describe_context(), data); m_707f = data; }

READ16_MEMBER(gcm394_base_video_device::video_703a_r) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_703a_r\n", machine().describe_context()); return m_703a; } // something to do with palette access, maybe bank?
WRITE16_MEMBER(gcm394_base_video_device::video_703a_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_703a_w %04x\n", machine().describe_context(), data); m_703a = data; }

READ16_MEMBER(gcm394_base_video_device::video_7062_r) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7062_r\n", machine().describe_context()); return m_7062; }
WRITE16_MEMBER(gcm394_base_video_device::video_7062_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7062_w %04x\n", machine().describe_context(), data); m_7062 = data; }

WRITE16_MEMBER(gcm394_base_video_device::video_7063_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7063_w %04x\n", machine().describe_context(), data); m_7063 = data; }

WRITE16_MEMBER(gcm394_base_video_device::video_702a_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_702a_w %04x\n", machine().describe_context(), data); m_702a = data; }

// read in IRQ
READ16_MEMBER(gcm394_base_video_device::video_7030_r) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7030_r\n", machine().describe_context()); return m_7030; }
WRITE16_MEMBER(gcm394_base_video_device::video_7030_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7030_w %04x\n", machine().describe_context(), data); m_7030 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_703c_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_703c_w %04x\n", machine().describe_context(), data); m_703c = data; }

WRITE16_MEMBER(gcm394_base_video_device::video_7080_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7080_w %04x\n", machine().describe_context(), data); m_7080 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7081_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7081_w %04x\n", machine().describe_context(), data); m_7081 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7082_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7082_w %04x\n", machine().describe_context(), data); m_7082 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7083_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7083_w %04x\n", machine().describe_context(), data); m_7083 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7084_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7084_w %04x\n", machine().describe_context(), data); m_7084 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7085_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7085_w %04x\n", machine().describe_context(), data); m_7085 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7086_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7086_w %04x\n", machine().describe_context(), data); m_7086 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7087_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7087_w %04x\n", machine().describe_context(), data); m_7087 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7088_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7088_w %04x\n", machine().describe_context(), data); m_7088 = data; }

READ16_MEMBER(gcm394_base_video_device::video_7083_r) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7083_r\n", machine().describe_context()); return m_7083; }

WRITE16_MEMBER(gcm394_base_video_device::palette_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::palette_w %04x : %04x (value of 0x703a is %04x)\n", machine().describe_context().c_str(), offset, data, m_703a); 

	if (m_703a & 0xfff0)
	{
		fatalerror("palette writes with m_703a %04x\n", m_703a);
	}
	else
	{
		offset |= (m_703a & 0x000f) << 8;

		m_paletteram[offset] = data;

		uint32_t pal = m_rgb555_to_rgb888[data & 0x7fff];
		int r = (pal >> 16) & 0xff;
		int g = (pal >> 8) & 0xff;
		int b = (pal >> 0) & 0xff;

		m_palette->set_pen_color(offset, rgb_t(r, g, b));
	}
}

READ16_MEMBER(gcm394_base_video_device::palette_r)
{
	if (m_703a & 0xfff0)
	{
		fatalerror("palette read with m_703a %04x\n", m_703a);
	}
	else
	{
		offset |= (m_703a & 0x000f) << 8;
		return m_paletteram[offset];
	}
}


void gcm394_base_video_device::check_video_irq()
{
	m_video_irq_cb((m_video_irq_status & 1) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(gcm394_base_video_device::vblank)
{
	int i = 0x0001;

	if (!state)
	{
		m_video_irq_status &= ~i;
		check_video_irq();
		return;
	}

	//if (m_video_irq_enable & 1)
	{
		m_video_irq_status |= i;
		check_video_irq();
	}
}

static GFXDECODE_START( gfx )
GFXDECODE_END

void gcm394_base_video_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 256*0x10);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx);

}


