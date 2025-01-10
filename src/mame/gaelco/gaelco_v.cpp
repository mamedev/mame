// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

  Gaelco Type 1 Video Hardware

  Functions to emulate the video hardware of the machine

  TODO:
  verify priority implementations

  NOTE:
  if Squash fails a protection check it will leave bad 'Insert Coin' text
  on the screen after a continue, this is not a sprite emulation bug, the
  machine expects a 68k clock of around 10Mhz and a refresh of around 57.4
  to pass the protection, see notes in main driver file.

***************************************************************************/

#include "emu.h"
#include "gaelco.h"
#include "screen.h"

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- -------x | flip x
      0  | -------- ------x- | flip y
      0  | xxxxxxxx xxxxxx-- | code
      1  | -------- --xxxxxx | color
      1  | -------- xx------ | priority
      1  | xxxxxxxx -------- | not used
*/

template<int Layer>
TILE_GET_INFO_MEMBER(gaelco_state::get_tile_info)
{
	int const data = m_videoram[(Layer * 0x1000 / 2) + (tile_index << 1)];
	int const data2 = m_videoram[(Layer * 0x1000 / 2) + (tile_index << 1) + 1];
	int const code = ((data & 0xfffc) >> 2);

	tileinfo.category = (data2 >> 6) & 0x03;

	tileinfo.set(1, 0x4000 + code, data2 & 0x3f, TILE_FLIPYX(data & 0x03));
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

void gaelco_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 const old = m_videoram[offset];
	COMBINE_DATA(&m_videoram[offset]);
	if (old != m_videoram[offset])
		m_tilemap[offset >> 11]->mark_tile_dirty(((offset << 1) & 0x0fff) >> 2);
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(gaelco_state,bigkarnk)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaelco_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaelco_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tilemap[0]->set_transmask(0, 0xff01, 0x00ff); // pens 1-7 opaque, pens 0, 8-15 transparent
	m_tilemap[1]->set_transmask(0, 0xff01, 0x00ff); // pens 1-7 opaque, pens 0, 8-15 transparent
}

VIDEO_START_MEMBER(squash_state,squash)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(squash_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(squash_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tilemap[0]->set_transmask(0, 0xff01, 0x00ff); // pens 1-7 opaque, pens 0, 8-15 transparent
	m_tilemap[1]->set_transmask(0, 0xff01, 0x00ff); // pens 1-7 opaque, pens 0, 8-15 transparent

	m_sprite_palette_force_high = 0x3c;
}

VIDEO_START_MEMBER(gaelco_state,maniacsq)
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaelco_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaelco_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	// it is possible Maniac Square hardware also has more complex priority handling, but does not use it
	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);
}


/***************************************************************************

    Sprites

***************************************************************************/

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | -----xxx -------- | not used
      0  | ----x--- -------- | sprite size
      0  | --xx---- -------- | sprite priority
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used
      2  | -------x xxxxxxxx | x position
      2  | -xxxxxx- -------- | sprite color
      3  | -------- ------xx | sprite code (8x8 quadrant)
      3  | xxxxxxxx xxxxxx-- | sprite code
*/

void gaelco_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	static const int x_offset[2] = {0x0,0x2};
	static const int y_offset[2] = {0x0,0x1};

	for (int i = 0x800 - 4 - 1; i >= 3; i -= 4)
	{
		int const sx = m_spriteram[i + 2] & 0x01ff;
		int const sy = (240 - (m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = m_spriteram[i + 3];
		int const color = (m_spriteram[i + 2] & 0x7e00) >> 9;
		int const attr = (m_spriteram[i] & 0xfe00) >> 9;
		int priority = (m_spriteram[i] & 0x3000) >> 12;

		bool const xflip = BIT(attr, 5);
		bool const yflip = BIT(attr, 6);

		/* palettes 0x38-0x3f are used for high priority sprites in Big Karnak
		   the same logic in Squash causes player sprites to be drawn over the
		   pixels that form the glass play area.

		   Is this accurate, or just exposing a different flaw in the priority
		   handling? */
		if (color >= m_sprite_palette_force_high)
			priority = 4;

		u32 pri_mask = 0;
		switch (priority)
		{
			case 0: pri_mask = 0xff00; break;  // above everything?
			case 1: pri_mask = 0xff00 | 0xf0f0; break;
			case 2: pri_mask = 0xff00 | 0xf0f0 | 0xcccc; break;
			case 3: pri_mask = 0xff00 | 0xf0f0 | 0xcccc | 0xaaaa; break;
			default:
			case 4: pri_mask = 0; break;
		}

		int spr_size;
		if (attr & 0x04)
			spr_size = 1;
		else
		{
			spr_size = 2;
			number &= (~3);
		}

		for (int y = 0; y < spr_size; y++)
		{
			for (int x = 0; x < spr_size; x++)
			{
				int ex = xflip ? (spr_size - 1 - x) : x;
				int ey = yflip ? (spr_size - 1 - y) : y;

				gfx->prio_transpen(bitmap, cliprect,
						number + x_offset[ex] + y_offset[ey],
						color,
						xflip, yflip,
						sx - 0x0f + x * 8, sy + y * 8,
						screen.priority(), pri_mask, 0);
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

u32 gaelco_state::screen_update_maniacsq(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* set scroll registers */
	m_tilemap[0]->set_scrolly(0, m_vregs[0]);
	m_tilemap[0]->set_scrollx(0, m_vregs[1] + 4);
	m_tilemap[1]->set_scrolly(0, m_vregs[2]);
	m_tilemap[1]->set_scrollx(0, m_vregs[3]);

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_tilemap[1]->draw(screen, bitmap, cliprect, 3, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 3, 0);

	m_tilemap[1]->draw(screen, bitmap, cliprect, 2, 1);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 2, 1);

	m_tilemap[1]->draw(screen, bitmap, cliprect, 1, 2);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 1, 2);

	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 4);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(screen, bitmap, cliprect);
	return 0;
}

u32 squash_state::screen_update_thoop(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* set scroll registers */
	m_tilemap[0]->set_scrolly(0, m_vregs[0]);
	m_tilemap[0]->set_scrollx(0, m_vregs[1] + 4);
	m_tilemap[1]->set_scrolly(0, m_vregs[2]);
	m_tilemap[1]->set_scrollx(0, m_vregs[3]);

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	// the priority handling differs from Big Karnak (or the Big Karnak implementation isn't correct)
	// the games only make minimal use of the priority features.

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 0);

	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 0);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 1);
	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 1);

	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 1);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 1);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 2);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 2);

	// sprites are sandwiched in here

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 4);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 4);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 8);
	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 8);

	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 8);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 8);

	draw_sprites(screen, bitmap, cliprect);
	return 0;
}

u32 bigkarnk_state::screen_update_bigkarnk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* set scroll registers */
	m_tilemap[0]->set_scrolly(0, m_vregs[0]);
	m_tilemap[0]->set_scrollx(0, m_vregs[1] + 4);
	m_tilemap[1]->set_scrolly(0, m_vregs[2]);
	m_tilemap[1]->set_scrollx(0, m_vregs[3]);

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 1);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 1);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 1);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 1);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 2);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 2);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 2);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 2);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 4);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 4);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 4);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 4);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 8);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 8);

	draw_sprites(screen, bitmap, cliprect);
	return 0;
}
