// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"

#include "includes/mario.h"

static const res_net_decode_info mario_decode_info =
{
	1,      // there may be two proms needed to construct color
	0,      // start at 0
	255,    // end at 255
	//  R,   G,   B
	{   0,   0,   0},       // offsets
	{   5,   2,   0},       // shifts
	{0x07,0x07,0x03}        // masks
};

static const res_net_info mario_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052 | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_EMITTER,    680, 0, 2, {  470, 220,   0 } }  // dkong
	}
};

static const res_net_info mario_net_info_std =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_EMITTER,    680, 0, 2, {  470, 220,   0 } }  // dkong
	}
};

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mario Bros. has a 512x8 palette PROM; interstingly, bytes 0-255 contain an
  inverted palette, as other Nintendo games like Donkey Kong, while bytes
  256-511 contain a non inverted palette. This was probably done to allow
  connection to both the special Nintendo and a standard monitor.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor -- inverter  -- RED
        -- 470 ohm resistor -- inverter  -- RED
        -- 1  kohm resistor -- inverter  -- RED
        -- 220 ohm resistor -- inverter  -- GREEN
        -- 470 ohm resistor -- inverter  -- GREEN
        -- 1  kohm resistor -- inverter  -- GREEN
        -- 220 ohm resistor -- inverter  -- BLUE
  bit 0 -- 470 ohm resistor -- inverter  -- BLUE

***************************************************************************/
PALETTE_INIT_MEMBER(mario_state, mario)
{
	const UINT8 *color_prom = memregion("proms")->base();
	std::vector<rgb_t> rgb;

	if (m_monitor == 0)
		compute_res_net_all(rgb, color_prom, mario_decode_info, mario_net_info);
	else
		compute_res_net_all(rgb, color_prom+256, mario_decode_info, mario_net_info_std);

	palette.set_pen_colors(0, rgb);
	palette.palette()->normalize_range(0, 255);
}

WRITE8_MEMBER(mario_state::mario_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mario_state::mario_gfxbank_w)
{
	if (m_gfx_bank != (data & 0x01))
	{
		m_gfx_bank = data & 0x01;
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER(mario_state::mario_palettebank_w)
{
	if (m_palette_bank != (data & 0x01))
	{
		m_palette_bank = data & 0x01;
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER(mario_state::mario_scroll_w)
{
	m_gfx_scroll = data + 17;
}

WRITE8_MEMBER(mario_state::mariobl_scroll_w)
{
	m_gfx_scroll = data;
}

WRITE8_MEMBER(mario_state::mario_flip_w)
{
	if (m_flip != (data & 0x01))
	{
		m_flip = data & 0x01;
		if (m_flip)
			machine().tilemap().set_flip_all(TILEMAP_FLIPX | TILEMAP_FLIPY);
		else
			machine().tilemap().set_flip_all(0);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(mario_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] + 256 * m_gfx_bank;
	int color = 8 + (m_videoram[tile_index] >> 5) + 16 * m_palette_bank;
	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void mario_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mario_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_gfxdecode->gfx(0)->set_granularity(8);

	m_gfx_bank = 0;
	m_palette_bank = 0;
	m_gfx_scroll = 0;
	m_flip = 0;
	save_item(NAME(m_gfx_bank));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_gfx_scroll));
	save_item(NAME(m_flip));
}

/*
 * Erratic line at top when scrolling down "Marios Bros" Title
 * confirmed on mametests.org as being present on real PCB as well.
 */

void mario_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int is_bootleg)
{
	/* TODO: draw_sprites should adopt the scanline logic from dkong.c
	 * The schematics have the same logic for sprite buffering.
	 */
	int offs;

	int start, end, inc;

	if (!is_bootleg)
	{
		start = 0;
		end = m_spriteram.bytes();
		inc = 4;
	}
	else
	{
		start = m_spriteram.bytes()-4;
		end = -4;
		inc = -4;
	}

	offs = start;

	while (offs != end)
	{
		if (is_bootleg || m_spriteram[offs])
		{
			int x, y;
			int code, color, flipx, flipy;

			if (!is_bootleg)
			{
				// from schematics ....
				y = (m_spriteram[offs + 0] + (m_flip ? 0xF7 : 0xF9) + 1) & 0xFF;
				x = m_spriteram[offs + 3];
				// sprite will be drawn if (y + scanline) & 0xF0 = 0xF0
				y = 240 - y; /* logical screen position */

				y = y ^ (m_flip ? 0xFF : 0x00); /* physical screen location */
				x = x ^ (m_flip ? 0xFF : 0x00); /* physical screen location */

				code = m_spriteram[offs + 2];
				color = (m_spriteram[offs + 1] & 0x0f) + 16 * m_palette_bank;
				flipx = (m_spriteram[offs + 1] & 0x80);
				flipy = (m_spriteram[offs + 1] & 0x40);

				if (m_flip)
				{
					y -= 14;
					x -= 7;
				}
				else
				{
					y += 1;
					x -= 8;
				}
			}
			else
			{
				y = (m_spriteram[offs + 3] + (m_flip ? 0xF7 : 0xF9) + 1) & 0xFF;
				x = m_spriteram[offs + 0];
				y = 240 - y; /* logical screen position */

			//  y = y ^ (m_flip ? 0xFF : 0x00); /* physical screen location */
			//  x = x ^ (m_flip ? 0xFF : 0x00); /* physical screen location */

				code = (m_spriteram[offs + 2] & 0x7f) | ((m_spriteram[offs + 1] & 0x40) << 1); // upper tile bit is where the flipy bit goes on mario
				color = (m_spriteram[offs + 1] & 0x0f) + 16 * m_palette_bank;
				flipx = (m_spriteram[offs + 1] & 0x80);
				flipy = (m_spriteram[offs + 2] & 0x80); // and the flipy bit is where the upper tile bit is on mario

				y += -7;
			}

			if (m_flip)
			{
				m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					code,
					color,
					!flipx, !flipy,
					x, y, 0);
			}
			else
			{
				m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					code,
					color,
					flipx, flipy,
					x, y, 0);
			}
		}

		offs += inc;
	}
}

UINT32 mario_state::screen_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int t;

	t = ioport("MONITOR")->read();
	if (t != m_monitor)
	{
		m_monitor = t;
		PALETTE_INIT_NAME(mario)(m_palette);
	}

	m_bg_tilemap->set_scrolly(0, m_gfx_scroll);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

UINT32 mario_state::screen_update_mario(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_common(screen, bitmap, cliprect);
	draw_sprites(bitmap, cliprect, 0);
	return 0;
}

UINT32 mario_state::screen_update_mariobl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// not sure
	m_palette_bank = m_gfx_bank; // might be the 'attr' ram
	machine().tilemap().mark_all_dirty();


	screen_update_common(screen, bitmap, cliprect);
	draw_sprites(bitmap, cliprect, 1);
	return 0;
}
