/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/bombjack.h"

WRITE8_MEMBER(bombjack_state::bombjack_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bombjack_state::bombjack_colorram_w)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bombjack_state::bombjack_background_w)
{

	if (m_background_image != data)
	{
		m_background_image = data;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(bombjack_state::bombjack_flipscreen_w)
{
	if (flip_screen_get(machine()) != (data & 0x01))
	{
		flip_screen_set(machine(), data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	bombjack_state *state = machine.driver_data<bombjack_state>();
	UINT8 *tilerom = machine.region("gfx4")->base();

	int offs = (state->m_background_image & 0x07) * 0x200 + tile_index;
	int code = (state->m_background_image & 0x10) ? tilerom[offs] : 0;
	int attr = tilerom[offs + 0x100];
	int color = attr & 0x0f;
	int flags = (attr & 0x80) ? TILE_FLIPY : 0;

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	bombjack_state *state = machine.driver_data<bombjack_state>();
	int code = state->m_videoram[tile_index] + 16 * (state->m_colorram[tile_index] & 0x10);
	int color = state->m_colorram[tile_index] & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( bombjack )
{
	bombjack_state *state = machine.driver_data<bombjack_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 16, 16);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(0);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	bombjack_state *state = machine.driver_data<bombjack_state>();
	int offs;

	for (offs = state->m_spriteram_size - 4; offs >= 0; offs -= 4)
	{

/*
 abbbbbbb cdefgggg hhhhhhhh iiiiiiii

 a        use big sprites (32x32 instead of 16x16)
 bbbbbbb  sprite code
 c        x flip
 d        y flip (used only in death sequence?)
 e        ? (set when big sprites are selected)
 f        ? (set only when the bonus (B) materializes?)
 gggg     color
 hhhhhhhh x position
 iiiiiiii y position
*/
		int sx,sy,flipx,flipy;


		sx = state->m_spriteram[offs + 3];

		if (state->m_spriteram[offs] & 0x80)
			sy = 225 - state->m_spriteram[offs + 2];
		else
			sy = 241 - state->m_spriteram[offs + 2];

		flipx = state->m_spriteram[offs + 1] & 0x40;
		flipy = state->m_spriteram[offs + 1] & 0x80;

		if (flip_screen_get(machine))
		{
			if (state->m_spriteram[offs + 1] & 0x20)
			{
				sx = 224 - sx;
				sy = 224 - sy;
			}
			else
			{
				sx = 240 - sx;
				sy = 240 - sy;
			}
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[(state->m_spriteram[offs] & 0x80) ? 3 : 2],
				state->m_spriteram[offs] & 0x7f,
				state->m_spriteram[offs + 1] & 0x0f,
				flipx,flipy,
				sx,sy,0);
	}
}

SCREEN_UPDATE_IND16( bombjack )
{
	bombjack_state *state = screen.machine().driver_data<bombjack_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
