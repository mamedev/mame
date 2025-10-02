// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  Galaxian hardware family

***************************************************************************/

#include "emu.h"
#include "galaxold.h"

#define STARS_COLOR_BASE        (memregion("proms")->bytes())
#define BULLETS_COLOR_BASE      (STARS_COLOR_BASE + 64)
#define BACKGROUND_COLOR_BASE   (BULLETS_COLOR_BASE + 2)

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Galaxian has one 32 bytes palette PROM, connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  The output of the background star generator is connected this way:

  bit 5 -- 100 ohm resistor  -- BLUE
        -- 150 ohm resistor  -- BLUE
        -- 100 ohm resistor  -- GREEN
        -- 150 ohm resistor  -- GREEN
        -- 100 ohm resistor  -- RED
  bit 0 -- 150 ohm resistor  -- RED

  The blue background in Scramble and other games goes through a 390 ohm
  resistor.

  The bullet RGB outputs go through 100 ohm resistors.

  The RGB outputs have a 470 ohm pull-down each.

***************************************************************************/
void galaxold_state::galaxold_palette(palette_device &palette)
{
	const uint8_t *color_prom = memregion("proms")->base();

	// first, the character/sprite palette
	int const len = memregion("proms")->bytes();
	for (int i = 0; i < len; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = BIT(*color_prom, 6);
		bit1 = BIT(*color_prom, 7);
		int const b = 0x52 * bit0 + 0xad * bit1;

		palette.set_pen_color(i, r, g, b);
		color_prom++;
	}

	galaxold_init_stars(STARS_COLOR_BASE);

	// bullets - yellow and white
	palette.set_pen_color(BULLETS_COLOR_BASE + 0, rgb_t(0xef, 0xef, 0x00));
	palette.set_pen_color(BULLETS_COLOR_BASE + 1, rgb_t(0xef, 0xef, 0xef));
}

void galaxold_state::scrambold_palette(palette_device &palette)
{
	galaxold_palette(palette);

	// blue background - 390 ohm resistor
	palette.set_pen_color(BACKGROUND_COLOR_BASE, rgb_t(0, 0, 0x56));
}



void galaxold_state::stratgyx_palette(palette_device &palette)
{
	galaxold_palette(palette);

	/*  The background color generator is connected this way:

	    RED   - 270 ohm resistor
	    GREEN - 560 ohm resistor
	    BLUE  - 470 ohm resistor */

	for (int i = 0; i < 8; i++)
	{
		int const r = BIT(i, 0) * 0x7c;
		int const g = BIT(i, 1) * 0x3c;
		int const b = BIT(i, 2) * 0x47;

		palette.set_pen_color(BACKGROUND_COLOR_BASE + i, r, g, b);
	}
}

void galaxold_state::s2650_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// first, the character/sprite palette
	int const len = memregion("proms")->bytes();
	for (int i = 0; i < len; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = BIT(*color_prom, 6);
		bit1 = BIT(*color_prom, 7);
		int const b = 0x52 * bit0 + 0xad * bit1;

		palette.set_pen_color(i, r, g, b);
		color_prom++;
	}
}

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Dark Planet has one 32 bytes palette PROM, connected to the RGB output this way:

  bit 5 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  The bullet RGB outputs go through 100 ohm resistors.

  The RGB outputs have a 470 ohm pull-down each.

***************************************************************************/
void galaxold_state::darkplnt_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// first, the character/sprite palette
	for (int i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		int const g = 0x00;
		// blue component
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, r, g, b);
		color_prom++;
	}

	// bullets - red and blue
	palette.set_pen_color(BULLETS_COLOR_BASE + 0, rgb_t(0xef, 0x00, 0x00));
	palette.set_pen_color(BULLETS_COLOR_BASE + 1, rgb_t(0x00, 0x00, 0xef));
}

void galaxold_state::minefld_palette(palette_device &palette)
{
	galaxold_palette(palette);

	// set up background colors

	// graduated blue

	for (int i = 0; i < 128; i++)
	{
		int const r = 0;
		int const g = i;
		int const b = i * 2;
		palette.set_pen_color(BACKGROUND_COLOR_BASE + i, r, g, b);
	}

	// graduated brown

	for (int i = 0; i < 128; i++)
	{
		int const r = i * 1.5;
		int const g = i * 0.75;
		int const b = i / 2;
		palette.set_pen_color(BACKGROUND_COLOR_BASE + 128 + i, r, g, b);
	}
}

void galaxold_state::rescue_palette(palette_device &palette)
{
	galaxold_palette(palette);

	// set up background colors

	// graduated blue

	for (int i = 0; i < 128; i++)
	{
		int const r = 0;
		int const g = i;
		int const b = i * 2;
		palette.set_pen_color(BACKGROUND_COLOR_BASE + i, r, g, b);
	}
}

void galaxold_state::mariner_palette(palette_device &palette)
{
	galaxold_palette(palette);

	/* set up background colors */

	/* 16 shades of blue - the 4 bits are connected to the following resistors:

	    bit 0 -- 4.7 kohm resistor
	          -- 2.2 kohm resistor
	          -- 1   kohm resistor
	    bit 0 -- .47 kohm resistor */

	for (int i = 0; i < 16; i++)
	{
		int const r = 0;
		int const g = 0;
		int const b = 0x0e * BIT(i, 0) + 0x1f * BIT(i, 1) + 0x43 * BIT(i, 2) + 0x8f * BIT(i, 3);

		palette.set_pen_color(BACKGROUND_COLOR_BASE + i, r, g, b);
	}
}

// swapped r/g/b hook-up
void galaxold_state::dambustr_palette(palette_device &palette)
{
	const uint8_t *color_prom = memregion("proms")->base();

	// first, the character/sprite palette
	int const len = memregion("proms")->bytes();
	for (int i = 0; i < len; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = BIT(*color_prom, 6);
		bit1 = BIT(*color_prom, 7);
		int const g = 0x52 * bit0 + 0xad * bit1;

		palette.set_pen_color(i, r, g, b);
		color_prom++;
	}

	galaxold_init_stars(STARS_COLOR_BASE);

	// bullets - yellow and white
	palette.set_pen_color(BULLETS_COLOR_BASE + 0, rgb_t(0xef, 0xef, 0x00));
	palette.set_pen_color(BULLETS_COLOR_BASE + 1, rgb_t(0xef, 0xef, 0xef));

	/*
	Assumption (not clear from the schematics):
	The background color generator is connected this way:

	    RED   - 470 ohm resistor
	    GREEN - 470 ohm resistor
	    BLUE  - 470 ohm resistor */

	for (int i = 0; i < 8; i++)
	{
		int const r = BIT(i, 0) * 0x47;
		int const g = BIT(i, 1) * 0x47;
		int const b = BIT(i, 2) * 0x47;
		palette.set_pen_color(BACKGROUND_COLOR_BASE + i, r, g, b);
	}
}


void galaxold_state::turtles_palette(palette_device &palette)
{
	galaxold_palette(palette);

	/*  The background color generator is connected this way:

	    RED   - 390 ohm resistor
	    GREEN - 470 ohm resistor
	    BLUE  - 390 ohm resistor */

	for (int i = 0; i < 8; i++)
	{
		int const r = BIT(i, 0) * 0x55;
		int const g = BIT(i, 1) * 0x47;
		int const b = BIT(i, 2) * 0x55;

		palette.set_pen_color(BACKGROUND_COLOR_BASE + i, r, g, b);
	}
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void galaxold_state::state_save_register()
{
	save_item(NAME(m_gfxbank));
	save_item(NAME(m_flipscreen_x));
	save_item(NAME(m_flipscreen_y));

	save_item(NAME(m_stars_on));
	save_item(NAME(m_stars_scrollpos));
	save_item(NAME(m_stars_blink_state));
	save_item(NAME(m_timer_adjusted));

	save_item(NAME(m_darkplnt_bullet_color));

	save_item(NAME(m_background_enable));
	save_item(NAME(m_background_red));
	save_item(NAME(m_background_green));
	save_item(NAME(m_background_blue));
}

void galaxold_state::video_start_common()
{
	m_modify_charcode = nullptr;
	m_modify_spritecode = nullptr;
	m_modify_color = nullptr;
	m_modify_ypos = nullptr;

	m_draw_bullets = nullptr;

	m_draw_background = &galaxold_state::galaxold_draw_background;
	m_background_enable = 0;
	m_background_blue = 0;
	m_background_red = 0;
	m_background_green = 0;

	m_stars_on = 0;
	m_draw_stars = &galaxold_state::noop_draw_stars;

	m_flipscreen_x = 0;
	m_flipscreen_y = 0;

	m_spriteram2_present = 0;

	std::fill(std::begin(m_gfxbank), std::end(m_gfxbank), 0);

	state_save_register();
}

VIDEO_START_MEMBER(galaxold_state,galaxold_plain)
{
	video_start_common();
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxold_state::get_tile_info)), TILEMAP_SCAN_ROWS,8,8,32,32);
	m_bg_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scroll_cols(32);

	m_color_mask = (m_gfxdecode->gfx(0)->granularity() == 4) ? 7 : 3;
}

VIDEO_START_MEMBER(galaxold_state,galaxold)
{
	VIDEO_START_CALL_MEMBER(galaxold_plain);

	m_draw_stars = &galaxold_state::galaxold_draw_stars;

	m_draw_bullets = &galaxold_state::galaxold_draw_bullets;
}

VIDEO_START_MEMBER(galaxold_state,scrambold)
{
	VIDEO_START_CALL_MEMBER(galaxold_plain);

	/* FIXME: This most probably needs to be adjusted
	 * again when RAW video params are added to scramble
	 */
	m_bg_tilemap->set_scrolldx(0, 0);

	m_draw_stars = &galaxold_state::scrambold_draw_stars;

	m_draw_bullets = &galaxold_state::scrambold_draw_bullets;

	m_draw_background = &galaxold_state::scrambold_draw_background;
}

VIDEO_START_MEMBER(galaxold_state, newsin7)
{
	VIDEO_START_CALL_MEMBER(scrambold);

	m_leftclip = 0;
}


VIDEO_START_MEMBER(galaxold_state,darkplnt)
{
	VIDEO_START_CALL_MEMBER(galaxold_plain);

	m_bg_tilemap->set_scrolldx(0, 0);
	m_draw_bullets = &galaxold_state::darkplnt_draw_bullets;
}

VIDEO_START_MEMBER(galaxold_state,rescue)
{
	VIDEO_START_CALL_MEMBER(scrambold);

	m_draw_bullets = &galaxold_state::rescue_draw_bullets;

	m_draw_stars = &galaxold_state::rescue_draw_stars;

	m_draw_background = &galaxold_state::rescue_draw_background;
}

VIDEO_START_MEMBER(galaxold_state,minefld)
{
	VIDEO_START_CALL_MEMBER(scrambold);

	m_draw_bullets = &galaxold_state::rescue_draw_bullets;

	m_draw_stars = &galaxold_state::rescue_draw_stars;

	m_draw_background = &galaxold_state::minefld_draw_background;
}

VIDEO_START_MEMBER(galaxold_state,stratgyx)
{
	VIDEO_START_CALL_MEMBER(galaxold_plain);

	m_draw_background = &galaxold_state::stratgyx_draw_background;

	// level 3 tank bullets
	m_draw_bullets = &galaxold_state::scrambold_draw_bullets;
}

VIDEO_START_MEMBER(galaxold_state,ckongs)
{
	VIDEO_START_CALL_MEMBER(scrambold);

	m_modify_spritecode = &galaxold_state::mshuttle_modify_spritecode;
}

VIDEO_START_MEMBER(galaxold_state,mariner)
{
	VIDEO_START_CALL_MEMBER(galaxold_plain);

	m_draw_stars = &galaxold_state::mariner_draw_stars;

	m_draw_bullets = &galaxold_state::scrambold_draw_bullets;

	m_draw_background = &galaxold_state::mariner_draw_background;

	m_modify_charcode = &galaxold_state::mariner_modify_charcode;
}

VIDEO_START_MEMBER(galaxold_state,mimonkey)
{
	VIDEO_START_CALL_MEMBER(scrambold);

	m_modify_charcode   = &galaxold_state::mimonkey_modify_charcode;
	m_modify_spritecode = &galaxold_state::mimonkey_modify_spritecode;
}

VIDEO_START_MEMBER(galaxold_state,dkongjrm)
{
	VIDEO_START_CALL_MEMBER(galaxold_plain);

	m_modify_charcode   = &galaxold_state::pisces_modify_charcode;
	m_modify_spritecode = &galaxold_state::dkongjrm_modify_spritecode;

	m_spriteram2_present= 1;
}

VIDEO_START_MEMBER(galaxold_state,dkongjrmc)
{
	VIDEO_START_CALL_MEMBER(galaxold_plain);

	m_modify_charcode   = &galaxold_state::pisces_modify_charcode;
	m_modify_spritecode = &galaxold_state::dkongjrmc_modify_spritecode;
}

VIDEO_START_MEMBER(galaxold_state,scorpion)
{
	VIDEO_START_CALL_MEMBER(scrambold);

	m_modify_spritecode = &galaxold_state::batman2_modify_spritecode;
}

void galaxold_state::pisces_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs)
{
	*code |= (m_gfxbank[0] << 6);
}

VIDEO_START_MEMBER(galaxold_state,pisces)
{
	VIDEO_START_CALL_MEMBER(galaxold);

	m_modify_charcode   = &galaxold_state::pisces_modify_charcode;
	m_modify_spritecode = &galaxold_state::pisces_modify_spritecode;
}

#ifdef UNUSED_FUNCTION
void galaxold_state::theend_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y)
{
	/* same as Galaxian, but all bullets are yellow */
	for (int i = 0; i < 4; i++)
	{
		x--;

		if (cliprect.contains(x, y))
			bitmap.pix(y, x) = BULLETS_COLOR_BASE;
	}
}

VIDEO_START_MEMBER(galaxold_state,theend)
{
	VIDEO_START_CALL_MEMBER(galaxold);

	m_draw_bullets = &galaxold_state::theend_draw_bullets;
}
#endif

void galaxold_state::mooncrst_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs)
{
	if (m_gfxbank[2] && ((*code & 0x30) == 0x20))
	{
		*code = (*code & 0x0f) | (m_gfxbank[0] << 4) | (m_gfxbank[1] << 5) | 0x40;
	}
}

VIDEO_START_MEMBER(galaxold_state,mooncrst)
{
	VIDEO_START_CALL_MEMBER(galaxold);

	m_modify_charcode   = &galaxold_state::mooncrst_modify_charcode;
	m_modify_spritecode = &galaxold_state::mooncrst_modify_spritecode;
}




TILE_GET_INFO_MEMBER(galaxold_state::drivfrcg_get_tile_info)
{
	int code = m_videoram[tile_index];
	uint8_t x = tile_index & 0x1f;
	uint8_t color = m_attributesram[(x << 1) | 1] & 7;
	uint8_t bank = m_attributesram[(x << 1) | 1] & 0x30;

	code |= (bank << 4);
	color |= ((m_attributesram[(x << 1) | 1] & 0x40) >> 3);

	tileinfo.set(0, code, color, 0);
}

VIDEO_START_MEMBER(galaxold_state,drivfrcg)
{
	video_start_common();
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxold_state::drivfrcg_get_tile_info)), TILEMAP_SCAN_ROWS,8,8,32,32);

	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(32);

	m_modify_spritecode = &galaxold_state::mshuttle_modify_spritecode;
	m_modify_color = &galaxold_state::drivfrcg_modify_color;

	m_color_mask = 0xff;
}

VIDEO_START_MEMBER(galaxold_state,ad2083)
{
	video_start_common();
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxold_state::drivfrcg_get_tile_info)), TILEMAP_SCAN_ROWS,8,8,32,32);

	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(32);

	m_modify_spritecode = &galaxold_state::ad2083_modify_spritecode;

	m_draw_bullets = &galaxold_state::scrambold_draw_bullets;

	m_draw_background = &galaxold_state::ad2083_draw_background;

	m_color_mask = 7;
}


void galaxold_state::racknrol_tiles_bank_w(offs_t offset, uint8_t data)
{
	m_racknrol_tiles_bank[offset] = data;
	m_bg_tilemap->mark_all_dirty();
}

TILE_GET_INFO_MEMBER(galaxold_state::racknrol_get_tile_info)
{
	int code = m_videoram[tile_index];
	uint8_t x = tile_index & 0x1f;
	uint8_t color = m_attributesram[(x << 1) | 1] & 7;
	uint8_t bank = m_racknrol_tiles_bank[x] & 7;

	code |= (bank << 8);

	tileinfo.set(0, code, color, 0);
}

VIDEO_START_MEMBER(galaxold_state,racknrol)
{
	video_start_common();
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxold_state::racknrol_get_tile_info)), TILEMAP_SCAN_ROWS,8,8,32,32);

	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(32);

	m_color_mask = 0xff;
}


// Harem

TILE_GET_INFO_MEMBER(galaxold_state::harem_get_tile_info)
{
	int code = m_videoram[tile_index];
	uint8_t x = tile_index & 0x1f;
	uint8_t color = m_attributesram[(x << 1) | 1] & 7;
	uint8_t bank = BIT(m_racknrol_tiles_bank[0], x/4);    // 1 bit every 4 columns

	code  |= bank * 0x200;

	tileinfo.set(0, code, color, 0);
}

void galaxold_state::harem_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs)
{
	*code |= (m_gfxbank[0] << 7) | 0x40;
}

VIDEO_START_MEMBER(galaxold_state,harem)
{
	video_start_common();
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxold_state::harem_get_tile_info)), TILEMAP_SCAN_ROWS,8,8,32,32);
//  m_bg_tilemap->set_transparent_pen(0);   // opaque tilemap to get sky and sand colors

	m_bg_tilemap->set_scroll_cols(32);

	m_color_mask = (m_gfxdecode->gfx(0)->granularity() == 4) ? 7 : 3;

	m_modify_spritecode = &galaxold_state::harem_modify_spritecode;
}

TILE_GET_INFO_MEMBER(galaxold_state::dambustr_get_tile_info2)
{
	uint8_t x = tile_index & 0x1f;

	uint16_t code = m_dambustr_videoram2[tile_index];
	uint8_t color = m_attributesram[(x << 1) | 1] & m_color_mask;

	if (m_modify_charcode)
	{
		(this->*m_modify_charcode)(&code, x);
	}

	if (m_modify_color)
	{
		(this->*m_modify_color)(&color);
	}

	tileinfo.set(0, code, color, 0);
}

VIDEO_START_MEMBER(galaxold_state,dambustr)
{
	VIDEO_START_CALL_MEMBER(galaxold);

	m_dambustr_bg_split_line = 0;
	m_dambustr_bg_color_1 = 0;
	m_dambustr_bg_color_2 = 0;
	m_dambustr_bg_priority = 0;
	m_dambustr_char_bank = 0;
	m_stars_scrollpos = 0;

	m_draw_background = &galaxold_state::dambustr_draw_background;

	m_modify_charcode   = &galaxold_state::dambustr_modify_charcode;
	m_modify_spritecode = &galaxold_state::dambustr_modify_spritecode;

	m_draw_bullets = &galaxold_state::dambustr_draw_bullets;

	/* allocate the temporary bitmap for the background priority */
	m_dambustr_tmpbitmap = std::make_unique<bitmap_ind16>(m_screen->width(), m_screen->height());

	/* make a copy of the tilemap to emulate background priority */
	m_dambustr_videoram2 = std::make_unique<uint8_t[]>(0x0400);
	m_dambustr_tilemap2 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxold_state::dambustr_get_tile_info2)), TILEMAP_SCAN_ROWS,8,8,32,32);

	m_dambustr_tilemap2->set_transparent_pen(0);
}


void galaxold_state::galaxold_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

uint8_t galaxold_state::galaxold_videoram_r(offs_t offset)
{
	return m_videoram[offset];
}


void galaxold_state::galaxold_attributesram_w(offs_t offset, uint8_t data)
{
	if (m_attributesram[offset] != data)
	{
		if (offset & 0x01)
		{
			/* color change */
			int i;

			for (i = offset >> 1; i < 0x0400; i += 32)
				m_bg_tilemap->mark_tile_dirty(i);
		}
		else
		{
			if (m_modify_ypos)
			{
				(this->*m_modify_ypos)(&data);
			}

			m_bg_tilemap->set_scrolly(offset >> 1, data);
		}

		m_attributesram[offset] = data;
	}
}


void galaxold_state::galaxold_flip_screen_x_w(uint8_t data)
{
	if (m_flipscreen_x != (data & 0x01))
	{
		m_flipscreen_x = data & 0x01;

		m_bg_tilemap->set_flip((m_flipscreen_x ? TILEMAP_FLIPX : 0) | (m_flipscreen_y ? TILEMAP_FLIPY : 0));
	}
}

void galaxold_state::galaxold_flip_screen_y_w(uint8_t data)
{
	if (m_flipscreen_y != (data & 0x01))
	{
		m_flipscreen_y = data & 0x01;

		m_bg_tilemap->set_flip((m_flipscreen_x ? TILEMAP_FLIPX : 0) | (m_flipscreen_y ? TILEMAP_FLIPY : 0));
	}
}


#ifdef UNUSED_FUNCTION
void galaxold_state::gteikob2_flip_screen_x_w(uint8_t data)
{
	galaxold_flip_screen_x_w(~data);
}

void galaxold_state::gteikob2_flip_screen_y_w(uint8_t data)
{
	galaxold_flip_screen_y_w(~data);
}
#endif


void galaxold_state::hotshock_flip_screen_w(uint8_t data)
{
	galaxold_flip_screen_x_w(data);
	galaxold_flip_screen_y_w(data);
}


void galaxold_state::scrambold_background_enable_w(uint8_t data)
{
	m_background_enable = data & 0x01;
}

void galaxold_state::scrambold_background_red_w(uint8_t data)
{
	m_background_red = data & 0x01;
}

void galaxold_state::scrambold_background_green_w(uint8_t data)
{
	m_background_green = data & 0x01;
}

void galaxold_state::scrambold_background_blue_w(uint8_t data)
{
	m_background_blue = data & 0x01;
}


void galaxold_state::galaxold_stars_enable_w(uint8_t data)
{
	m_stars_on = data & 0x01;

	if (!m_stars_on)
	{
		m_stars_scrollpos = 0;
	}
}


void galaxold_state::darkplnt_bullet_color_w(uint8_t data)
{
	m_darkplnt_bullet_color = data & 0x01;
}



void galaxold_state::galaxold_gfxbank_w(offs_t offset, uint8_t data)
{
	if (m_gfxbank[offset] != data)
	{
		m_gfxbank[offset] = data;

		m_bg_tilemap->mark_all_dirty();
	}
}

void galaxold_state::dambustr_bg_split_line_w(uint8_t data)
{
	m_dambustr_bg_split_line = data;
}


void galaxold_state::dambustr_bg_color_w(uint8_t data)
{
	m_dambustr_bg_color_1 = (BIT(data,2)<<2) | (BIT(data,1)<<1) | BIT(data,0);
	m_dambustr_bg_color_2 = (BIT(data,6)<<2) | (BIT(data,5)<<1) | BIT(data,4);
	m_dambustr_bg_priority = BIT(data,3);
	m_dambustr_char_bank = BIT(data,7);
	m_bg_tilemap->mark_all_dirty();
}



/* character banking functions */

void galaxold_state::mooncrst_modify_charcode(uint16_t *code, uint8_t x)
{
	if (m_gfxbank[2] && ((*code & 0xc0) == 0x80))
	{
		*code = (*code & 0x3f) | (m_gfxbank[0] << 6) | (m_gfxbank[1] << 7) | 0x0100;
	}
}

void galaxold_state::pisces_modify_charcode(uint16_t *code, uint8_t x)
{
	*code |= (m_gfxbank[0] << 8);
}

void galaxold_state::mimonkey_modify_charcode(uint16_t *code, uint8_t x)
{
	*code |= (m_gfxbank[0] << 8) | (m_gfxbank[2] << 9);
}

void galaxold_state::mariner_modify_charcode(uint16_t *code, uint8_t x)
{
	uint8_t *prom;


	/* bit 0 of the PROM controls character banking */

	prom = memregion("user2")->base();

	*code |= ((prom[x] & 0x01) << 8);
}

void galaxold_state::dambustr_modify_charcode(uint16_t *code, uint8_t x)
{
	if (m_dambustr_char_bank == 0) { // text mode
		*code |= 0x0300;
	}
	else {              // graphics mode
		if (x == 28)        // only line #28 stays in text mode
			*code |= 0x0300;
		else
			*code &= 0x00ff;
	};
}



/* sprite banking functions */

void galaxold_state::mshuttle_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs)
{
	*code |= ((spriteram[offs + 2] & 0x30) << 2);
}


void galaxold_state::mimonkey_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs)
{
	*code |= (m_gfxbank[0] << 6) | (m_gfxbank[2] << 7);
}

void galaxold_state::batman2_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs)
{
	/* only the upper 64 sprites are used */
	*code |= 0x40;
}

void galaxold_state::dkongjrm_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs)
{
	/* No x flip */
	*code = (spriteram[offs + 1] & 0x7f) | 0x80;
	*flipx = 0;
}

void galaxold_state::dkongjrmc_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs)
{
	*code = (spriteram[offs + 1] & 0x7f) | 0x80; // bit 6 is also X flip
}

void galaxold_state::ad2083_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs)
{
	/* No x flip */
	*code = (spriteram[offs + 1] & 0x7f) | ((spriteram[offs + 2] & 0x30) << 2);
	*flipx = 0;
}

void galaxold_state::dambustr_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs)
{
	*code += 0x40;
}


/* color PROM mapping functions */

void galaxold_state::drivfrcg_modify_color(uint8_t *color)
{
	*color = ((*color & 0x40) >> 3) | (*color & 7);
}


/* y position mapping functions */


/* bullet drawing functions */

void galaxold_state::galaxold_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y)
{
	for (int i = 0; i < 4; i++)
	{
		x--;

		if (cliprect.contains(x, y))
		{
			/* yellow missile, white shells (this is the terminology on the schematics) */
			int const color = ((offs == 7*4) ? BULLETS_COLOR_BASE : BULLETS_COLOR_BASE + 1);

			bitmap.pix(y, x) = color;
		}
	}
}

void galaxold_state::scrambold_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y)
{
	if (m_flipscreen_x)  x++;

	x = x - 6;

	if (cliprect.contains(x, y))
		/* yellow bullets */
		bitmap.pix(y, x) = BULLETS_COLOR_BASE;
}

void galaxold_state::darkplnt_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y)
{
	if (m_flipscreen_x)  x++;

	x = x - 6;

	if (cliprect.contains(x, y))
		bitmap.pix(y, x) = 32 + m_darkplnt_bullet_color;
}

void galaxold_state::dambustr_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y)
{
	if (flip_screen_x())  x++;

	x = x - 6;

	/* bullets are 2 pixels wide */
	for (int i = 0; i < 2; i++)
	{
		int color;
		if (offs < 4*4)
		{
			color = BULLETS_COLOR_BASE;
			y--;
		}
		else
		{
			color = BULLETS_COLOR_BASE + 1;
			x--;
		}

		if (cliprect.contains(x, y))
			bitmap.pix(y, x) = color;
	}
}

void galaxold_state::rescue_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y)
{
	if (flip_screen_x())  x++;

	x = x - 6;

	int color = BULLETS_COLOR_BASE;

	/* bullets are 2 pixels square */
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
		{
			if (cliprect.contains(x+i, y+j))
				bitmap.pix(y+j, x+i) = color;
		}
}


/* background drawing functions */

void galaxold_state::galaxold_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* plain black background */
	bitmap.fill(0, cliprect);
}

void galaxold_state::scrambold_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_background_enable)
		bitmap.fill(BACKGROUND_COLOR_BASE, cliprect);
	else
		bitmap.fill(0, cliprect);
}

void galaxold_state::ad2083_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int color = (m_background_blue << 2) | (m_background_green << 1) | m_background_red;

	bitmap.fill(BACKGROUND_COLOR_BASE + color, cliprect);
}

void galaxold_state::stratgyx_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t x;
	uint8_t *prom;
	int base = BACKGROUND_COLOR_BASE;


	/* the background PROM is connected the following way:

	   bit 0 = 0 enables the blue gun if BCB is asserted
	   bit 1 = 0 enables the red gun if BCR is asserted and
	             the green gun if BCG is asserted
	   bits 2-7 are unconnected */

	prom = memregion("user1")->base();

	for (x = 0; x < 32; x++)
	{
		int sx,color;


		color = 0;

		if ((~prom[x] & 0x02) && m_background_red)   color |= 0x01;
		if ((~prom[x] & 0x02) && m_background_green) color |= 0x02;
		if ((~prom[x] & 0x01) && m_background_blue)  color |= 0x04;

		if (m_flipscreen_x)
			sx = 8 * (31 - x);
		else
			sx = 8 * x;

		bitmap.plot_box(sx, 0, 8, 256, base + color);
	}
}

void galaxold_state::minefld_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_background_enable)
	{
		int base = BACKGROUND_COLOR_BASE;
		int x;


		for (x = 0; x < 128; x++)
			bitmap.plot_box(x,       0, 1, 256, base + x);

		for (x = 0; x < 120; x++)
			bitmap.plot_box(x + 128, 0, 1, 256, base + x + 128);

		bitmap.plot_box(248, 0, 16, 256, base);
	}
	else
		bitmap.fill(0, cliprect);
}

void galaxold_state::rescue_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_background_enable)
	{
		int base = BACKGROUND_COLOR_BASE;
		int x;

		for (x = 0; x < 128; x++)
			bitmap.plot_box(x,       0, 1, 256, base + x);

		for (x = 0; x < 120; x++)
			bitmap.plot_box(x + 128, 0, 1, 256, base + x + 8);

		bitmap.plot_box(248, 0, 16, 256, base);
	}
	else
		bitmap.fill(0, cliprect);
}

void galaxold_state::mariner_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int base = BACKGROUND_COLOR_BASE;
	uint8_t x;
	uint8_t *prom;


	/* the background PROM contains the color codes for each 8 pixel
	   line (column) of the screen.  The first 0x20 bytes for unflipped,
	   and the 2nd 0x20 bytes for flipped screen. */

	prom = memregion("user1")->base();

	if (m_flipscreen_x)
	{
		for (x = 0; x < 32; x++)
		{
			int color;

			if (x == 0)
				color = 0;
			else
				color = prom[0x20 + x - 1];

			bitmap.plot_box(8 * (31 - x), 0, 8, 256, base + color);
		}
	}
	else
	{
		for (x = 0; x < 32; x++)
		{
			int color;

			if (x == 31)
				color = 0;
			else
				color = prom[x + 1];

			bitmap.plot_box(8 * x, 0, 8, 256, base + color);
		}
	}
}

void galaxold_state::dambustr_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int base = BACKGROUND_COLOR_BASE;
	int col1 = base + m_dambustr_bg_color_1;
	int col2 = base + m_dambustr_bg_color_2;

	if (flip_screen_x())
	{
		bitmap.plot_box(  0, 0, 256-m_dambustr_bg_split_line, 256, col2);
		bitmap.plot_box(256-m_dambustr_bg_split_line, 0, m_dambustr_bg_split_line, 256, col1);
	}
	else
	{
		bitmap.plot_box(  0, 0, 256-m_dambustr_bg_split_line, 256, col1);
		bitmap.plot_box(256-m_dambustr_bg_split_line, 0, m_dambustr_bg_split_line, 256, col2);
	}

}

void galaxold_state::dambustr_draw_upper_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (flip_screen_x())
	{
		rectangle clip(254 - m_dambustr_bg_split_line, m_dambustr_bg_split_line, 0, 255);
		copybitmap(bitmap, *m_dambustr_tmpbitmap, 0, 0, 0, 0, clip);
	}
	else
	{
		rectangle clip(0, 254 - m_dambustr_bg_split_line, 0, 255);
		copybitmap(bitmap, *m_dambustr_tmpbitmap, 0, 0, 0, 0, clip);
	}
}



/* star drawing functions */

void galaxold_state::galaxold_init_stars(int colors_offset)
{
	struct star_gold *stars = m_stars;
	int i;
	int total_stars;
	uint32_t generator;
	int x,y;


	m_stars_on = 0;
	m_stars_blink_state = 0;
	m_stars_blink_timer = timer_alloc(FUNC(galaxold_state::stars_blink_callback), this);
	m_stars_scroll_timer = timer_alloc(FUNC(galaxold_state::stars_scroll_callback), this);
	m_timer_adjusted = 0;
	m_stars_colors_start = colors_offset;

	for (i = 0;i < 64;i++)
	{
		int bits,r,g,b;
		static const int map[4] = { 0x00, 0x88, 0xcc, 0xff };


		bits = (i >> 0) & 0x03;
		r = map[bits];
		bits = (i >> 2) & 0x03;
		g = map[bits];
		bits = (i >> 4) & 0x03;
		b = map[bits];
		m_palette->set_pen_color(colors_offset+i,r,g,b);
	}


	/* precalculate the star background */

	total_stars = 0;
	generator = 0;

	for (y = 0;y < 256;y++)
	{
		for (x = 0;x < 512;x++)
		{
			uint32_t bit0;


			bit0 = ((~generator >> 16) & 0x01) ^ ((generator >> 4) & 0x01);

			generator = (generator << 1) | bit0;

			if (((~generator >> 16) & 0x01) && (generator & 0xff) == 0xff)
			{
				int color;


				color = (~(generator >> 8)) & 0x3f;
				if (color)
				{
					stars[total_stars].x = x;
					stars[total_stars].y = y;
					stars[total_stars].color = color;

					total_stars++;
				}
			}
		}
	}

	if (total_stars != STAR_COUNT)
	{
		fatalerror("total_stars = %d, STAR_COUNT = %d\n",total_stars,STAR_COUNT);
	}
}

void galaxold_state::plot_star(bitmap_ind16 &bitmap, int x, int y, int color, const rectangle &cliprect)
{
	if (m_flipscreen_x)
		x = 255 - x;

	if (m_flipscreen_y)
		y = 255 - y;

	if (cliprect.contains(x, y))
		bitmap.pix(y, x) = m_stars_colors_start + color;
}

void galaxold_state::noop_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
}

void galaxold_state::galaxold_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	struct star_gold *stars = m_stars;
	int offs;


	if (!m_timer_adjusted)
	{
		start_stars_scroll_timer();
		m_timer_adjusted = 1;
	}


	for (offs = 0;offs < STAR_COUNT;offs++)
	{
		int x,y;


		x = ((stars[offs].x +   m_stars_scrollpos) & 0x01ff) >> 1;
		y = ( stars[offs].y + ((m_stars_scrollpos + stars[offs].x) >> 9)) & 0xff;

		if ((y & 0x01) ^ ((x >> 3) & 0x01))
		{
			plot_star(bitmap, x, y, stars[offs].color, cliprect);
		}
	}
}

void galaxold_state::scrambold_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	struct star_gold *stars = m_stars;
	int offs;


	if (!m_timer_adjusted)
	{
		start_stars_blink_timer(100000, 10000, 0.00001);
		m_timer_adjusted = 1;
	}


	for (offs = 0;offs < STAR_COUNT;offs++)
	{
		int x,y;


		x = stars[offs].x >> 1;
		y = stars[offs].y;

		if ((y & 0x01) ^ ((x >> 3) & 0x01))
		{
			/* determine when to skip plotting */
			switch (m_stars_blink_state & 0x03)
			{
			case 0:
				if (!(stars[offs].color & 0x01))  continue;
				break;
			case 1:
				if (!(stars[offs].color & 0x04))  continue;
				break;
			case 2:
				if (!(stars[offs].y & 0x02))  continue;
				break;
			case 3:
				/* always plot */
				break;
			}

			plot_star(bitmap, x, y, stars[offs].color, cliprect);
		}
	}
}

void galaxold_state::rescue_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	struct star_gold *stars = m_stars;
	int offs;


	/* same as Scramble, but only top (left) half of screen */

	if (!m_timer_adjusted)
	{
		start_stars_blink_timer(100000, 10000, 0.00001);
		m_timer_adjusted = 1;
	}


	for (offs = 0;offs < STAR_COUNT;offs++)
	{
		int x,y;


		x = stars[offs].x >> 1;
		y = stars[offs].y;

		if ((x < 128) && ((y & 0x01) ^ ((x >> 3) & 0x01)))
		{
			/* determine when to skip plotting */
			switch (m_stars_blink_state & 0x03)
			{
			case 0:
				if (!(stars[offs].color & 0x01))  continue;
				break;
			case 1:
				if (!(stars[offs].color & 0x04))  continue;
				break;
			case 2:
				if (!(stars[offs].y & 0x02))  continue;
				break;
			case 3:
				/* always plot */
				break;
			}

			plot_star(bitmap, x, y, stars[offs].color, cliprect);
		}
	}
}

void galaxold_state::mariner_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	struct star_gold *stars = m_stars;
	int offs;
	uint8_t *prom;


	if (!m_timer_adjusted)
	{
		start_stars_scroll_timer();
		m_timer_adjusted = 1;
	}


	/* bit 2 of the PROM controls star visibility */

	prom = memregion("user2")->base();

	for (offs = 0;offs < STAR_COUNT;offs++)
	{
		int x,y;


		x = ((stars[offs].x +   -m_stars_scrollpos) & 0x01ff) >> 1;
		y = ( stars[offs].y + ((-m_stars_scrollpos + stars[offs].x) >> 9)) & 0xff;

		if ((y & 0x01) ^ ((x >> 3) & 0x01))
		{
			if (prom[(x/8 + 1) & 0x1f] & 0x04)
			{
				plot_star(bitmap, x, y, stars[offs].color, cliprect);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(galaxold_state::stars_blink_callback)
{
	m_stars_blink_state++;
}

void galaxold_state::start_stars_blink_timer(double ra, double rb, double c)
{
	/* calculate the period using the formula given in the 555 datasheet */

	int period_in_ms = 693 * (ra + 2.0 * rb) * c;

	m_stars_blink_timer->adjust(attotime::from_msec(period_in_ms), 0, attotime::from_msec(period_in_ms));
}


TIMER_CALLBACK_MEMBER(galaxold_state::stars_scroll_callback)
{
	if (m_stars_on)
	{
		m_stars_scrollpos++;
	}
}

void galaxold_state::start_stars_scroll_timer()
{
	m_stars_scroll_timer->adjust(m_screen->frame_period(), 0, m_screen->frame_period());
}



TILE_GET_INFO_MEMBER(galaxold_state::get_tile_info)
{
	uint8_t x = tile_index & 0x1f;

	uint16_t code = m_videoram[tile_index];
	uint8_t color = m_attributesram[(x << 1) | 1] & m_color_mask;

	if (m_modify_charcode)
	{
		(this->*m_modify_charcode)(&code, x);
	}

	if (m_modify_color)
	{
		(this->*m_modify_color)(&color);
	}

	tileinfo.set(0, code, color, 0);
}


void galaxold_state::draw_bullets_common(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;


	for (offs = 0;offs < m_bulletsram.bytes();offs += 4)
	{
		uint8_t sx,sy;

		sy = 255 - m_bulletsram[offs + 1];
		sx = 255 - m_bulletsram[offs + 3];

		if (m_flipscreen_y)  sy = 255 - sy;

		(this->*m_draw_bullets)(bitmap, cliprect, offs, sx, sy);
	}
}


void galaxold_state::draw_sprites(bitmap_ind16 &bitmap, uint8_t *spriteram, size_t spriteram_size)
{
	const rectangle spritevisiblearea((0+m_leftclip)*8+1, 32*8-1, 2*8,   30*8-1);
	const rectangle spritevisibleareaflipx(0*8, (32-m_leftclip)*8-2, 2*8, 30*8-1);

	int offs;


	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		uint8_t sx,sy,color;
		int flipx,flipy,code;


		sx = spriteram[offs + 3] + 1;   /* the existence of +1 is supported by a LOT of games */
		sy = spriteram[offs];           /* Anteater, Mariner, for example */
		flipx = spriteram[offs + 1] & 0x40;
		flipy = spriteram[offs + 1] & 0x80;
		code = spriteram[offs + 1] & 0x3f;
		color = spriteram[offs + 2] & m_color_mask;

		if (m_modify_spritecode)
		{
			(this->*m_modify_spritecode)(spriteram, &code, &flipx, &flipy, offs);
		}

		if (m_modify_color)
		{
			(this->*m_modify_color)(&color);
		}

		if (m_modify_ypos)
		{
			(this->*m_modify_ypos)(&sy);
		}

		if (m_flipscreen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (m_flipscreen_y)
		{
			flipy = !flipy;
		}
		else
		{
			sy = 240 - sy;
		}


		/* In at least Amidar Turtles, sprites #0, #1 and #2 need to be moved */
		/* down (left) one pixel to be positioned correctly. */
		/* Note that the adjustment must be done AFTER handling flipscreen, thus */
		/* proving that this is a hardware related "feature" */

		if (offs < 3*4)  sy++;


		m_gfxdecode->gfx(1)->transpen(bitmap,m_flipscreen_x ? spritevisibleareaflipx : spritevisiblearea,
				code,color,
				flipx,flipy,
				sx,sy,0);
	}
}


uint32_t galaxold_state::screen_update_galaxold(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	(this->*m_draw_background)(screen, bitmap, cliprect);

	if (m_stars_on)
	{
		(this->*m_draw_stars)(bitmap, cliprect);
	}


	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if (m_draw_bullets)
	{
		draw_bullets_common(bitmap, cliprect);
	}


	draw_sprites(bitmap, m_spriteram, m_spriteram.bytes());

	if (m_spriteram2_present)
	{
		draw_sprites(bitmap, m_spriteram2, m_spriteram2.bytes());
	}
	return 0;
}


uint32_t galaxold_state::screen_update_dambustr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, j;
	uint8_t color;

	(this->*m_draw_background)(screen, bitmap, cliprect);

	if (m_stars_on)
	{
		(this->*m_draw_stars)(bitmap, cliprect);
	}

	/* save the background for drawing it again later, if background has priority over characters */
	copybitmap(*m_dambustr_tmpbitmap, bitmap, 0, 0, 0, 0, m_dambustr_tmpbitmap->cliprect());

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if (m_draw_bullets)
	{
		draw_bullets_common(bitmap, cliprect);
	}

	draw_sprites(bitmap, m_spriteram, m_spriteram.bytes());

	if (m_dambustr_bg_priority)
	{
		/* draw the upper part of the background, as it has priority */
		dambustr_draw_upper_background(bitmap, cliprect);

		/* only rows with color code > 3 are stronger than the background */
		memset(m_dambustr_videoram2.get(), 0x20, 0x0400);
		for (i=0; i<32; i++) {
			color = m_attributesram[(i << 1) | 1] & m_color_mask;
			if (color > 3) {
				for (j=0; j<32; j++)
					m_dambustr_videoram2[32*j+i] = m_videoram[32*j+i];
			};
		};
		m_dambustr_tilemap2->mark_all_dirty();
		m_dambustr_tilemap2->draw(screen, bitmap, cliprect, 0, 0);
	};

	return 0;
}
