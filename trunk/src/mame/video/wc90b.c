#include "emu.h"
#include "includes/wc90b.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	wc90b_state *state = machine.driver_data<wc90b_state>();
	int attr = state->m_bgvideoram[tile_index];
	int tile = state->m_bgvideoram[tile_index + 0x800];
	SET_TILE_INFO(
			9 + ((attr & 3) + ((attr >> 1) & 4)),
			tile,
			attr >> 4,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	wc90b_state *state = machine.driver_data<wc90b_state>();
	int attr = state->m_fgvideoram[tile_index];
	int tile = state->m_fgvideoram[tile_index + 0x800];
	SET_TILE_INFO(
			1 + ((attr & 3) + ((attr >> 1) & 4)),
			tile,
			attr >> 4,
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	wc90b_state *state = machine.driver_data<wc90b_state>();
	SET_TILE_INFO(
			0,
			state->m_txvideoram[tile_index + 0x800] + ((state->m_txvideoram[tile_index] & 0x07) << 8),
			state->m_txvideoram[tile_index] >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( wc90b )
{
	wc90b_state *state = machine.driver_data<wc90b_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,     16,16,64,32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,16,16,64,32);
	state->m_tx_tilemap = tilemap_create(machine, get_tx_tile_info,tilemap_scan_rows, 8, 8,64,32);

	tilemap_set_transparent_pen(state->m_fg_tilemap,15);
	tilemap_set_transparent_pen(state->m_tx_tilemap,15);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( wc90b_bgvideoram_w )
{
	wc90b_state *state = space->machine().driver_data<wc90b_state>();
	state->m_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap,offset & 0x7ff);
}

WRITE8_HANDLER( wc90b_fgvideoram_w )
{
	wc90b_state *state = space->machine().driver_data<wc90b_state>();
	state->m_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap,offset & 0x7ff);
}

WRITE8_HANDLER( wc90b_txvideoram_w )
{
	wc90b_state *state = space->machine().driver_data<wc90b_state>();
	state->m_txvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_tx_tilemap,offset & 0x7ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	wc90b_state *state = machine.driver_data<wc90b_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs, sx, sy;

	/* draw all visible sprites of specified priority */
	for ( offs = state->m_spriteram_size - 8 ; offs >= 0 ; offs -= 8 )
	{
		if ( ( ~( spriteram[offs+3] >> 7 ) & 1 ) == priority )
		{
			int code = ( spriteram[offs + 3] & 0x3f ) << 4;
			int bank = spriteram[offs + 0];
			int flags = spriteram[offs + 4];

			code += ( bank & 0xf0 ) >> 4;
			code <<= 2;
			code += ( bank & 0x0f ) >> 2;

			sx = spriteram[offs + 2];
			if (!(spriteram[offs + 3] & 0x40)) sx -= 0x0100;

			sy = 240 - spriteram[offs + 1];

			drawgfx_transpen( bitmap, cliprect,machine.gfx[17], code,
					flags >> 4, /* color */
					bank & 1,   /* flipx */
					bank & 2,   /* flipy */
					sx,
					sy,15 );
		}
	}
}

SCREEN_UPDATE( wc90b )
{
	wc90b_state *state = screen->machine().driver_data<wc90b_state>();
	tilemap_set_scrollx(state->m_bg_tilemap,0,8 * (state->m_scroll2x[0] & 0x7f) + 256 - 4 + (state->m_scroll_x_lo[0] & 0x07));
	tilemap_set_scrolly(state->m_bg_tilemap,0,state->m_scroll2y[0] + 1 + ((state->m_scroll2x[0] & 0x80) ? 256 : 0));
	tilemap_set_scrollx(state->m_fg_tilemap,0,8 * (state->m_scroll1x[0] & 0x7f) + 256 - 6 + ((state->m_scroll_x_lo[0] & 0x38) >> 3));
	tilemap_set_scrolly(state->m_fg_tilemap,0,state->m_scroll1y[0] + 1 + ((state->m_scroll1x[0] & 0x80) ? 256 : 0));

	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,state->m_fg_tilemap,0,0);
	draw_sprites(screen->machine(), bitmap,cliprect, 1 );
	tilemap_draw(bitmap,cliprect,state->m_tx_tilemap,0,0);
	draw_sprites(screen->machine(), bitmap,cliprect, 0 );
	return 0;
}
