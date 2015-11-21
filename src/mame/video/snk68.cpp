// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Acho A. Tang, Nicola Salmoria
/***************************************************************************

    SNK 68000 video routines

Notes:
    Search & Rescue uses Y flip on sprites only.
    Street Smart uses X flip on sprites only.

    Seems to be controlled in same byte as flipscreen.

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/snk68.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(snk68_state::get_pow_tile_info)
{
	int tile = m_fg_tile_offset + (m_pow_fg_videoram[2*tile_index] & 0xff);
	int color = m_pow_fg_videoram[2*tile_index+1] & 0x07;

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(snk68_state::get_searchar_tile_info)
{
	int data = m_pow_fg_videoram[2*tile_index];
	int tile = data & 0x7ff;
	int color = (data & 0x7000) >> 12;

	// used in the ikari3 intro
	int flags = (data & 0x8000) ? TILE_FORCE_LAYER0 : 0;

	SET_TILE_INFO_MEMBER(0, tile, color, flags);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void snk68_state::common_video_start()
{
	m_fg_tilemap->set_transparent_pen(0);
	save_item(NAME(m_sprite_flip_axis));
}

void snk68_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk68_state::get_pow_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_fg_tile_offset = 0;

	common_video_start();

	save_item(NAME(m_fg_tile_offset));
}

VIDEO_START_MEMBER(snk68_state,searchar)
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(snk68_state::get_searchar_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	common_video_start();
}

/***************************************************************************

  Memory handlers

***************************************************************************/

READ16_MEMBER(snk68_state::spriteram_r)
{
	// streetsj expects the MSB of every 32-bit word to be FF. Presumably RAM
	// exists only for 3 bytes out of 4 and the fourth is unmapped.
	if (!(offset & 1))
		return m_spriteram[offset] | 0xff00;
	else
		return m_spriteram[offset];
}

WRITE16_MEMBER(snk68_state::spriteram_w)
{
	UINT16 newword = m_spriteram[offset];

	if (!(offset & 1))
		data |= 0xff00;

	COMBINE_DATA(&newword);

	if (m_spriteram[offset] != newword)
	{
		int vpos = m_screen->vpos();

		if (vpos > 0)
			m_screen->update_partial(vpos - 1);

		m_spriteram[offset] = newword;
	}
}

READ16_MEMBER(snk68_state::pow_fg_videoram_r)
{
	// RAM is only 8-bit
	return m_pow_fg_videoram[offset] | 0xff00;
}

WRITE16_MEMBER(snk68_state::pow_fg_videoram_w)
{
	data |= 0xff00;
	COMBINE_DATA(&m_pow_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE16_MEMBER(snk68_state::searchar_fg_videoram_w)
{
	// RAM is full 16-bit, though only half of it is used by the hardware
	COMBINE_DATA(&m_pow_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE16_MEMBER(snk68_state::pow_flipscreen_w)
{
	if (ACCESSING_BITS_0_7)
	{
		flip_screen_set(data & 0x08);

		m_sprite_flip_axis = data & 0x04;   // for streetsm? though might not be present on this board

		if (m_fg_tile_offset != ((data & 0x70) << 4))
		{
			m_fg_tile_offset = (data & 0x70) << 4;
			m_fg_tilemap->mark_all_dirty();
		}
	}
}

WRITE16_MEMBER(snk68_state::searchar_flipscreen_w)
{
	if (ACCESSING_BITS_0_7)
	{
		flip_screen_set(data & 0x08);
		m_sprite_flip_axis = data & 0x04;
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

void snk68_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int group)
{
	const UINT16* tiledata = &m_spriteram[0x800*group];

	// pow has 0x4000 tiles and independent x/y flipping
	// the other games have > 0x4000 tiles and flipping in only one direction
	// (globally selected)
	bool const is_pow = (m_gfxdecode->gfx(1)->elements() <= 0x4000);
	bool const flip = flip_screen();

	for (int offs = 0; offs < 0x800; offs += 0x40)
	{
		int mx = (m_spriteram[offs + 2*group] & 0xff) << 4;
		int my = m_spriteram[offs + 2*group + 1];
		int i;

		mx = mx | (my >> 12);

		mx = ((mx + 16) & 0x1ff) - 16;
		my = -my;

		if (flip)
		{
			mx = 240 - mx;
			my = 240 - my;
		}

		// every sprite is a column 32 tiles (512 pixels) tall
		for (i = 0; i < 0x20; ++i)
		{
			my &= 0x1ff;

			if (my <= cliprect.max_y && my + 15 >= cliprect.min_y)
			{
				int color = *(tiledata++) & 0x7f;
				int tile = *(tiledata++);
				int fx,fy;

				if (is_pow)
				{
					fx = tile & 0x4000;
					fy = tile & 0x8000;
					tile &= 0x3fff;
				}
				else
				{
					if (m_sprite_flip_axis)
					{
						fx = 0;
						fy = tile & 0x8000;
					}
					else
					{
						fx = tile & 0x8000;
						fy = 0;
					}
					tile &= 0x7fff;
				}

				if (flip)
				{
					fx = !fx;
					fy = !fy;
				}

				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						tile,
						color,
						fx, fy,
						mx, my, 0);
			}
			else
			{
				tiledata += 2;
			}

			if (flip)
				my -= 16;
			else
				my += 16;
		}
	}
}


UINT32 snk68_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x7ff, cliprect);

	/* This appears to be the correct priority order */
	draw_sprites(bitmap, cliprect, 2);
	draw_sprites(bitmap, cliprect, 3);
	draw_sprites(bitmap, cliprect, 1);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
