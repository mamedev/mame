/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/holeland.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( holeland_get_tile_info )
{
	holeland_state *state = machine.driver_data<holeland_state>();
	int attr = state->m_colorram[tile_index];
	int tile_number = state->m_videoram[tile_index] | ((attr & 0x03) << 8);

/*if (machine.input().code_pressed(KEYCODE_Q) && (attr & 0x10)) tile_number = rand(); */
/*if (machine.input().code_pressed(KEYCODE_W) && (attr & 0x20)) tile_number = rand(); */
/*if (machine.input().code_pressed(KEYCODE_E) && (attr & 0x40)) tile_number = rand(); */
/*if (machine.input().code_pressed(KEYCODE_R) && (attr & 0x80)) tile_number = rand(); */
	SET_TILE_INFO(
			0,
			tile_number,
			state->m_palette_offset + ((attr >> 4) & 0x0f),
			TILE_FLIPYX((attr >> 2) & 0x03));
	tileinfo.group = (attr >> 4) & 1;
}

static TILE_GET_INFO( crzrally_get_tile_info )
{
	holeland_state *state = machine.driver_data<holeland_state>();
	int attr = state->m_colorram[tile_index];
	int tile_number = state->m_videoram[tile_index] | ((attr & 0x03) << 8);

	SET_TILE_INFO(
			0,
			tile_number,
			state->m_palette_offset + ((attr >> 4) & 0x0f),
			TILE_FLIPYX((attr >> 2) & 0x03));
	tileinfo.group = (attr >> 4) & 1;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( holeland )
{
	holeland_state *state = machine.driver_data<holeland_state>();
	state->m_bg_tilemap = tilemap_create(machine, holeland_get_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	state->m_bg_tilemap->set_transmask(0, 0xff, 0x00); /* split type 0 is totally transparent in front half */
	state->m_bg_tilemap->set_transmask(1, 0x01, 0xfe); /* split type 1 has pen 0? transparent in front half */
}

VIDEO_START( crzrally )
{
	holeland_state *state = machine.driver_data<holeland_state>();
	state->m_bg_tilemap = tilemap_create(machine, crzrally_get_tile_info, tilemap_scan_cols, 8, 8, 32, 32);
}

WRITE8_MEMBER(holeland_state::holeland_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(holeland_state::holeland_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(holeland_state::holeland_pal_offs_w)
{
	if ((data & 1) != m_po[offset])
	{
		m_po[offset] = data & 1;
		m_palette_offset = (m_po[0] + (m_po[1] << 1)) << 4;
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER(holeland_state::holeland_scroll_w)
{
	m_bg_tilemap->set_scrollx(0, data);
}

WRITE8_MEMBER(holeland_state::holeland_flipscreen_w)
{
	if (offset)
		flip_screen_y_set(machine(), data);
	else
		flip_screen_x_set(machine(), data);
}


static void holeland_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	holeland_state *state = machine.driver_data<holeland_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs, code, sx, sy, color, flipx, flipy;

	/* Weird, sprites entries don't start on DWORD boundary */
	for (offs = 3; offs < state->m_spriteram_size - 1; offs += 4)
	{
		sy = 236 - spriteram[offs];
		sx = spriteram[offs + 2];

		/* Bit 7 unknown */
		code = spriteram[offs + 1] & 0x7f;
		color = state->m_palette_offset + (spriteram[offs + 3] >> 4);

		/* Bit 0, 1 unknown */
		flipx = spriteram[offs + 3] & 0x04;
		flipy = spriteram[offs + 3] & 0x08;

		if (flip_screen_x_get(machine))
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y_get(machine))
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,flipy,
				2*sx,2*sy,0);
	}
}

static void crzrally_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	holeland_state *state = machine.driver_data<holeland_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs, code, sx, sy, color, flipx, flipy;

	/* Weird, sprites entries don't start on DWORD boundary */
	for (offs = 3; offs < state->m_spriteram_size - 1; offs += 4)
	{
		sy = 236 - spriteram[offs];
		sx = spriteram[offs + 2];

		code = spriteram[offs + 1] + ((spriteram[offs + 3] & 0x01) << 8);
		color = (spriteram[offs + 3] >> 4) + ((spriteram[offs + 3] & 0x01) << 4);

		/* Bit 1 unknown */
		flipx = spriteram[offs + 3] & 0x04;
		flipy = spriteram[offs + 3] & 0x08;

		if (flip_screen_x_get(machine))
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y_get(machine))
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}

SCREEN_UPDATE_IND16( holeland )
{
	holeland_state *state = screen.machine().driver_data<holeland_state>();
/*state->m_bg_tilemap->mark_all_dirty(); */
	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	holeland_draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}

SCREEN_UPDATE_IND16( crzrally )
{
	holeland_state *state = screen.machine().driver_data<holeland_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	crzrally_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
