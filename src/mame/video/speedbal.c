 /****************************************************************************
 *                                                                           *
 * video.c                                                                 *
 *                                                                           *
 * Functions to emulate the video hardware of the machine.                   *
 *                                                                           *
 ****************************************************************************/

#include "emu.h"
#include "includes/speedbal.h"


static TILE_GET_INFO( get_tile_info_bg )
{
	speedbal_state *state = machine.driver_data<speedbal_state>();
	int code = state->m_background_videoram[tile_index*2] + ((state->m_background_videoram[tile_index*2+1] & 0x30) << 4);
	int color = state->m_background_videoram[tile_index*2+1] & 0x0f;

	SET_TILE_INFO(1, code, color, 0);
	tileinfo.group = (color == 8);
}

static TILE_GET_INFO( get_tile_info_fg )
{
	speedbal_state *state = machine.driver_data<speedbal_state>();
	int code = state->m_foreground_videoram[tile_index*2] + ((state->m_foreground_videoram[tile_index*2+1] & 0x30) << 4);
	int color = state->m_foreground_videoram[tile_index*2+1] & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
	tileinfo.group = (color == 9);
}

/*************************************
 *                                   *
 *      Start-Stop                   *
 *                                   *
 *************************************/

VIDEO_START( speedbal )
{
	speedbal_state *state = machine.driver_data<speedbal_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_tile_info_bg, tilemap_scan_cols_flip_x,  16, 16, 16, 16);
	state->m_fg_tilemap = tilemap_create(machine, get_tile_info_fg, tilemap_scan_cols_flip_x,   8,  8, 32, 32);

	state->m_bg_tilemap->set_transmask(0,0xffff,0x0000); /* split type 0 is totally transparent in front half */
	state->m_bg_tilemap->set_transmask(1,0x00f7,0x0000); /* split type 1 has pen 0-2, 4-7 transparent in front half */

	state->m_fg_tilemap->set_transmask(0,0xffff,0x0001); /* split type 0 is totally transparent in front half and has pen 0 transparent in back half */
	state->m_fg_tilemap->set_transmask(1,0x0001,0x0001); /* split type 1 has pen 0 transparent in front and back half */
}



/*************************************
 *                                   *
 *  Foreground characters RAM        *
 *                                   *
 *************************************/

WRITE8_MEMBER(speedbal_state::speedbal_foreground_videoram_w)
{
	m_foreground_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset>>1);
}

/*************************************
 *                                   *
 *  Background tiles RAM             *
 *                                   *
 *************************************/

WRITE8_MEMBER(speedbal_state::speedbal_background_videoram_w)
{
	m_background_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset>>1);
}


/*************************************
 *                                   *
 *   Sprite drawing                  *
 *                                   *
 *************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	speedbal_state *state = machine.driver_data<speedbal_state>();
	UINT8 *spriteram = state->m_spriteram;
	int x,y,code,color,offset,flipx,flipy;

	/* Drawing sprites: 64 in total */

	for (offset = 0;offset < state->m_spriteram_size;offset += 4)
	{
		if(!(spriteram[offset + 2] & 0x80))
			continue;

		x = 243 - spriteram[offset + 3];
		y = 239 - spriteram[offset + 0];

		code = BITSWAP8(spriteram[offset + 1],0,1,2,3,4,5,6,7) | ((spriteram[offset + 2] & 0x40) << 2);

		color = spriteram[offset + 2] & 0x0f;

		flipx = flipy = 0;

		if(flip_screen_get(machine))
		{
			x = 246 - x;
			y = 238 - y;
			flipx = flipy = 1;
		}

		drawgfx_transpen (bitmap,cliprect,machine.gfx[2],
				code,
				color,
				flipx,flipy,
				x,y,0);
	}
}

/*************************************
 *                                   *
 *   Refresh screen                  *
 *                                   *
 *************************************/

SCREEN_UPDATE_IND16( speedbal )
{
	speedbal_state *state = screen.machine().driver_data<speedbal_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}
