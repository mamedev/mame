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
	dynamic_array<rgb_t> rgb;

	compute_res_net_all(rgb, color_prom, mario_decode_info, mario_net_info);
	palette.set_pen_colors(0, rgb, 256);
	compute_res_net_all(rgb, color_prom+256, mario_decode_info, mario_net_info_std);
	palette.set_pen_colors(256, rgb, 256);

	palette.palette()->normalize_range(0, 255);
	palette.palette()->normalize_range(256, 511);
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
	int color;

	color =  ((m_videoram[tile_index] >> 2) & 0x38) | 0x40 | (m_palette_bank<<7) | (m_monitor<<8);
	color = color >> 2;
	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void mario_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mario_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_gfx_bank = 0;
	m_palette_bank = 0;
	m_gfx_scroll = 0;
	save_item(NAME(m_gfx_bank));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_gfx_scroll));
	save_item(NAME(m_flip));
}

/*
 * Erratic line at top when scrolling down "Marios Bros" Title
 * confirmed on mametests.org as being present on real PCB as well.
 */

void mario_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* TODO: draw_sprites should adopt the scanline logic from dkong.c
	 * The schematics have the same logic for sprite buffering.
	 */
	int offs;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		if (m_spriteram[offs])
		{
			int x, y;

			// from schematics ....
			y = (m_spriteram[offs] + (m_flip ? 0xF7 : 0xF9) + 1) & 0xFF;
			x = m_spriteram[offs+3];
			// sprite will be drawn if (y + scanline) & 0xF0 = 0xF0
			y = 240 - y; /* logical screen position */

			y = y ^ (m_flip ? 0xFF : 0x00); /* physical screen location */
			x = x ^ (m_flip ? 0xFF : 0x00); /* physical screen location */

			if (m_flip)
			{
				y -= 14;
				x -= 7;
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						m_spriteram[offs + 2],
						(m_spriteram[offs + 1] & 0x0f) + 16 * m_palette_bank + 32 * m_monitor,
						!(m_spriteram[offs + 1] & 0x80),!(m_spriteram[offs + 1] & 0x40),
						x, y,0);
			}
			else
			{
				y += 1;
				x -= 8;
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						m_spriteram[offs + 2],
						(m_spriteram[offs + 1] & 0x0f) + 16 * m_palette_bank + 32 * m_monitor,
						(m_spriteram[offs + 1] & 0x80),(m_spriteram[offs + 1] & 0x40),
						x, y,0);
			}
		}
	}
}

UINT32 mario_state::screen_update_mario(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int t;

	t = ioport("MONITOR")->read();
	if (t != m_monitor)
	{
		m_monitor = t;
		machine().tilemap().mark_all_dirty();
	}

	m_bg_tilemap->set_scrolly(0, m_gfx_scroll);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);

	return 0;
}
