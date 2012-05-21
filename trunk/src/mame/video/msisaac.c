/*
*   Video Driver for Metal Soldier Isaac II (1985)
*/

#include "emu.h"
#include "includes/msisaac.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	msisaac_state *state = machine.driver_data<msisaac_state>();
	int tile_number = state->m_videoram[tile_index];
	SET_TILE_INFO( 0,
			tile_number,
			0x10,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	msisaac_state *state = machine.driver_data<msisaac_state>();
	int tile_number = state->m_videoram2[tile_index];
	SET_TILE_INFO( 1,
			0x100 + tile_number,
			0x30,
			0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	msisaac_state *state = machine.driver_data<msisaac_state>();
	int tile_number = state->m_videoram3[tile_index];

	/* graphics 0 or 1 */
	int gfx_b = (state->m_bg2_textbank >> 3) & 1;

	SET_TILE_INFO( gfx_b,
			tile_number,
			0x20,
			0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( msisaac )
{
	msisaac_state *state = machine.driver_data<msisaac_state>();
	state->m_bg_tilemap  = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg2_tilemap = tilemap_create(machine, get_bg2_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_fg_tilemap  = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_bg2_tilemap->set_transparent_pen(0);
	state->m_fg_tilemap->set_transparent_pen(0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(msisaac_state::msisaac_fg_scrolly_w)
{
	m_fg_tilemap->set_scrolly(0, data);
}

WRITE8_MEMBER(msisaac_state::msisaac_fg_scrollx_w)
{
	m_fg_tilemap->set_scrollx(0, 9 + data);
}

WRITE8_MEMBER(msisaac_state::msisaac_bg2_scrolly_w)
{
	m_bg2_tilemap->set_scrolly(0, data);
}

WRITE8_MEMBER(msisaac_state::msisaac_bg2_scrollx_w)
{
	m_bg2_tilemap->set_scrollx(0, 9 + 2 + data);
}

WRITE8_MEMBER(msisaac_state::msisaac_bg_scrolly_w)
{
	m_bg_tilemap->set_scrolly(0, data);
}

WRITE8_MEMBER(msisaac_state::msisaac_bg_scrollx_w)
{
	m_bg_tilemap->set_scrollx(0, 9 + 4 + data);
}


#ifdef UNUSED_FUNCTION
WRITE8_MEMBER(msisaac_state::msisaac_textbank1_w)
{
	if (textbank1!=data)
	{
		textbank1 = data;
		fg_tilemap->mark_all_dirty();
	}
}
#endif

WRITE8_MEMBER(msisaac_state::msisaac_bg2_textbank_w)
{
	if (m_bg2_textbank != data )
	{
		m_bg2_textbank = data;
		m_bg2_tilemap->mark_all_dirty();

		//check if we are correct on this one
		if ((data != 8) && (data != 0))
		{
			logerror("bg2 control=%2x\n", data);
		}
	}
}

WRITE8_MEMBER(msisaac_state::msisaac_bg_videoram_w)
{
	m_videoram2[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(msisaac_state::msisaac_bg2_videoram_w)
{
	m_videoram3[offset] = data;
	m_bg2_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(msisaac_state::msisaac_fg_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}


/***************************************************************************

  Display refresh

***************************************************************************/
static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	msisaac_state *state = machine.driver_data<msisaac_state>();
	const UINT8 *source = state->m_spriteram + 32 * 4 - 4;
	const UINT8 *finish = state->m_spriteram; /* ? */

	while (source >= finish)
	{
		int sx = source[0];
		int sy = 240 - source[1] - 1;
		int attributes = source[2];
		int sprite_number = source[3];

		int color = (attributes >> 4) & 0xf;
		int flipx = (attributes & 0x1);
		int flipy = (attributes & 0x2);

		gfx_element *gfx = machine.gfx[2];

		if (attributes & 4)
		{
			//color = rand() & 15;
			gfx = machine.gfx[3];
		}

		if (attributes & 8)	/* double size sprite */
		{
			switch (attributes & 3)
			{
			case 0: /* flipx==0 && flipy==0 */
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number+1,color,
					flipx,flipy,
					sx,sy-16,0 );
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number,color,
					flipx,flipy,
					sx,sy,0 );
				break;
			case 1: /* flipx==1 && flipy==0 */
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number+1,color,
					flipx,flipy,
					sx,sy-16,0 );
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number,color,
					flipx,flipy,
					sx,sy,0 );
				break;
			case 2: /* flipx==0 && flipy==1 */
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number,color,
					flipx,flipy,
					sx,sy-16,0 );
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number+1,color,
					flipx,flipy,
					sx,sy,0 );
				break;
			case 3: /* flipx==1 && flipy==1 */
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number,color,
					flipx,flipy,
					sx,sy-16,0 );
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number+1,color,
					flipx,flipy,
					sx,sy,0 );
				break;
			}
		}
		else
		{
			drawgfx_transpen(bitmap,cliprect,gfx,
				sprite_number,
				color,
				flipx,flipy,
				sx,sy,0 );
		}
		source -= 4;
	}
}

SCREEN_UPDATE_IND16( msisaac )
{
	msisaac_state *state = screen.machine().driver_data<msisaac_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_bg2_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
