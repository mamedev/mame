#include "emu.h"
#include "includes/exprraid.h"


WRITE8_MEMBER(exprraid_state::exprraid_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(exprraid_state::exprraid_colorram_w)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(exprraid_state::exprraid_flipscreen_w)
{
	if (flip_screen_get(machine()) != (data & 0x01))
	{
		flip_screen_set(machine(), data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER(exprraid_state::exprraid_bgselect_w)
{
	if (m_bg_index[offset] != data)
	{
		m_bg_index[offset] = data;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(exprraid_state::exprraid_scrollx_w)
{
	m_bg_tilemap->set_scrollx(offset, data);
}

WRITE8_MEMBER(exprraid_state::exprraid_scrolly_w)
{
	m_bg_tilemap->set_scrolly(0, data);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	exprraid_state *state = machine.driver_data<exprraid_state>();
	UINT8 *tilerom = machine.region("gfx4")->base();

	int data, attr, bank, code, color, flags;
	int quadrant = 0, offs;

	int sx = tile_index % 32;
	int sy = tile_index / 32;

	if (sx >= 16) quadrant++;
	if (sy >= 16) quadrant += 2;

	offs = (sy % 16) * 16 + (sx % 16) + (state->m_bg_index[quadrant] & 0x3f) * 0x100;

	data = tilerom[offs];
	attr = tilerom[offs + 0x4000];
	bank = (2 * (attr & 0x03) + ((data & 0x80) >> 7)) + 2;
	code = data & 0x7f;
	color = (attr & 0x18) >> 3;
	flags = (attr & 0x04) ? TILE_FLIPX : 0;

	tileinfo.category = ((attr & 0x80) ? 1 : 0);

	SET_TILE_INFO(bank, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	exprraid_state *state = machine.driver_data<exprraid_state>();
	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index] + ((attr & 0x07) << 8);
	int color = (attr & 0x10) >> 4;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( exprraid )
{
	exprraid_state *state = machine.driver_data<exprraid_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_bg_tilemap->set_scroll_rows(2);
	state->m_fg_tilemap->set_transparent_pen(0);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	exprraid_state *state = machine.driver_data<exprraid_state>();
	int offs;

	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		int attr = state->m_spriteram[offs + 1];
		int code = state->m_spriteram[offs + 3] + ((attr & 0xe0) << 3);
		int color = (attr & 0x03) + ((attr & 0x08) >> 1);
		int flipx = (attr & 0x04);
		int flipy = 0;
		int sx = ((248 - state->m_spriteram[offs + 2]) & 0xff) - 8;
		int sy = state->m_spriteram[offs];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, machine.gfx[1],
			code, color,
			flipx, flipy,
			sx, sy, 0);

		/* double height */

		if (attr & 0x10)
		{
			drawgfx_transpen(bitmap,cliprect, machine.gfx[1],
				code + 1, color,
				flipx, flipy,
				sx, sy + (flip_screen_get(machine) ? -16 : 16), 0);
		}
	}
}

SCREEN_UPDATE_IND16( exprraid )
{
	exprraid_state *state = screen.machine().driver_data<exprraid_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, 1, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
