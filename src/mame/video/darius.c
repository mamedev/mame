#include "emu.h"
#include "video/taitoic.h"
#include "includes/darius.h"

/***************************************************************************/

INLINE void actual_get_fg_tile_info( running_machine &machine, tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum )
{
	UINT16 code = (ram[tile_index + 0x2000] & 0x7ff);
	UINT16 attr = ram[tile_index];

	SET_TILE_INFO(
			gfxnum,
			code,
			((attr & 0xff) << 2),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

static TILE_GET_INFO( get_fg_tile_info )
{
	darius_state *state = machine.driver_data<darius_state>();
	actual_get_fg_tile_info(machine, tileinfo, tile_index, state->m_fg_ram, 2);
}

/***************************************************************************/

VIDEO_START( darius )
{
	darius_state *state = machine.driver_data<darius_state>();

	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,128,64);

	state->m_fg_tilemap->set_transparent_pen(0);
}

/***************************************************************************/

WRITE16_MEMBER(darius_state::darius_fg_layer_w)
{

	COMBINE_DATA(&m_fg_ram[offset]);
	if (offset < 0x4000)
		m_fg_tilemap->mark_tile_dirty((offset & 0x1fff));
}

/***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int primask, int x_offs, int y_offs )
{
	darius_state *state = machine.driver_data<darius_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs, curx, cury;
	UINT16 code, data, sx, sy;
	UINT8 flipx, flipy, color, priority;

	for (offs = state->m_spriteram_size / 2 - 4; offs >= 0; offs -= 4)
	{
		code = spriteram[offs + 2] & 0x1fff;

		if (code)
		{
			data = spriteram[offs];
			sy = (256 - data) & 0x1ff;

			data = spriteram[offs + 1];
			sx = data & 0x3ff;

			data = spriteram[offs + 2];
			flipx = ((data & 0x4000) >> 14);
			flipy = ((data & 0x8000) >> 15);

			data = spriteram[offs + 3];
			priority = (data & 0x80) >> 7;  // 0 = low
			if (priority != primask)
				continue;
			color = (data & 0x7f);

			curx = sx - x_offs;
			cury = sy + y_offs;

			if (curx > 900) curx -= 1024;
			if (cury > 400) cury -= 512;

			drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
					code, color,
					flipx, flipy,
					curx, cury, 0);
		}
	}
}



static UINT32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs)
{
	darius_state *state = screen.machine().driver_data<darius_state>();

	pc080sn_tilemap_update(state->m_pc080sn);

	// draw bottom layer(always active)
	pc080sn_tilemap_draw_offset(state->m_pc080sn, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0, -xoffs, 0);

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(screen.machine(), bitmap, cliprect, 0, xoffs, -8); // draw sprites with priority 0 which are under the mid layer

	// draw middle layer
	pc080sn_tilemap_draw_offset(state->m_pc080sn, bitmap, cliprect, 1, 0, 0, -xoffs, 0);

	draw_sprites(screen.machine(), bitmap, cliprect, 1, xoffs, -8); // draw sprites with priority 1 which are over the mid layer

	/* top(text) layer is in fixed position */
	state->m_fg_tilemap->set_scrollx(0, 0 + xoffs);
	state->m_fg_tilemap->set_scrolly(0, -8);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_UPDATE_IND16( darius_left ) { return update_screen(screen, bitmap, cliprect, 36 * 8 * 0); }
SCREEN_UPDATE_IND16( darius_middle ) { return update_screen(screen, bitmap, cliprect, 36 * 8 * 1); }
SCREEN_UPDATE_IND16( darius_right ) { return update_screen(screen, bitmap, cliprect, 36 * 8 * 2); }

