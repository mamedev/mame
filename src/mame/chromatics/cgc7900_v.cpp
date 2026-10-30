// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "cgc7900.h"
#include "screen.h"

#include <algorithm>

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define OVERLAY_CUR                 BIT(cell, 31)   /* places a cursor in the cell if SET */
#define OVERLAY_BLK                 BIT(cell, 30)   /* blinks the foreground character in the cell if SET */
#define OVERLAY_VF                  BIT(cell, 28)   /* makes the foreground visible if SET (else transparent) */
#define OVERLAY_VB                  BIT(cell, 27)   /* makes the background visible if SET (else transparent) */
#define OVERLAY_PL                  BIT(cell, 24)   /* uses bits 0-7 as PLOT DOT descriptor if SET (else ASCII) */
#define OVERLAY_BR                  BIT(cell, 18)   /* turns on Red in background if SET */
#define OVERLAY_BG                  BIT(cell, 17)   /* turns on Green in background if SET */
#define OVERLAY_BB                  BIT(cell, 16)   /* turns on Blue in background if SET */
#define OVERLAY_FR                  BIT(cell, 10)   /* turns on Red in foreground if SET */
#define OVERLAY_FG                  BIT(cell, 9)    /* turns on Green in foreground if SET */
#define OVERLAY_FB                  BIT(cell, 8)    /* turns on Blue in background if SET */
#define OVERLAY_DATA                (cell & 0xff)   /* ASCII or Plot Dot character */

void cgc7900_state::cgc7900_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0xff));
	palette.set_pen_color(2, rgb_t(0x00, 0xff, 0x00));
	palette.set_pen_color(3, rgb_t(0x00, 0xff, 0xff));
	palette.set_pen_color(4, rgb_t(0xff, 0x00, 0x00));
	palette.set_pen_color(5, rgb_t(0xff, 0x00, 0xff));
	palette.set_pen_color(6, rgb_t(0xff, 0xff, 0x00));
	palette.set_pen_color(7, rgb_t::white());
}

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    z_mode_r - Z mode read
-------------------------------------------------*/

u16 cgc7900_state::z_mode_r(offs_t offset)
{
	offs_t const pixel = (offset + (bitmap_roll() << 10)) & 0xfffff;
	offs_t const word = pixel >> 4;
	int const bit = 15 - (pixel & 0x0f);
	u16 const select = m_latch[LATCH_PLANE_SELECT];
	u16 data = 0;

	for (int plane = 0; plane < 16; plane++)
		if (BIT(select, plane) && BIT(m_plane_ram[(plane << 16) | word], bit))
			data |= 1 << plane;

	return data;
}

/*-------------------------------------------------
    z_mode_w - Z mode write
-------------------------------------------------*/

void cgc7900_state::z_mode_w(offs_t offset, u16 data, u16 mem_mask)
{
	offs_t const pixel = (offset + (bitmap_roll() << 10)) & 0xfffff;
	offs_t const word = pixel >> 4;
	u16 const bit = 1 << (15 - (pixel & 0x0f));
	u16 const select = m_latch[LATCH_PLANE_SELECT] & mem_mask;

	for (int plane = 0; plane < 16; plane++)
	{
		if (!BIT(select, plane))
			continue;

		u16 &ram = m_plane_ram[(plane << 16) | word];

		if (BIT(data, plane))
			ram |= bit;
		else
			ram &= ~bit;
	}
}

/*-------------------------------------------------
    color_status_w - color status write
-------------------------------------------------*/

void cgc7900_state::color_status_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 const fg = m_latch[LATCH_COLOR_STATUS_FG];
	u16 const bg = m_latch[LATCH_COLOR_STATUS_BG];

	for (int plane = 0; plane < 16; plane++)
	{
		u16 const color = (BIT(fg, plane) ? data : 0) | (BIT(bg, plane) ? ~data : 0);
		u16 &ram = m_plane_ram[(plane << 16) | offset];

		ram = (ram & ~mem_mask) | (color & mem_mask);
	}
}

/*-------------------------------------------------
    clut_r - color lookup table read
-------------------------------------------------*/

u16 cgc7900_state::clut_r(offs_t offset)
{
	return BIT(offset, 0) ? m_clut_ram[offset] : (0xff00 | m_clut_ram[offset]);
}

/*-------------------------------------------------
    clut_w - color lookup table write
-------------------------------------------------*/

void cgc7900_state::clut_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_clut_ram[offset]);

	if (!BIT(offset, 0))
		m_clut_ram[offset] &= 0x00ff;
}

/*-------------------------------------------------
    overlay_r - overlay memory read
-------------------------------------------------*/

u16 cgc7900_state::overlay_r(offs_t offset)
{
	return m_overlay_ram[((((offset >> 1) + overlay_roll()) & 0xfff) << 1) | (offset & 1)];
}

/*-------------------------------------------------
    overlay_w - overlay memory write
-------------------------------------------------*/

void cgc7900_state::overlay_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_overlay_ram[((((offset >> 1) + overlay_roll()) & 0xfff) << 1) | (offset & 1)]);
}

/*-------------------------------------------------
    latch_r - hardware latch read
-------------------------------------------------*/

u16 cgc7900_state::latch_r(offs_t offset)
{
	return m_cmos_ram[offset];
}

/*-------------------------------------------------
    latch_w - hardware latch write
-------------------------------------------------*/

void cgc7900_state::latch_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_cmos_ram[offset]);
	COMBINE_DATA(&m_latch[offset]);
}

/*-------------------------------------------------
    cgc7900_sync_r - sync information read
-------------------------------------------------*/

u16 cgc7900_state::sync_r()
{
	u16 data = 0xffff;

	/*

	    bit     signal      description

	     0      _VERT       vertical retrace (0=vblank)
	     1                  interlace (1=first field, 0=second field)
	     2      _HG         horizontal retrace (0=hblank)
	     3      1
	     4      1
	     5      1
	     6      1
	     7      1
	     8      1
	     9      1
	    10      1
	    11      1
	    12      1
	    13      1
	    14      1
	    15      1

	*/

	if (m_screen->vblank()) data &= ~1;
	if (BIT(m_screen->frame_number(), 0)) data &= ~2;
	if (m_screen->hblank()) data &= ~4;

	return data;
}

/***************************************************************************
    VIDEO
***************************************************************************/

/*-------------------------------------------------
    update_clut - update color lookup table
-------------------------------------------------*/

void cgc7900_state::update_clut()
{
	for (int i = 0; i < 256; i++)
		m_clut[i] = rgb_t(m_clut_ram[i << 1] & 0xff, m_clut_ram[(i << 1) | 1] >> 8, m_clut_ram[(i << 1) | 1] & 0xff);
}

void cgc7900_state::device_post_load()
{
	update_clut();
}

/*-------------------------------------------------
    vblank_w - vertical retrace
-------------------------------------------------*/

void cgc7900_state::vblank_w(int state)
{
	if (state)
	{
		update_clut();

		bool off_center = false;

		for (auto &axis : m_joystick)
			if ((axis->read() & 0x3ff) != 0x200)
				off_center = true;

		irq_encoder(11, off_center ? ASSERT_LINE : CLEAR_LINE);
	}

	irq_encoder(12, state);
}

/*-------------------------------------------------
    draw_bitmap - draw bitmap image
-------------------------------------------------*/

void cgc7900_state::draw_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int const base = image_select() ? 8 : 0;
	u16 mask = m_latch[LATCH_PLANE_SWITCH];

	int const zoom_h = zoom_x() + 1;
	int const zoom_v = zoom_y() + 1;

	if (m_blink)
		mask &= ~m_latch[LATCH_BLINK_SELECT];

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		offs_t const row = ((pan_y() + y / zoom_v + bitmap_roll()) & 0x3ff) << 6;
		u32 *const dest = &bitmap.pix(y);
		offs_t cached = ~offs_t(0);
		u16 data[8] = { };

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			offs_t const column = (pan_x() + x / zoom_h) & 0x3ff;
			offs_t const word = row | (column >> 4);
			int const bit = 15 - (column & 0x0f);

			if (word != cached)
			{
				for (int plane = 0; plane < 8; plane++)
					data[plane] = BIT(mask, base + plane) ? m_plane_ram[((base + plane) << 16) | word] : 0;

				cached = word;
			}

			u8 index = 0;

			for (int plane = 0; plane < 8; plane++)
				index |= BIT(data[plane], bit) << plane;

			dest[x] = m_clut[index];
		}
	}
}

/*-------------------------------------------------
    draw_overlay - draw text overlay
-------------------------------------------------*/

void cgc7900_state::draw_overlay(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();
	u8 const *const char_rom = m_char_rom->base();

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int const sy = y / 16;
		int const line = (y % 16) / 2;
		u32 *const dest = &bitmap.pix(y);

		for (int sx = cliprect.min_x / 12; (sx < 85) && (sx * 12 <= cliprect.max_x); sx++)
		{
			offs_t const addr = (((sy * 85) + sx + overlay_roll()) & 0xfff) << 1;
			u32 const cell = (m_overlay_ram[addr] << 16) | m_overlay_ram[addr + 1];
			int const x0 = std::max(sx * 12, cliprect.min_x);
			int const x1 = std::min(sx * 12 + 11, cliprect.max_x);

			if (OVERLAY_CUR && (!overlay_cursor_blink() || m_blink) && ((line == 0) || (line == 7)))
			{
				for (int x = x0; x <= x1; x++)
					dest[x] = pen[7];

				continue;
			}

			u8 pattern;

			if (OVERLAY_PL)
				pattern = BIT(OVERLAY_DATA, (y % 16) / 4) | (BIT(OVERLAY_DATA, 4 + (y % 16) / 4) << 1);
			else
				pattern = char_rom[(OVERLAY_DATA << 3) | line];

			if (OVERLAY_BLK && overlay_character_blink() && !m_blink)
				pattern = 0;

			for (int x = x0; x <= x1; x++)
			{
				bool const dot = OVERLAY_PL ? BIT(pattern, (x - sx * 12) / 6) : BIT(pattern, (x - sx * 12) / 2);

				if (dot)
				{
					if (OVERLAY_VF) dest[x] = pen[(cell >> 8) & 0x07];
				}
				else
				{
					if (OVERLAY_VB) dest[x] = pen[(cell >> 16) & 0x07];
				}
			}
		}
	}
}

/*-------------------------------------------------
    TIMER_DEVICE_CALLBACK_MEMBER( blink_tick )
-------------------------------------------------*/

TIMER_DEVICE_CALLBACK_MEMBER(cgc7900_state::blink_tick)
{
	m_blink = !m_blink;
}

u32 cgc7900_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	draw_bitmap(bitmap, cliprect);
	draw_overlay(bitmap, cliprect);

	return 0;
}

/*-------------------------------------------------
    gfx_layout cgc7900_charlayout
-------------------------------------------------*/

static const gfx_layout cgc7900_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ STEP8(7,-1) },
	{ STEP8(0,8) },
	8*8
};

/*-------------------------------------------------
    GFXDECODE( cgc7900 )
-------------------------------------------------*/

static GFXDECODE_START( gfx_cgc7900 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, cgc7900_charlayout, 0, 1 )
GFXDECODE_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

/*-------------------------------------------------
    MACHINE_DRIVER( cgc7900_video )
-------------------------------------------------*/

void cgc7900_state::cgc7900_video(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG));
	// interlaced fields of 395.5 lines are modeled as 791 half-lines at twice the pixel clock
	screen.set_raw(XTAL(28'480'000) * 2, 1200, 0, 1024, 791, 0, 768);
	screen.set_screen_update(FUNC(cgc7900_state::screen_update));
	screen.screen_vblank().set(FUNC(cgc7900_state::vblank_w));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_cgc7900);
	PALETTE(config, m_palette, FUNC(cgc7900_state::cgc7900_palette), 8);

	TIMER(config, "blink").configure_periodic(FUNC(cgc7900_state::blink_tick), attotime::from_hz(XTAL(28'480'000)/7500000));
}
