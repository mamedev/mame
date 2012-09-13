#include "emu.h"
#include "includes/pushman.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(pushman_state::background_scan_rows)
{
	/* logical (col,row) -> memory offset */
	return ((col & 0x7)) + ((7 - (row & 0x7)) << 3) + ((col & 0x78) << 3) + ((0x38 - (row & 0x38)) << 7);
}

TILE_GET_INFO_MEMBER(pushman_state::get_back_tile_info)
{
	UINT8 *bg_map = machine().root_device().memregion("gfx4")->base();
	int tile;

	tile = bg_map[tile_index << 1] + (bg_map[(tile_index << 1) + 1] << 8);
	SET_TILE_INFO_MEMBER(
			2,
			(tile & 0xff) | ((tile & 0x4000) >> 6),
			(tile >> 8) & 0xf,
			(tile & 0x2000) ? TILE_FLIPX : 0);
}

TILE_GET_INFO_MEMBER(pushman_state::get_text_tile_info)
{

	int tile = m_videoram[tile_index];
	SET_TILE_INFO_MEMBER(
			0,
			(tile & 0xff) | ((tile & 0xc000) >> 6) | ((tile & 0x2000) >> 3),
			(tile >> 8) & 0xf,
			(tile & 0x1000) ? TILE_FLIPY : 0);	/* not used? from Tiger Road */
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void pushman_state::video_start()
{

	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(pushman_state::get_back_tile_info),this), tilemap_mapper_delegate(FUNC(pushman_state::background_scan_rows),this), 32, 32, 128, 64);
	m_tx_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(pushman_state::get_text_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_tx_tilemap->set_transparent_pen(3);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(pushman_state::pushman_scroll_w)
{
	COMBINE_DATA(&m_control[offset]);
}

WRITE16_MEMBER(pushman_state::pushman_videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	pushman_state *state = machine.driver_data<pushman_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs, x, y, color, sprite, flipx, flipy;

	for (offs = 0x0800 - 4; offs >=0; offs -= 4)
	{
		/* Don't draw empty sprite table entries */
		x = spriteram[offs + 3] & 0x1ff;
		if (x == 0x180)
			continue;
		if (x > 0xff)
			x = 0 - (0x200 - x);

		y = 240 - spriteram[offs + 2];
		color = ((spriteram[offs + 1] >> 2) & 0xf);
		sprite = spriteram[offs] & 0x7ff;
		/* ElSemi - Sprite flip info */
		flipx = spriteram[offs + 1] & 2;
		flipy = spriteram[offs + 1] & 1;	/* flip y untested */

		if (state->flip_screen())
		{
			x = 240 - x;
			y = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1], sprite,
                color, flipx, flipy, x, y, 15);
	}
}

SCREEN_UPDATE_IND16( pushman )
{
	pushman_state *state = screen.machine().driver_data<pushman_state>();

	/* Setup the tilemaps */
	state->m_bg_tilemap->set_scrollx(0, state->m_control[0]);
	state->m_bg_tilemap->set_scrolly(0, 0xf00 - state->m_control[1]);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
