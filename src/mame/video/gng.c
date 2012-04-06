/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/gng.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	gng_state *state = machine.driver_data<gng_state>();
	UINT8 attr = state->m_fgvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			state->m_fgvideoram[tile_index] + ((attr & 0xc0) << 2),
			attr & 0x0f,
			TILE_FLIPYX((attr & 0x30) >> 4));
}

static TILE_GET_INFO( get_bg_tile_info )
{
	gng_state *state = machine.driver_data<gng_state>();
	UINT8 attr = state->m_bgvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			1,
			state->m_bgvideoram[tile_index] + ((attr & 0xc0) << 2),
			attr & 0x07,
			TILE_FLIPYX((attr & 0x30) >> 4));
	tileinfo.group = (attr & 0x08) >> 3;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gng )
{
	gng_state *state = machine.driver_data<gng_state>();
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(3);
	state->m_bg_tilemap->set_transmask(0, 0xff, 0x00); /* split type 0 is totally transparent in front half */
	state->m_bg_tilemap->set_transmask(1, 0x41, 0xbe); /* split type 1 has pens 0 and 6 transparent in front half */
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(gng_state::gng_fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(gng_state::gng_bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}


WRITE8_MEMBER(gng_state::gng_bgscrollx_w)
{
	m_scrollx[offset] = data;
	m_bg_tilemap->set_scrollx(0, m_scrollx[0] + 256 * m_scrollx[1]);
}

WRITE8_MEMBER(gng_state::gng_bgscrolly_w)
{
	m_scrolly[offset] = data;
	m_bg_tilemap->set_scrolly(0, m_scrolly[0] + 256 * m_scrolly[1]);
}


WRITE8_MEMBER(gng_state::gng_flipscreen_w)
{
	flip_screen_set(machine(), ~data & 1);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gng_state *state = machine.driver_data<gng_state>();
	UINT8 *buffered_spriteram = state->m_spriteram->buffer();
	const gfx_element *gfx = machine.gfx[2];
	int offs;


	for (offs = state->m_spriteram->bytes() - 4; offs >= 0; offs -= 4)
	{
		UINT8 attributes = buffered_spriteram[offs + 1];
		int sx = buffered_spriteram[offs + 3] - 0x100 * (attributes & 0x01);
		int sy = buffered_spriteram[offs + 2];
		int flipx = attributes & 0x04;
		int flipy = attributes & 0x08;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,gfx,
				buffered_spriteram[offs] + ((attributes << 2) & 0x300),
				(attributes >> 4) & 3,
				flipx,flipy,
				sx,sy,15);
	}
}

SCREEN_UPDATE_IND16( gng )
{
	gng_state *state = screen.machine().driver_data<gng_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
