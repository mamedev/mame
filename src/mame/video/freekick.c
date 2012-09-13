/* Free Kick Video Hardware */

#include "emu.h"
#include "includes/freekick.h"


TILE_GET_INFO_MEMBER(freekick_state::get_freek_tile_info)
{
	int tileno, palno;

	tileno = m_videoram[tile_index] + ((m_videoram[tile_index + 0x400] & 0xe0) << 3);
	palno = m_videoram[tile_index + 0x400] & 0x1f;
	SET_TILE_INFO_MEMBER(0, tileno, palno, 0);
}


void freekick_state::video_start()
{
	m_freek_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(freekick_state::get_freek_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


WRITE8_MEMBER(freekick_state::freek_videoram_w)
{
	m_videoram[offset] = data;
	m_freek_tilemap->mark_tile_dirty(offset & 0x3ff);
}

static void gigas_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	freekick_state *state = machine.driver_data<freekick_state>();
	int offs;

	for (offs = 0; offs < state->m_spriteram.bytes(); offs += 4)
	{
		int xpos = state->m_spriteram[offs + 3];
		int ypos = state->m_spriteram[offs + 2];
		int code = state->m_spriteram[offs + 0] | ((state->m_spriteram[offs + 1] & 0x20) << 3);

		int flipx = 0;
		int flipy = 0;
		int color = state->m_spriteram[offs + 1] & 0x1f;

		if (state->flip_screen_x())
		{
			xpos = 240 - xpos;
			flipx = !flipx;
		}
		if (state->flip_screen_y())
		{
			ypos = 256 - ypos;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,flipy,
				xpos,240-ypos,0);
	}
}


static void pbillrd_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	freekick_state *state = machine.driver_data<freekick_state>();
	int offs;

	for (offs = 0; offs < state->m_spriteram.bytes(); offs += 4)
	{
		int xpos = state->m_spriteram[offs + 3];
		int ypos = state->m_spriteram[offs + 2];
		int code = state->m_spriteram[offs + 0];

		int flipx = 0;//state->m_spriteram[offs + 0] & 0x80; //?? unused ?
		int flipy = 0;//state->m_spriteram[offs + 0] & 0x40;
		int color = state->m_spriteram[offs + 1] & 0x0f;

		if (state->flip_screen_x())
		{
			xpos = 240 - xpos;
			flipx = !flipx;
		}
		if (state->flip_screen_y())
		{
			ypos = 256 - ypos;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,flipy,
				xpos,240-ypos,0);
	}
}



static void freekick_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	freekick_state *state = machine.driver_data<freekick_state>();
	int offs;

	for (offs = 0; offs < state->m_spriteram.bytes(); offs += 4)
	{
		int xpos = state->m_spriteram[offs + 3];
		int ypos = state->m_spriteram[offs + 0];
		int code = state->m_spriteram[offs + 1] + ((state->m_spriteram[offs + 2] & 0x20) << 3);

		int flipx = state->m_spriteram[offs + 2] & 0x80;	//?? unused ?
		int flipy = state->m_spriteram[offs + 2] & 0x40;
		int color = state->m_spriteram[offs + 2] & 0x1f;

		if (state->flip_screen_x())
		{
			xpos = 240 - xpos;
			flipx = !flipx;
		}
		if (state->flip_screen_y())
		{
			ypos = 256 - ypos;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,flipy,
				xpos,248-ypos,0);
	}
}

SCREEN_UPDATE_IND16( gigas )
{
	freekick_state *state = screen.machine().driver_data<freekick_state>();
	state->m_freek_tilemap->draw(bitmap, cliprect, 0, 0);
	gigas_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( pbillrd )
{
	freekick_state *state = screen.machine().driver_data<freekick_state>();
	state->m_freek_tilemap->draw(bitmap, cliprect, 0, 0);
	pbillrd_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( freekick )
{
	freekick_state *state = screen.machine().driver_data<freekick_state>();
	state->m_freek_tilemap->draw(bitmap, cliprect, 0, 0);
	freekick_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
