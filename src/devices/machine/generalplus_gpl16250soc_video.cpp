// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
/*****************************************************************************

    SunPlus GCM394-series SoC peripheral emulation (Video)

    This is very similar to spg2xx but with additional features, layers and modes

**********************************************************************/


#include "emu.h"
#include "generalplus_gpl16250soc_video.h"

DEFINE_DEVICE_TYPE(GCM394_VIDEO, gcm394_video_device, "gcm394_video", "GeneralPlus GPL16250 System-on-a-Chip (Video)")

#define LOG_GCM394_VIDEO_PALETTE  (1U << 5)
#define LOG_GCM394_VIDEO_DMA      (1U << 4)
#define LOG_GCM394_TMAP_EXTRA     (1U << 3)
#define LOG_GCM394_TMAP           (1U << 2)
#define LOG_GCM394_VIDEO          (1U << 1)

#define VERBOSE             (LOG_GCM394_TMAP | LOG_GCM394_VIDEO_DMA | LOG_GCM394_VIDEO |LOG_GCM394_VIDEO_PALETTE)

#include "logmacro.h"


gcm394_base_video_device::gcm394_base_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_video_irq_cb(*this),
	m_palette(*this, "palette"),
	m_gfxdecode(*this, "gfxdecode"),
	m_space_read_cb(*this, 0),
	m_rowscroll(*this, "^rowscroll"),
	m_rowzoom(*this, "^rowzoom"),
	m_alt_extrasprite_hack(0),
	m_alt_tile_addressing(0),
	m_use_legacy_mode(false),
	m_disallow_resolution_control(false),
	m_renderer(*this, "renderer")
{
}

gcm394_video_device::gcm394_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gcm394_base_video_device(mconfig, GCM394_VIDEO, tag, owner, clock)
{
}

void gcm394_base_video_device::decodegfx(const char* tag)
{
	if (!memregion(tag))
		return;

	uint8_t* gfxregion = memregion(tag)->base();
	int gfxregionsize = memregion(tag)->bytes();

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
		obj_layout.total = gfxregionsize / (16 * 16 * 4 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x10 * 0x10, 0));
		m_maxgfxelement++;
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
		obj_layout.total = gfxregionsize / (16 * 32 * 4 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x10 * 0x10, 0));
		m_maxgfxelement++;
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
		obj_layout.total = gfxregionsize / (32 * 16 * 4 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x10 * 0x10, 0));
		m_maxgfxelement++;
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
		obj_layout.total = gfxregionsize / (32 * 32 * 4 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x10 * 0x10, 0));
		m_maxgfxelement++;
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
		obj_layout.total = gfxregionsize / (8 * 16 * 2 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x40 * 0x10, 0));
		m_maxgfxelement++;
	}

	if (1)
	{
		const uint32_t texlayout_xoffset[64] = { STEP64(0,2) };
		const uint32_t texlayout_yoffset[32] = { STEP32(0,2 * 64) };

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
		obj_layout.total = gfxregionsize / (16 * 32 * 2 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x40 * 0x10, 0));
		m_maxgfxelement++;
	}

	if (1)
	{
		gfx_layout obj_layout =
		{
			32,32,
			0,
			8,
			{ STEP8(0,1) },
			{ STEP32(0,8) },
			{ STEP32(0,8 * 32) },
			32 * 32 * 8
		};
		obj_layout.total = gfxregionsize / (32 * 32 * 8 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x10, 0));
		m_maxgfxelement++;
	}

	if (1)
	{
		gfx_layout obj_layout =
		{
			32,32,
			0,
			6,
			{ 0,1,2,3,4,5 },
			{ STEP32(0,6) },
			{ STEP32(0,6 * 32) },
			32 * 32 * 6
		};
		obj_layout.total = gfxregionsize / (32 * 32 * 6 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x40, 0));
		m_maxgfxelement++;
	}
}

void gcm394_base_video_device::device_start()
{
	m_maxgfxelement = 0;

	// debug helper only
	if (memregion(":maincpu"))
		decodegfx(":maincpu");

	m_screenpos_timer = timer_alloc(FUNC(gcm394_base_video_device::screen_pos_reached), this);
	m_screenpos_timer->adjust(attotime::never);

	save_item(NAME(m_page0_addr_lsb));
	save_item(NAME(m_page0_addr_msb));
	save_item(NAME(m_page1_addr_lsb));
	save_item(NAME(m_page1_addr_msb));
	save_item(NAME(m_707e_spritebank));
	save_item(NAME(m_videodma_size));
	save_item(NAME(m_videodma_dest));
	save_item(NAME(m_videodma_source));
	save_item(NAME(m_tmap0_regs));
	save_item(NAME(m_tmap1_regs));
	save_item(NAME(m_tmap2_regs));
	save_item(NAME(m_tmap3_regs));
	save_item(NAME(m_tmap0_scroll));
	save_item(NAME(m_tmap1_scroll));
	save_item(NAME(m_tmap2_scroll));
	save_item(NAME(m_tmap3_scroll));
	save_item(NAME(m_707f));
	save_item(NAME(m_703a_palettebank));
	save_item(NAME(m_video_irq_enable));
	save_item(NAME(m_video_irq_status));
	save_item(NAME(m_702a));
	save_item(NAME(m_7030_brightness));
	save_item(NAME(m_xirqpos));
	save_item(NAME(m_yirqpos));
	save_item(NAME(m_703c_tvcontrol1));
	save_item(NAME(m_7042_sprite));
	save_item(NAME(m_7080));
	save_item(NAME(m_7081));
	save_item(NAME(m_7082));
	save_item(NAME(m_7083));
	save_item(NAME(m_7084));
	save_item(NAME(m_7085));
	save_item(NAME(m_7086));
	save_item(NAME(m_7087));
	save_item(NAME(m_7088));
	save_item(NAME(m_sprite_7022_gfxbase_lsb));
	save_item(NAME(m_sprite_702d_gfxbase_msb));
	save_item(NAME(m_page2_addr_lsb));
	save_item(NAME(m_page2_addr_msb));
	save_item(NAME(m_page3_addr_lsb));
	save_item(NAME(m_page3_addr_msb));
	save_item(NAME(m_spriteram));
	save_item(NAME(m_paletteram));
	save_item(NAME(m_maxgfxelement));
	save_item(NAME(m_alt_tile_addressing));
}

void gcm394_base_video_device::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_tmap0_regs[i] = 0x0000;
		m_tmap1_regs[i] = 0x0000;
		m_tmap2_regs[i] = 0x0000;
		m_tmap3_regs[i] = 0x0000;
		m_tmap2_scroll[i] = 0x0000;
		m_tmap3_scroll[i] = 0x0000;
	}

	for (int i = 0; i < 2; i++)
	{
		m_tmap0_scroll[i] = 0x0000;
		m_tmap1_scroll[i] = 0x0000;
	}

	for (int i = 0; i < 0x400*2; i++)
	{
		m_spriteram[i] = 0x0000;
	}

	for (int i=0;i<0x100 * 0x10;i++)
		m_paletteram[i] = machine().rand()&0x7fff;

	m_707f = 0x0000;
	m_703a_palettebank = 0x0000;
	m_video_irq_enable = 0x0000;
	m_video_irq_status = 0x0000;

	m_702a = 0x0000;
	m_7030_brightness = 0x0000;
	m_xirqpos = 0x0000;
	m_yirqpos = 0x0000;
	m_703c_tvcontrol1 = 0x0000;

	m_7042_sprite = 0x0000;

	m_7080 = 0x0000;
	m_7081 = 0x0000;
	m_7082 = 0x0000;
	m_7083 = 0x0000;
	m_7084 = 0x0000;
	m_7085 = 0x0000;
	m_7086 = 0x0000;
	m_7087 = 0x0000;
	m_7088 = 0x0000;

	m_707e_spritebank = 0x0000;
	m_videodma_size = 0x0000;
	m_videodma_dest = 0x0000;
	m_videodma_source = 0x0000;

	m_sprite_7022_gfxbase_lsb = 0;
	m_sprite_702d_gfxbase_msb = 0;
	m_page2_addr_lsb = 0;
	m_page2_addr_msb = 0;
	m_page3_addr_lsb = 0;
	m_page3_addr_msb = 0;

	m_renderer->set_video_spaces(m_cpuspace, m_cs_space, m_csbase);
}

uint32_t gcm394_base_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// For jak_car2 and jak_gtg the palette entry for 'magenta' in the test mode is intentionally set to a transparent black pen
	// (it is stored in the palette table in ROM that way, and copied directly) so the only way for the magenta entries on the screen
	// to be correctly displayed is if there is a magenta BG pen to fall through to (or for another palette write to change the palette
	// that is copied, but this does not appear to be the case).  How the bg pen is set is unknown, it is not a regular palette entry.
	// The 'bitmap test mode' in jak_car2 requires this to be black instead.

	// jak_s500 briely sets pen 0 of the layer to magenta, but then ends up erasing it

	uint32_t page0_addr = (m_page0_addr_msb << 16) | m_page0_addr_lsb;
	uint32_t page1_addr = (m_page1_addr_msb << 16) | m_page1_addr_lsb;
	uint32_t page2_addr = (m_page2_addr_msb << 16) | m_page2_addr_lsb;
	uint32_t page3_addr = (m_page3_addr_msb << 16) | m_page3_addr_lsb;

	if (m_use_legacy_mode)
	{
		page0_addr = page0_addr * 0x40;
		page1_addr = page1_addr * 0x40;
		page2_addr = page2_addr * 0x40;
		page3_addr = page3_addr * 0x40;
	}


	if (0)
	{
		uint16_t attr0 = m_tmap0_regs[0];
		uint16_t attr1 = m_tmap1_regs[0];
		uint16_t attr2 = m_tmap2_regs[0];
		uint16_t attr3 = m_tmap3_regs[0];
		uint16_t ctrl0 = m_tmap0_regs[1];
		uint16_t ctrl1 = m_tmap1_regs[1];
		uint16_t ctrl2 = m_tmap2_regs[1];
		uint16_t ctrl3 = m_tmap3_regs[1];

		popmessage(
			"p0ct u:%02x Bl:%d HC:%d Ycmp:%d Hcmp:%d RS:%d E:%d WP:%d Rg:%d Bm:%d gfxadr: %08x t:%04x p:%04x\n"
			"p1ct u:%02x Bl:%d HC:%d Ycmp:%d Hcmp:%d RS:%d E:%d WP:%d Rg:%d Bm:%d gfxadr: %08x t:%04x p:%04x\n"
			"p2ct u:%02x Bl:%d HC:%d Ycmp:%d Hcmp:%d RS:%d E:%d WP:%d Rg:%d Bm:%d gfxadr: %08x t:%04x p:%04x\n"
			"p3ct u:%02x Bl:%d HC:%d Ycmp:%d Hcmp:%d RS:%d E:%d WP:%d Rg:%d Bm:%d gfxadr: %08x t:%04x p:%04x\n"
			"p0attr dw:%01x dh:%01x Z:%d P:%d V:%d H:%d FY:%d FX:%d D:%d xs: %04x ys %04x\n"
			"p1attr dw:%01x dh:%01x Z:%d P:%d V:%d H:%d FY:%d FX:%d D:%d xs: %04x ys %04x\n"
			"p2attr dw:%01x dh:%01x Z:%d P:%d V:%d H:%d FY:%d FX:%d D:%d xs: %04x ys %04x\n"
			"p3attr dw:%01x dh:%01x Z:%d P:%d V:%d H:%d FY:%d FX:%d D:%d xs: %04x ys %04x\n"
			"palbank %04x 707e: %04x 707f: %04x tvc703c: %04x spr7042: %04x\n",
			(ctrl0 & 0xfe00) >> 9, BIT(ctrl0, 8), BIT(ctrl0, 7), BIT(ctrl0, 6), BIT(ctrl0, 5), BIT(ctrl0, 4), BIT(ctrl0, 3), BIT(ctrl0, 2), BIT(ctrl0, 1), BIT(ctrl0, 0), page0_addr, m_tmap0_regs[2], m_tmap0_regs[3],
			(ctrl1 & 0xfe00) >> 9, BIT(ctrl1, 8), BIT(ctrl1, 7), BIT(ctrl1, 6), BIT(ctrl1, 5), BIT(ctrl1, 4), BIT(ctrl1, 3), BIT(ctrl1, 2), BIT(ctrl1, 1), BIT(ctrl1, 0), page1_addr, m_tmap1_regs[2], m_tmap1_regs[3],
			(ctrl2 & 0xfe00) >> 9, BIT(ctrl2, 8), BIT(ctrl2, 7), BIT(ctrl2, 6), BIT(ctrl2, 5), BIT(ctrl2, 4), BIT(ctrl2, 3), BIT(ctrl2, 2), BIT(ctrl2, 1), BIT(ctrl2, 0), page2_addr, m_tmap2_regs[2], m_tmap2_regs[3],
			(ctrl3 & 0xfe00) >> 9, BIT(ctrl3, 8), BIT(ctrl3, 7), BIT(ctrl3, 6), BIT(ctrl3, 5), BIT(ctrl3, 4), BIT(ctrl3, 3), BIT(ctrl3, 2), BIT(ctrl3, 1), BIT(ctrl3, 0), page3_addr, m_tmap3_regs[2], m_tmap3_regs[3],
			BIT(attr0, 15), BIT(attr0, 14), (attr0 >> 12) & 3, (attr0 >> 8) & 15, 8 << ((attr0 >> 6) & 3), 8 << ((attr0 >> 4) & 3), BIT(attr0, 3), BIT(attr0, 2), 2 * ((attr0 & 3) + 1), m_tmap0_scroll[0], m_tmap0_scroll[1],
			BIT(attr1, 15), BIT(attr1, 14), (attr1 >> 12) & 3, (attr1 >> 8) & 15, 8 << ((attr1 >> 6) & 3), 8 << ((attr1 >> 4) & 3), BIT(attr1, 3), BIT(attr1, 2), 2 * ((attr1 & 3) + 1), m_tmap1_scroll[0], m_tmap1_scroll[1],
			BIT(attr2, 15), BIT(attr2, 14), (attr2 >> 12) & 3, (attr2 >> 8) & 15, 8 << ((attr2 >> 6) & 3), 8 << ((attr2 >> 4) & 3), BIT(attr2, 3), BIT(attr2, 2), 2 * ((attr2 & 3) + 1), m_tmap2_scroll[0], m_tmap2_scroll[1],
			BIT(attr3, 15), BIT(attr3, 14), (attr3 >> 12) & 3, (attr3 >> 8) & 15, 8 << ((attr3 >> 6) & 3), 8 << ((attr3 >> 4) & 3), BIT(attr3, 3), BIT(attr3, 2), 2 * ((attr3 & 3) + 1), m_tmap3_scroll[0], m_tmap3_scroll[1],
			m_703a_palettebank, m_707e_spritebank, m_707f, m_703c_tvcontrol1, m_7042_sprite
		);
	}

	//const uint16_t bgcol = 0x7c1f; // magenta
//  const uint16_t bgcol = 0x0000; // black
	bool highres = false;
	if (!m_disallow_resolution_control)
	{
		if (m_707f & 0x0010)
		{
			highres = true;
			m_screen->set_visible_area(0, 640 - 1, 0, 480 - 1);
		}
		else
		{
			highres = false;
			m_screen->set_visible_area(0, 320 - 1, 0, 240 - 1);
		}
	}

	address_space &mem = m_cpu->space(AS_PROGRAM);


	uint32_t sprites_addr = (m_sprite_702d_gfxbase_msb << 16) | m_sprite_7022_gfxbase_lsb;

	if (m_use_legacy_mode)
		sprites_addr *= 0x40;

	for (uint32_t scanline = (uint32_t)cliprect.min_y; scanline <= (uint32_t)cliprect.max_y; scanline++)
	{
		m_renderer->new_line(cliprect);

		for (int i = 0; i < 4; i++)
		{
			m_renderer->draw_page(true, true, m_alt_tile_addressing ? false : true, m_703a_palettebank, cliprect, scanline, i, page0_addr, m_tmap0_scroll, m_tmap0_regs, mem, m_paletteram, m_rowscroll, 0);
			m_renderer->draw_page(true, true, m_alt_tile_addressing ? false : true, m_703a_palettebank, cliprect, scanline, i, page1_addr, m_tmap1_scroll, m_tmap1_regs, mem, m_paletteram, m_rowscroll, 1);
			m_renderer->draw_page(true, true, m_alt_tile_addressing ? false : true, m_703a_palettebank, cliprect, scanline, i, page2_addr, m_tmap2_scroll, m_tmap2_regs, mem, m_paletteram, m_rowscroll, 2);
			m_renderer->draw_page(true, true, m_alt_tile_addressing ? false : true, m_703a_palettebank, cliprect, scanline, i, page3_addr, m_tmap3_scroll, m_tmap3_regs, mem, m_paletteram, m_rowscroll, 3);

			m_renderer->draw_sprites(true, m_use_legacy_mode ? 2 : 1, m_alt_extrasprite_hack ? true : false, m_703a_palettebank, highres, cliprect, scanline, i, sprites_addr, mem, m_paletteram, m_spriteram, -1);
		}

		m_renderer->apply_saturation_and_fade(bitmap, cliprect, scanline);
	}

	return 0;
}


void gcm394_base_video_device::write_tmap_scroll(int tmap, uint16_t* regs, int offset, uint16_t data)
{
	switch (offset)
	{
	case 0x0: // Page X scroll
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d X Scroll = %04x\n", machine().describe_context(), tmap, data);
		regs[offset] = data;
		break;

	case 0x1: // Page Y scroll
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d Y Scroll = %04x\n", machine().describe_context(), tmap, data);
		regs[offset] = data;
		break;
	}
}

void gcm394_base_video_device::write_tmap_regs(int tmap, uint16_t* regs, int offset, uint16_t data)
{
	switch (offset)
	{
	case 0x0: // Page Attributes
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d Attributes = %04x (unk %01x: Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n",  machine().describe_context(), tmap, data,
			(data & 0xc000) >> 14, (data >> 12) & 3, (data >> 8) & 15, 8 << ((data >> 6) & 3), 8 << ((data >> 4) & 3), BIT(data, 3), BIT(data, 2), 2 * ((data & 3) + 1));
		regs[offset] = data;
		break;

	case 0x1: // Page Control
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d Control = %04x (unk:%02x Blend:%d, HiColor:%d, unk:%d, unk%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n",  machine().describe_context(), tmap, data,
			(data & 0xfe00) >> 9, BIT(data, 8), BIT(data, 7), BIT(data, 6), BIT(data, 5), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));
		regs[offset] = data;
		break;

	case 0x2: // Page Tile Address
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d Tile Address = %04x\n",  machine().describe_context(), tmap, data);
		regs[offset] = data;
		break;

	case 0x3: // Page Attribute Address
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d Attribute Address = %04x\n",  machine().describe_context(), tmap, data);
		regs[offset] = data;
		break;
	}
}


// offsets 0,1,4,5,6,7 used in main IRQ code
// offsets 2,3 only cleared on startup

// Based on code analysis this seems to be the same as the regular tilemap regs, except for the addition of regs 2,3 which shift the remaining ones along.
// As the hardware appears to support ROZ these are probably 2 extra tile layers, with the 2 additional words being the ROZ parameters?


void gcm394_base_video_device::write_tmap_extrascroll(int tmap, uint16_t* regs, int offset, uint16_t data)
{
	switch (offset)
	{
	case 0x0: // Page X scroll
	case 0x1: // Page Y scroll
		write_tmap_scroll(tmap, regs, offset, data);
		break;

	case 0x2: //
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d X Unk Rotation Zoom Attribute1 = %04x\n", machine().describe_context(), tmap, data);
		regs[offset] = data;
		break;

	case 0x3:
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d X Unk Rotation Zoom Attribute = %04x\n", machine().describe_context(), tmap, data);
		regs[offset] = data;
		break;

	}
}


// **************************************** TILEMAP 0 *************************************************

uint16_t gcm394_base_video_device::tmap0_regs_r(offs_t offset)
{
	if (offset < 2)
	{
		return m_tmap0_scroll[offset];
	}
	else
	{
		return m_tmap0_regs[offset-2];
	}
}

void gcm394_base_video_device::tmap0_regs_w(offs_t offset, uint16_t data)
{
	LOGMASKED(LOG_GCM394_TMAP_EXTRA, "%s:gcm394_base_video_device::tmap0_regs_w %01x %04x\n", machine().describe_context(), offset, data);
	if (offset < 2)
	{
		write_tmap_scroll(0, m_tmap0_scroll, offset, data);
	}
	else
	{
		write_tmap_regs(0, m_tmap0_regs, offset-2, data);
	}
}

uint16_t gcm394_base_video_device::tmap0_tilebase_lsb_r()
{
	return m_page0_addr_lsb;
}

void gcm394_base_video_device::tmap0_tilebase_lsb_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap0_tilebase_lsb_w %04x\n", machine().describe_context(), data);
	m_page0_addr_lsb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(tmap0 tilegfxbase is now %04x%04x)\n", m_page0_addr_msb, m_page0_addr_lsb);
}

uint16_t gcm394_base_video_device::tmap0_tilebase_msb_r()
{
	return m_page0_addr_msb;
}

void gcm394_base_video_device::tmap0_tilebase_msb_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap0_tilebase_msb_w %04x\n", machine().describe_context(), data);
	m_page0_addr_msb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(tmap0 tilegfxbase is now %04x%04x)\n", m_page0_addr_msb, m_page0_addr_lsb);
}

// **************************************** TILEMAP 1 *************************************************


uint16_t gcm394_base_video_device::tmap1_regs_r(offs_t offset)
{
	if (offset < 2)
	{
		return m_tmap1_scroll[offset];
	}
	else
	{
		return m_tmap1_regs[offset-2];
	}
}

void gcm394_base_video_device::tmap1_regs_w(offs_t offset, uint16_t data)
{
	LOGMASKED(LOG_GCM394_TMAP_EXTRA, "%s:gcm394_base_video_device::tmap1_regs_w %01x %04x\n", machine().describe_context(), offset, data);
	if (offset < 2)
	{
		write_tmap_scroll(1, m_tmap1_scroll, offset, data);
	}
	else
	{
		write_tmap_regs(1, m_tmap1_regs, offset-2, data);
	}
}

uint16_t gcm394_base_video_device::tmap1_tilebase_lsb_r()
{
	return m_page1_addr_lsb;
}

void gcm394_base_video_device::tmap1_tilebase_lsb_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap1_tilebase_lsb_w %04x\n", machine().describe_context(), data);
	m_page1_addr_lsb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(tmap1 tilegfxbase is now %04x%04x)\n", m_page1_addr_msb, m_page1_addr_lsb);
}

uint16_t gcm394_base_video_device::tmap1_tilebase_msb_r()
{
	return m_page1_addr_msb;
}

void gcm394_base_video_device::tmap1_tilebase_msb_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap1_tilebase_msb_w %04x\n", machine().describe_context(), data);
	m_page1_addr_msb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(tmap1 tilegfxbase is now %04x%04x)\n", m_page1_addr_msb, m_page1_addr_lsb);
}

// **************************************** unknown video device 1 (another tilemap? roz? line? zooming sprite layer?) *************************************************

uint16_t gcm394_base_video_device::tmap2_regs_r(offs_t offset)
{
	if (offset < 4)
	{
		return m_tmap2_scroll[offset];
	}
	else
	{
		return m_tmap2_regs[offset-4];
	}
}

void gcm394_base_video_device::tmap2_regs_w(offs_t offset, uint16_t data)
{
	LOGMASKED(LOG_GCM394_TMAP_EXTRA, "%s:gcm394_base_video_device::tmap2_regs_w %01x %04x\n", machine().describe_context(), offset, data);
	if (offset < 4)
	{
		write_tmap_extrascroll(2, m_tmap2_scroll, offset, data);
	}
	else
	{
		write_tmap_regs(2, m_tmap2_regs, offset-4, data);
	}
}

uint16_t gcm394_base_video_device::tmap2_tilebase_lsb_r()
{
	return m_page2_addr_lsb;
}


void gcm394_base_video_device::tmap2_tilebase_lsb_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::tmap2_tilebase_lsb_w %04x\n", machine().describe_context(), data);
	m_page2_addr_lsb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(unk_vid1 tilegfxbase is now %04x%04x)\n", m_page2_addr_msb, m_page2_addr_lsb);
}

uint16_t gcm394_base_video_device::tmap2_tilebase_msb_r()
{
	return m_page2_addr_msb;
}

void gcm394_base_video_device::tmap2_tilebase_msb_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::tmap2_tilebase_msb_w %04x\n", machine().describe_context(), data);
	m_page2_addr_msb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(unk_vid1 tilegfxbase is now %04x%04x)\n", m_page2_addr_msb, m_page2_addr_lsb);
}

// **************************************** unknown video device 2 (another tilemap? roz? lines? zooming sprite layer?) *************************************************

uint16_t gcm394_base_video_device::tmap3_regs_r(offs_t offset)
{
	if (offset < 4)
	{
		return m_tmap3_scroll[offset];
	}
	else
	{
		return m_tmap3_regs[offset-4];
	}
}

void gcm394_base_video_device::tmap3_regs_w(offs_t offset, uint16_t data)
{
	LOGMASKED(LOG_GCM394_TMAP_EXTRA, "%s:gcm394_base_video_device::tmap3_regs_w %01x %04x\n", machine().describe_context(), offset, data);
	if (offset < 4)
	{
		write_tmap_extrascroll(3, m_tmap3_scroll, offset, data);
	}
	else
	{
		write_tmap_regs(3, m_tmap3_regs, offset-4, data);
	}
}

uint16_t gcm394_base_video_device::tmap3_tilebase_lsb_r()
{
	return m_page3_addr_lsb;
}


void gcm394_base_video_device::tmap3_tilebase_lsb_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::tmap3_tilebase_lsb_w %04x\n", machine().describe_context(), data);
	m_page3_addr_lsb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(unk_vid2 tilegfxbase is now %04x%04x)\n", m_page3_addr_msb, m_page3_addr_lsb);
}

uint16_t gcm394_base_video_device::tmap3_tilebase_msb_r()
{
	return m_page3_addr_msb;
}


void gcm394_base_video_device::tmap3_tilebase_msb_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::tmap3_tilebase_msb_w %04x\n", machine().describe_context(), data);
	m_page3_addr_msb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(unk_vid2 tilegfxbase is now %04x%04x)\n", m_page3_addr_msb, m_page3_addr_lsb);
}

// **************************************** sprite control registers *************************************************

// set to 001264c0 in wrlshunt, which point at the menu selectors (game names, arrows etc.)

uint16_t gcm394_base_video_device::sprite_7022_gfxbase_lsb_r()
{
	return m_sprite_7022_gfxbase_lsb;
}

void gcm394_base_video_device::sprite_7022_gfxbase_lsb_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::sprite_7022_gfxbase_lsb_w %04x\n", machine().describe_context(), data);
	m_sprite_7022_gfxbase_lsb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(sprite tilebase is now %04x%04x)\n", m_sprite_702d_gfxbase_msb, m_sprite_7022_gfxbase_lsb);
}

uint16_t gcm394_base_video_device::sprite_702d_gfxbase_msb_r()
{
	return m_sprite_702d_gfxbase_msb;
}

void gcm394_base_video_device::sprite_702d_gfxbase_msb_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::sprite_702d_gfxbase_msb_w %04x\n", machine().describe_context(), data);
	m_sprite_702d_gfxbase_msb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(sprite tilebase tilegfxbase is now %04x%04x)\n", m_sprite_702d_gfxbase_msb, m_sprite_7022_gfxbase_lsb);
}

uint16_t gcm394_base_video_device::sprite_7042_extra_r()
{
	uint16_t retdata = m_7042_sprite;
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::sprite_7042_extra_r (returning: %04x)\n", machine().describe_context(), retdata);
	return retdata;
}

void gcm394_base_video_device::sprite_7042_extra_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::sprite_7042_extra_w %04x\n", machine().describe_context(), data);
	m_7042_sprite = data;
	m_renderer->set_video_reg_42(data);

	//popmessage("extra modes %04x\n", data);
}


// **************************************** video DMA device *************************************************

void gcm394_base_video_device::video_dma_source_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_source_w %04x\n", machine().describe_context(), data);
	m_videodma_source = data;
}

void gcm394_base_video_device::video_dma_dest_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_dest_w %04x\n", machine().describe_context(), data);
	m_videodma_dest = data;
}

uint16_t gcm394_base_video_device::video_dma_size_busy_r()
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_size_busy_r\n", machine().describe_context());
	return 0x0000;
}

void gcm394_base_video_device::video_dma_size_trigger_w(address_space &space, uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_size_trigger_w %04x\n", machine().describe_context(), data);
	m_videodma_size = data;

	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s: doing sprite / video DMA source %04x dest %04x size %04x value of 707e (bank) %04x value of 707f %04x\n", machine().describe_context(), m_videodma_source, m_videodma_dest, m_videodma_size, m_707e_spritebank, m_707f );

	for (int i = 0; i <= m_videodma_size; i++)
	{
		uint16_t dat = space.read_word(m_videodma_source+i);
		space.write_word(m_videodma_dest + i, dat);
	}

	m_videodma_size = 0x0000;

	if (m_video_irq_enable & 4)
	{
		const uint16_t old = m_video_irq_status;
		m_video_irq_status |= 4;
		const uint16_t changed = old ^ (m_video_irq_enable & m_video_irq_status);
		if (changed)
			check_video_irq();
	}
}

void gcm394_base_video_device::video_707e_spritebank_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_707e_spritebank_w %04x\n", machine().describe_context(), data);
	m_707e_spritebank = data;
}

uint16_t gcm394_base_video_device::video_707c_r()
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_707c_r\n", machine().describe_context());
	return 0x8000;
}

/* 707f is VERY important, lots of rendering codepaths in the code depend on the value it returns.

   all operations in the code based on 707f are bit based, usually read register, set / clear a bit
   and then write register, or read register and test an individual bit.

   our current codeflow means that bits are only ever set, not cleared.

   are the bits triggers? acks? enables? status flags?

   in wrlshunt this ends up being set to  02f9   ---- --x- xxxx x--x
   and in smartfp it ends up being set to 0065   ---- ---- -xx- -x-x

   is this because wrlshunt uses more layers?
*/


uint16_t gcm394_base_video_device::video_707f_r()
{
	uint16_t retdata = m_renderer->get_video_reg_7f();
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_707f_r (returning %04x)\n", machine().describe_context(), retdata);
	return retdata;
}
void gcm394_base_video_device::video_707f_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_707f_w %04x\n", machine().describe_context(), data);

	for (int i = 0; i < 16; i++)
	{
		uint16_t mask = 1 << i;

		if ((m_707f & mask) != (data & mask))
		{
			if (data & mask)
			{
				LOGMASKED(LOG_GCM394_VIDEO, "\tbit %04x Low -> High\n", mask);
			}
			else
			{
				LOGMASKED(LOG_GCM394_VIDEO, "\tbit %04x High -> Low\n", mask);
			}
		}
	}

	m_707f = data;
	m_renderer->set_video_reg_7f(data);
	//popmessage("707f is %04x\n", data);
}

uint16_t gcm394_base_video_device::video_703a_palettebank_r()
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_703a_palettebank_r\n", machine().describe_context());
	return m_703a_palettebank;
}

void gcm394_base_video_device::video_703a_palettebank_w(uint16_t data)
{
	// I don't think bit 0 (0x01) is a bank select, it might be a 'mode select' for how the palette operates.
	// neither lazertag or tkmag220 set it
	// lazertag uses 2 banks (0 and 8)
	// tkmag220 only uses 1 bank (0)

	// ---- bb-s
	// bb = write bank?
	// s = sprite palette bank select?

	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_703a_palettebank_w %04x\n", machine().describe_context(), data);
	m_703a_palettebank = data;
}

uint16_t gcm394_base_video_device::videoirq_source_enable_r()
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::videoirq_source_enable_r\n", machine().describe_context());
	return m_video_irq_enable;
}

void gcm394_base_video_device::videoirq_source_enable_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "videoirq_source_enable_w: Video IRQ Enable = %04x (DMA:%d, Timing:%d, Blanking:%d)\n", data, BIT(data, 2), BIT(data, 1), BIT(data, 0));
	const uint16_t old = m_video_irq_enable & m_video_irq_status;
	m_video_irq_enable = data & 0x0007;
	const uint16_t changed = old ^ (m_video_irq_enable & m_video_irq_status);
	if (changed)
		check_video_irq();
}

uint16_t gcm394_base_video_device::video_7063_videoirq_source_r()
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7063_videoirq_source_r\n", machine().describe_context());
	return m_video_irq_status;
}


void gcm394_base_video_device::video_7063_videoirq_source_ack_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "video_7063_videoirq_source_ack_w: Video IRQ Acknowledge = %04x\n", data);
	const uint16_t old = m_video_irq_enable & m_video_irq_status;
	m_video_irq_status &= ~data;
	const uint16_t changed = old ^ (m_video_irq_enable & m_video_irq_status);
	if (changed)
		check_video_irq();
}

void gcm394_base_video_device::video_702a_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_702a_w %04x\n", machine().describe_context(), data);
	m_702a = data;
	m_renderer->set_video_reg_2a(data);
}

uint16_t gcm394_base_video_device::video_curline_r()
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s: video_r: Current Line: %04x\n", machine().describe_context(), m_screen->vpos());
	return m_screen->vpos();
}

// read in IRQ
uint16_t gcm394_base_video_device::video_7030_brightness_r()
{
	/* wrlshunt ends up doing an explicit jump to 0000 shortly after boot if you just return the value written here, however I think that is correct code flow and something else is wrong
	   as this simply looks like some kind of brightness register - there is code to decrease it from 0xff to 0x00 by 0x5 increments (waiting for it to hit 0x05) and code to do the reverse
	   either way it really looks like the data written should be read back.
	*/
	uint16_t retdat = m_7030_brightness;
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7030_brightness_r (returning %04x)\n", machine().describe_context(), retdat);
	return retdat;
}

void gcm394_base_video_device::video_7030_brightness_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7030_brightness_w %04x\n", machine().describe_context(), data);
	m_7030_brightness = data;
	m_renderer->set_video_reg_30(data);
}

void gcm394_base_video_device::update_raster_split_position()
{
	// this might need updating to handle higher res modes
	LOGMASKED(LOG_GCM394_VIDEO, "update_raster_split_position: %04x,%04x\n", m_yirqpos, m_xirqpos);
	if (m_xirqpos < 300 && m_yirqpos < 256)
	{
		// where does -19 come from? needed for raster on paccon xevious to fire at correct line for bg scrolling to be seamless
		m_screenpos_timer->adjust(m_screen->time_until_pos(m_yirqpos-19, m_xirqpos));
		//printf("setting irq timer for y:%d x:%d", m_yirqpos, m_xirqpos);
	}
	else
		m_screenpos_timer->adjust(attotime::never);
}

void gcm394_base_video_device::split_irq_ypos_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:split_irq_ypos_w %04x\n", machine().describe_context(), data);

	m_yirqpos = data & 0x1ff;
	update_raster_split_position();
}

void gcm394_base_video_device::split_irq_xpos_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:split_irq_xpos_w %04x\n", machine().describe_context(), data);

	m_xirqpos = data & 0x1ff;
	update_raster_split_position();
}

uint16_t gcm394_base_video_device::video_703c_tvcontrol1_r()
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_703c_tvcontrol1_r\n", machine().describe_context());
	return m_703c_tvcontrol1;
}

void gcm394_base_video_device::video_703c_tvcontrol1_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_703c_tvcontrol1_w %04x\n", machine().describe_context(), data);
	m_703c_tvcontrol1 = data;
	m_renderer->set_video_reg_3c(data);
}

uint16_t gcm394_base_video_device::video_7051_r()
{
	/* related to what ends up crashing wrlshunt? */
	uint16_t retdat = 0x03ff;
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7051_r (returning %04x)\n", machine().describe_context(), retdat);
	return retdat;
}

uint16_t gcm394_base_video_device::video_70e0_prng_r()
{
	// several games in the beijuehh use this to generate random pieces / enemies, bit 0x8000 can't be set or it will generate a negative index into tables
	uint16_t retdat = machine().rand() & 0x7fff;
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_70e0_prng_r (returning %04x)\n", machine().describe_context(), retdat);
	return retdat;
}


// this block get set once, in a single function, could be important
void gcm394_base_video_device::video_7080_w(uint16_t data) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7080_w %04x\n", machine().describe_context(), data); m_7080 = data; }
void gcm394_base_video_device::video_7081_w(uint16_t data) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7081_w %04x\n", machine().describe_context(), data); m_7081 = data; }
void gcm394_base_video_device::video_7082_w(uint16_t data) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7082_w %04x\n", machine().describe_context(), data); m_7082 = data; }
void gcm394_base_video_device::video_7083_w(uint16_t data) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7083_w %04x\n", machine().describe_context(), data); m_7083 = data; }
void gcm394_base_video_device::video_7084_w(uint16_t data) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7084_w %04x\n", machine().describe_context(), data); m_7084 = data; }
void gcm394_base_video_device::video_7085_w(uint16_t data) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7085_w %04x\n", machine().describe_context(), data); m_7085 = data; }
void gcm394_base_video_device::video_7086_w(uint16_t data) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7086_w %04x\n", machine().describe_context(), data); m_7086 = data; }
void gcm394_base_video_device::video_7087_w(uint16_t data) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7087_w %04x\n", machine().describe_context(), data); m_7087 = data; }
void gcm394_base_video_device::video_7088_w(uint16_t data) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7088_w %04x\n", machine().describe_context(), data); m_7088 = data; }

uint16_t gcm394_base_video_device::video_7083_r() { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7083_r\n", machine().describe_context()); return m_7083; }

void gcm394_base_video_device::spriteram_w(offs_t offset, uint16_t data)
{
	// transfers an additional word for each sprite with this bit set (smartfp) or an entire extra bank (wrlshunt)
	// wrlshunt instead seems to base if it writes the extra data based on 707f so maybe this is more complex than banking

	// however for 707e only 0/1 is written, and it also gets written before system DMA, so despite being in the video DMA
	// region seems to operate separate from that.

	if (m_707e_spritebank == 0x0000)
	{
		m_spriteram[offset] = data;
	}
	else if (m_707e_spritebank == 0x0001)
	{
		m_spriteram[offset + 0x400] = data;
	}
	else
	{
		LOGMASKED(LOG_GCM394_VIDEO, "%s: spriteram_w %04x %04x unknown bank %04x\n", machine().describe_context(), offset, data, m_707e_spritebank);
	}
}

uint16_t gcm394_base_video_device::spriteram_r(offs_t offset)
{
	if (m_707e_spritebank == 0x0000)
	{
		return m_spriteram[offset];
	}
	else if (m_707e_spritebank == 0x0001)
	{
		return m_spriteram[offset + 0x400];
	}
	else
	{
		LOGMASKED(LOG_GCM394_VIDEO, "%s: spriteram_r %04x unknown bank %04x\n", machine().describe_context(), offset,  m_707e_spritebank);
		return 0x0000;
	}
}

void gcm394_base_video_device::palette_w(offs_t offset, uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO_PALETTE, "%s:gcm394_base_video_device::palette_w %04x : %04x (value of 0x703a is %04x)\n", machine().describe_context(), offset, data, m_703a_palettebank);

	if (m_703a_palettebank & 0xfff0)
	{
		LOGMASKED(LOG_GCM394_VIDEO_PALETTE,"palette writes with m_703a_palettebank %04x\n", m_703a_palettebank);
	}


	offset |= (m_703a_palettebank & 0x000c) << 6;
	m_paletteram[offset] = data;

	// for debug
	m_palette->set_pen_color(offset, rgb_t(
		(((data >> 10) & 0x1f)<<3),
		(((data >> 5)  & 0x1f)<<3),
		(((data >> 0)  & 0x1f)<<3)));

}

uint16_t gcm394_base_video_device::palette_r(offs_t offset)
{
	if (m_703a_palettebank & 0xfff0)
	{
		LOGMASKED(LOG_GCM394_VIDEO_PALETTE,"palette read with m_703a_palettebank %04x\n", m_703a_palettebank);
	}

	offset |= (m_703a_palettebank & 0x000c) << 6;
	return m_paletteram[offset];
}

void gcm394_base_video_device::video_701c_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_701c_w (unknown video reg?) %04x\n", machine().describe_context(), data);
	m_renderer->set_video_reg_1c(data);
}

void gcm394_base_video_device::video_701d_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_701d_w (unknown video reg?) %04x\n", machine().describe_context(), data);
	m_renderer->set_video_reg_1d(data);
}

void gcm394_base_video_device::video_701e_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_701e_w (unknown video reg?) %04x\n", machine().describe_context(), data);
	m_renderer->set_video_reg_1e(data);
}


void gcm394_base_video_device::check_video_irq()
{
	LOGMASKED(LOG_GCM394_VIDEO, "%ssserting Video IRQ (%04x, %04x)\n", (m_video_irq_status & m_video_irq_enable) ? "A" : "Dea", m_video_irq_status, m_video_irq_enable);
	m_video_irq_cb((m_video_irq_status & m_video_irq_enable) ? ASSERT_LINE : CLEAR_LINE);
}

void gcm394_base_video_device::vblank(int state)
{
	if (!state)
	{
		m_video_irq_status &= ~1;
		LOGMASKED(LOG_GCM394_VIDEO, "Setting video IRQ status to %04x\n", m_video_irq_status);
		check_video_irq();
		return;
	}

	if (m_video_irq_enable & 1)
	{
		// jak_prft expects 0x800 to be set in the status register or most of the main vblank code is skipped, why?

		m_video_irq_status |= 1 | 0x800;
		LOGMASKED(LOG_GCM394_VIDEO, "Setting video IRQ status to %04x\n", m_video_irq_status);
		check_video_irq();
	}
}

TIMER_CALLBACK_MEMBER(gcm394_base_video_device::screen_pos_reached)
{
	if (m_video_irq_enable & 2)
	{
		m_video_irq_status |= 2;
		check_video_irq();
	}

	//printf("firing irq timer\n");

	m_screen->update_partial(m_screen->vpos());

	// fire again, jak_dbz pinball needs this
	m_screenpos_timer->adjust(m_screen->time_until_pos(m_yirqpos-19, m_xirqpos));
}


static GFXDECODE_START( gfx )
GFXDECODE_END

void gcm394_base_video_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 256*0x10);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx);

	SPG_RENDERER(config, m_renderer, 0);
}


