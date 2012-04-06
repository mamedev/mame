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

	state->m_fg_tilemap->set_transparent_pen(15);
	state->m_tx_tilemap->set_transparent_pen(15);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(wc90b_state::wc90b_bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE8_MEMBER(wc90b_state::wc90b_fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE8_MEMBER(wc90b_state::wc90b_txvideoram_w)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x7ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
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

SCREEN_UPDATE_IND16( wc90b )
{
	wc90b_state *state = screen.machine().driver_data<wc90b_state>();
	state->m_bg_tilemap->set_scrollx(0,8 * (state->m_scroll2x[0] & 0x7f) + 256 - 4 + (state->m_scroll_x_lo[0] & 0x07));
	state->m_bg_tilemap->set_scrolly(0,state->m_scroll2y[0] + 1 + ((state->m_scroll2x[0] & 0x80) ? 256 : 0));
	state->m_fg_tilemap->set_scrollx(0,8 * (state->m_scroll1x[0] & 0x7f) + 256 - 6 + ((state->m_scroll_x_lo[0] & 0x38) >> 3));
	state->m_fg_tilemap->set_scrolly(0,state->m_scroll1y[0] + 1 + ((state->m_scroll1x[0] & 0x80) ? 256 : 0));

	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap,cliprect, 1 );
	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap,cliprect, 0 );
	return 0;
}
