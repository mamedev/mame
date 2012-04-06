#include "emu.h"
#include "includes/ohmygod.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	ohmygod_state *state = machine.driver_data<ohmygod_state>();
	UINT16 code = state->m_videoram[2 * tile_index + 1];
	UINT16 attr = state->m_videoram[2 * tile_index];
	SET_TILE_INFO(
			0,
			code,
			(attr & 0x0f00) >> 8,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( ohmygod )
{
	ohmygod_state *state = machine.driver_data<ohmygod_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(ohmygod_state::ohmygod_videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(ohmygod_state::ohmygod_spritebank_w)
{
	if (ACCESSING_BITS_8_15)
		m_spritebank = data & 0x8000;
}

WRITE16_MEMBER(ohmygod_state::ohmygod_scrollx_w)
{
	COMBINE_DATA(&m_scrollx);
	m_bg_tilemap->set_scrollx(0, m_scrollx - 0x81ec);
}

WRITE16_MEMBER(ohmygod_state::ohmygod_scrolly_w)
{
	COMBINE_DATA(&m_scrolly);
	m_bg_tilemap->set_scrolly(0, m_scrolly - 0x81ef);
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	ohmygod_state *state = machine.driver_data<ohmygod_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0; offs < state->m_spriteram_size / 4; offs += 4)
	{
		int sx, sy, code, color, flipx;
		UINT16 *sr;

		sr = state->m_spritebank ? (spriteram + state->m_spriteram_size / 4) : spriteram;

		code = sr[offs + 3] & 0x0fff;
		color = sr[offs + 2] & 0x000f;
		sx = sr[offs + 0] - 29;
		sy = sr[offs + 1];
		if (sy >= 32768)
			sy -= 65536;
		flipx = sr[offs + 3] & 0x8000;

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,0,
				sx,sy,0);
	}
}

SCREEN_UPDATE_IND16( ohmygod )
{
	ohmygod_state *state = screen.machine().driver_data<ohmygod_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
