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
	const uint8_t *color_prom = memregion("proms")->base();
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
	save_item(NAME(m_copy_sprites));
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void namcos86_state::tilebank_select_w(offs_t offset, uint8_t data)
{
	uint32_t const bit = BIT(offset, 10);
	if (m_tilebank != bit)
	{
		m_tilebank = bit;
		m_tilegen[0]->mark_all_dirty();
	}
}

void namcos86_state::backcolor_w(uint8_t data)
{
	m_backcolor = data;
}


void namcos86_state::spriteram_w(offs_t offset, uint8_t data)
{
	m_spriteram[offset] = data;

	// a write to this offset tells the sprite chip to buffer the sprite list
	if (offset == 0x1ff2)
		m_copy_sprites = true;
}


/***************************************************************************

  Display refresh

***************************************************************************/

/*
sprite format:

0-3  scratchpad RAM
4-9  CPU writes here, hardware copies from here to 10-15
10   xx------  X size (16, 8, 32, 4)
10   --x-----  X flip
10   ---xx---  X offset inside 32x32 tile
10   -----xxx  tile bank
11   xxxxxxxx  tile number
12   xxxxxxx-  color
12   -------x  X position MSB
13   xxxxxxxx  X position
14   xxx-----  priority
14   ---xx---  Y offset inside 32x32 tile
14   -----xx-  Y size (16, 8, 32, 4)
14   -------x  Y flip
15   xxxxxxxx  Y position
*/

void namcos86_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint8_t *source = &m_spriteram[0x2000-0x20]; // the last is NOT a sprite
	const uint8_t *finish = &m_spriteram[0x1800];
	gfx_element *gfx = m_gfxdecode->gfx(0);

	int const sprite_xoffs = m_spriteram[0x1ff5] + ((m_spriteram[0x1ff4] & 1) << 8);
	int const sprite_yoffs = m_spriteram[0x1ff7];

	int const bank_sprites = m_gfxdecode->gfx(0)->elements() / 8;

	static const int sprite_size[4] = { 16, 8, 32, 4 };

	while (source >= finish)
	{
		int const attr1 = source[10];
		int const attr2 = source[14];
		int color = source[12];
		bool flipx = BIT(attr1, 5);
		bool flipy = BIT(attr2, 0);
		int const sizex = sprite_size[(attr1 & 0xc0) >> 6];
		int const sizey = sprite_size[(attr2 & 0x06) >> 1];
		int const tx = (attr1 & 0x18) & (~(sizex - 1));
		int const ty = (attr2 & 0x18) & (~(sizey - 1));
		int sx = source[13] + ((color & 0x01) << 8);
		int sy = -source[15] - sizey;
		int sprite = source[11];
		int const sprite_bank = attr1 & 7;
		int const priority = (source[14] & 0xe0) >> 5;
		uint32_t const pri_mask = (0xff << (priority + 1)) & 0xff;

		sprite &= bank_sprites - 1;
		sprite += sprite_bank * bank_sprites;
		color = color >> 1;

		sx += sprite_xoffs;
		sy -= sprite_yoffs;

		if (flip_screen())
		{
			sx = -sx - sizex;
			sy = -sy - sizey;
			flipx = !flipx;
			flipy = !flipy;
		}

		sy++;   // sprites are buffered and delayed by one scanline

		gfx->set_source_clip(tx, sizex, ty, sizey);
		gfx->prio_transpen(bitmap, cliprect,
				sprite,
				color,
				flipx, flipy,
				sx & 0x1ff,
				((sy + 16) & 0xff) - 16,
				screen.priority(), pri_mask, 0xf);

		source -= 0x10;
	}
}


uint32_t namcos86_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// flip screen is embedded in the sprite control registers
	flip_screen_set(BIT(m_spriteram[0x1ff6], 0));
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

	draw_sprites(screen,bitmap,cliprect);
	return 0;
}


void namcos86_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		if (m_copy_sprites)
		{
			for (int i = 0x1800; i < 0x2000; i += 16)
			{
				for (int j = 10; j < 16; j++)
					m_spriteram[i + j] = m_spriteram[i + j - 6];
			}

			m_copy_sprites = false;
		}
	}
}
