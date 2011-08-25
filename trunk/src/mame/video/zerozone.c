/***************************************************************************

  video/zerozone.c

***************************************************************************/

#include "emu.h"
#include "includes/zerozone.h"

WRITE16_HANDLER( zerozone_tilemap_w )
{
	zerozone_state *state = space->machine().driver_data<zerozone_state>();

	COMBINE_DATA(&state->m_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_zz_tilemap,offset);
}


WRITE16_HANDLER(zerozone_tilebank_w)
{
	zerozone_state *state = space->machine().driver_data<zerozone_state>();

//  popmessage ("Data %04x",data);
	state->m_tilebank = data & 0x07;
	tilemap_mark_all_tiles_dirty(state->m_zz_tilemap);
}

static TILE_GET_INFO( get_zerozone_tile_info )
{
	zerozone_state *state = machine.driver_data<zerozone_state>();
	int tileno = state->m_videoram[tile_index] & 0x07ff;
	int colour = state->m_videoram[tile_index] & 0xf000;

	if (state->m_videoram[tile_index] & 0x0800)
		tileno += state->m_tilebank * 0x800;

	SET_TILE_INFO(0, tileno, colour >> 12, 0);
}

VIDEO_START( zerozone )
{
	zerozone_state *state = machine.driver_data<zerozone_state>();

	// i'm not 100% sure it should be opaque, pink title screen looks strange in las vegas girls
	// but if its transparent other things look incorrect
	state->m_zz_tilemap = tilemap_create(machine, get_zerozone_tile_info, tilemap_scan_cols, 8, 8, 64, 32);
}

SCREEN_UPDATE( zerozone )
{
	zerozone_state *state = screen->machine().driver_data<zerozone_state>();

	tilemap_draw(bitmap, cliprect, state->m_zz_tilemap, 0, 0);
	return 0;
}
