/***************************************************************************

    Kyugo hardware games

***************************************************************************/

#include "emu.h"
#include "includes/kyugo.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	kyugo_state *state = machine.driver_data<kyugo_state>();
	int code = state->m_fgvideoram[tile_index];
	SET_TILE_INFO(0,
				  code,
				  2 * state->m_color_codes[code >> 3] + state->m_fgcolor,
				  0);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	kyugo_state *state = machine.driver_data<kyugo_state>();
	int code = state->m_bgvideoram[tile_index];
	int attr = state->m_bgattribram[tile_index];
	SET_TILE_INFO(1,
				  code | ((attr & 0x03) << 8),
				  (attr >> 4) | (state->m_bgpalbank << 4),
				  TILE_FLIPYX((attr & 0x0c) >> 2));
}


/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( kyugo )
{
	kyugo_state *state = machine.driver_data<kyugo_state>();

	state->m_color_codes = machine.region("proms")->base() + 0x300;

	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_fg_tilemap->set_transparent_pen(0);

	state->m_fg_tilemap->set_scrolldx(0, 224);
	state->m_bg_tilemap->set_scrolldx(-32, 32);
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(kyugo_state::kyugo_fgvideoram_w)
{

	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(kyugo_state::kyugo_bgvideoram_w)
{

	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(kyugo_state::kyugo_bgattribram_w)
{

	m_bgattribram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


READ8_MEMBER(kyugo_state::kyugo_spriteram_2_r)
{

	// only the lower nibble is connected
	return m_spriteram_2[offset] | 0xf0;
}


WRITE8_MEMBER(kyugo_state::kyugo_scroll_x_lo_w)
{
	m_scroll_x_lo = data;
}


WRITE8_MEMBER(kyugo_state::kyugo_gfxctrl_w)
{

	/* bit 0 is scroll MSB */
	m_scroll_x_hi = data & 0x01;

	/* bit 5 is front layer color (Son of Phoenix only) */
	if (m_fgcolor != ((data & 0x20) >> 5))
	{
		m_fgcolor = (data & 0x20) >> 5;

		m_fg_tilemap->mark_all_dirty();
	}

	/* bit 6 is background palette bank */
	if (m_bgpalbank != ((data & 0x40) >> 6))
	{
		m_bgpalbank = (data & 0x40) >> 6;
		m_bg_tilemap->mark_all_dirty();
	}

	if (data & 0x9e)
		popmessage("%02x",data);
}


WRITE8_MEMBER(kyugo_state::kyugo_scroll_y_w)
{
	m_scroll_y = data;
}


WRITE8_MEMBER(kyugo_state::kyugo_flipscreen_w)
{

	if (m_flipscreen != (data & 0x01))
	{
		m_flipscreen = (data & 0x01);
		machine().tilemap().set_flip_all((m_flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY): 0));
	}
}


/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	kyugo_state *state = machine.driver_data<kyugo_state>();

	/* sprite information is scattered through memory */
	/* and uses a portion of the text layer memory (outside the visible area) */
	UINT8 *spriteram_area1 = &state->m_spriteram_1[0x28];
	UINT8 *spriteram_area2 = &state->m_spriteram_2[0x28];
	UINT8 *spriteram_area3 = &state->m_fgvideoram[0x28];

	int n;

	for (n = 0; n < 12 * 2; n++)
	{
		int offs, y, sy, sx, color;

		offs = 2 * (n % 12) + 64 * (n / 12);

		sx = spriteram_area3[offs + 1] + 256 * (spriteram_area2[offs + 1] & 1);
		if (sx > 320)
			sx -= 512;

		sy = 255 - spriteram_area1[offs] + 2;
		if (sy > 0xf0)
			sy -= 256;

		if (state->m_flipscreen)
			sy = 240 - sy;

		color = spriteram_area1[offs + 1] & 0x1f;

		for (y = 0; y < 16; y++)
		{
			int code, attr, flipx, flipy;

			code = spriteram_area3[offs + 128 * y];
			attr = spriteram_area2[offs + 128 * y];

			code = code | ((attr & 0x01) << 9) | ((attr & 0x02) << 7);

			flipx =  attr & 0x08;
			flipy =  attr & 0x04;

			if (state->m_flipscreen)
			{
				flipx = !flipx;
				flipy = !flipy;
			}


			drawgfx_transpen( bitmap, cliprect,machine.gfx[2],
					 code,
					 color,
					 flipx,flipy,
					 sx,state->m_flipscreen ? sy - 16*y : sy + 16*y, 0 );
		}
	}
}


SCREEN_UPDATE_IND16( kyugo )
{
	kyugo_state *state = screen.machine().driver_data<kyugo_state>();

	if (state->m_flipscreen)
		state->m_bg_tilemap->set_scrollx(0, -(state->m_scroll_x_lo + (state->m_scroll_x_hi * 256)));
	else
		state->m_bg_tilemap->set_scrollx(0,   state->m_scroll_x_lo + (state->m_scroll_x_hi * 256));

	state->m_bg_tilemap->set_scrolly(0, state->m_scroll_y);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
