// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*******************************************************************

Namco System 86 Video Hardware

*******************************************************************/

#include "emu.h"
#include "namcos86.h"

#include "video/resnet.h"
#include "screen.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Rolling Thunder has two palette PROMs (512x8 and 512x4) and two 2048x8
  lookup table PROMs.
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

***************************************************************************/

void namcos86_state::namcos86_palette(palette_device &palette)
{
	u8 const *color_prom = memregion("proms")->base();
	static constexpr int resistances[4] = { 2200, 1000, 470, 220 };

	// compute the color output resistor weights
	double rweights[4], gweights[4], bweights[4];
	compute_resistor_weights(0, 255, -1.0,
			4, &resistances[0], rweights, 0, 0,
			4, &resistances[0], gweights, 0, 0,
			4, &resistances[0], bweights, 0, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 512; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		bit3 = BIT(color_prom[i], 3);
		int const r = combine_weights(rweights, bit0, bit1, bit2, bit3);

		// green component
		bit0 = BIT(color_prom[i], 4);
		bit1 = BIT(color_prom[i], 5);
		bit2 = BIT(color_prom[i], 6);
		bit3 = BIT(color_prom[i], 7);
		int const g = combine_weights(gweights, bit0, bit1, bit2, bit3);

		// blue component
		bit0 = BIT(color_prom[i | 0x200], 0);
		bit1 = BIT(color_prom[i | 0x200], 1);
		bit2 = BIT(color_prom[i | 0x200], 2);
		bit3 = BIT(color_prom[i | 0x200], 3);
		int const b = combine_weights(bweights, bit0, bit1, bit2, bit3);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x400;

	// tiles lookup table
	for (int i = 0; i < 2048; i++)
		palette.set_pen_indirect(i, *color_prom++);

	// sprites lookup table
	for (int i = 0; i < 2048; i++)
		palette.set_pen_indirect(2048 + i, 256 + *color_prom++);

	// color_prom now points to the beginning of the tile address decode PROM

	m_tile_address_prom = color_prom; // we'll need this at run time
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

void namcos86_state::tile_cb_0(u8 layer, u8 &gfxno, u32 &code)
{
	code = (code & 0xff) | ((((m_tile_address_prom[((layer & 1) << 4) + (((code >> 8) & 0x03) << 2)] & 0x0e) >> 1) << 8) | (m_tilebank << 11));
}

void namcos86_state::tile_cb_1(u8 layer, u8 &gfxno, u32 &code)
{
	code = (code & 0xff) | (((m_tile_address_prom[((layer & 1) << 4) + ((code >> 8) & 0x03)] & 0xe0) >> 5) << 8);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void namcos86_state::video_start()
{
	save_item(NAME(m_tilebank));
	save_item(NAME(m_backcolor));
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void namcos86_state::tilebank_select_w(offs_t offset, u8 data)
{
	u32 const bit = BIT(offset, 10);
	if (m_tilebank != bit)
	{
		m_tilebank = bit;
		m_tilegen[0]->mark_all_dirty();
	}
}

void namcos86_state::backcolor_w(u8 data)
{
	m_backcolor = data;
}


/***************************************************************************

  Display refresh

***************************************************************************/

u32 namcos86_state::sprite_pri_cb(u8 attr1, u8 attr2)
{
	int const priority = (attr2 & 0xe0) >> 5;
	return (0xff << (priority + 1)) & 0xff;
}

u32 namcos86_state::sprite_bank_cb(u32 code, u32 bank)
{
	int const bank_sprites = m_spritegen->gfx(0)->elements() / 8;
	code &= bank_sprites - 1;
	return code + (bank * bank_sprites);
}

u32 namcos86_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilegen[0]->init_scroll(flip_screen());
	m_tilegen[1]->init_scroll(flip_screen());

	screen.priority().fill(0, cliprect);

	bitmap.fill(m_tilegen[0]->gfx(0)->colorbase() + 8*m_backcolor+7, cliprect);

	for (int layer = 0; layer < 8; layer++)
	{
		for (int i = 3; i >= 0; i--)
		{
			if (((m_tilegen[i >> 1]->xscroll_r(i & 1) & 0x0e00) >> 9) == layer)
				m_tilegen[i >> 1]->draw(screen, bitmap, cliprect, i & 1, 0, layer, 0);
		}
	}

	m_spritegen->draw_sprites(screen, bitmap, cliprect);
	return 0;
}


void namcos86_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_spritegen->copy_sprites();
	}
}
