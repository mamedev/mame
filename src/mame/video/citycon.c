/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/citycon.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( citycon_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x60) << 5);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	citycon_state *state = machine.driver_data<citycon_state>();
	SET_TILE_INFO(
			0,
			state->m_videoram[tile_index],
			(tile_index & 0x03e0) >> 5,	/* color depends on scanline only */
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	citycon_state *state = machine.driver_data<citycon_state>();
	UINT8 *rom = machine.region("gfx4")->base();
	int code = rom[0x1000 * state->m_bg_image + tile_index];
	SET_TILE_INFO(
			3 + state->m_bg_image,
			code,
			rom[0xc000 + 0x100 * state->m_bg_image + code],
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( citycon )
{
	citycon_state *state = machine.driver_data<citycon_state>();
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, citycon_scan, 8, 8, 128, 32);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, citycon_scan, 8, 8, 128, 32);

	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_fg_tilemap->set_scroll_rows(32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(citycon_state::citycon_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(citycon_state::citycon_linecolor_w)
{
	m_linecolor[offset] = data;
}


WRITE8_MEMBER(citycon_state::citycon_background_w)
{

	/* bits 4-7 control the background image */
	if (m_bg_image != (data >> 4))
	{
		m_bg_image = (data >> 4);
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 0 flips screen */
	/* it is also used to multiplex player 1 and player 2 controls */
	flip_screen_set(machine(), data & 0x01);

	/* bits 1-3 are unknown */
//  if ((data & 0x0e) != 0) logerror("background register = %02x\n", data);
}



static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	citycon_state *state = machine.driver_data<citycon_state>();
	int offs;

	for (offs = state->m_spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int sx, sy, flipx;

		sx = state->m_spriteram[offs + 3];
		sy = 239 - state->m_spriteram[offs];
		flipx = ~state->m_spriteram[offs + 2] & 0x10;
		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 238 - sy;
			flipx = !flipx;
		}

		drawgfx_transpen(bitmap, cliprect, machine.gfx[state->m_spriteram[offs + 1] & 0x80 ? 2 : 1],
				state->m_spriteram[offs + 1] & 0x7f,
				state->m_spriteram[offs + 2] & 0x0f,
				flipx,flip_screen_get(machine),
				sx, sy, 0);
	}
}


INLINE void changecolor_RRRRGGGGBBBBxxxx( running_machine &machine, int color, int indx )
{
	citycon_state *state = machine.driver_data<citycon_state>();
	int data = state->m_generic_paletteram_8[2 * indx | 1] | (state->m_generic_paletteram_8[2 * indx] << 8);
	palette_set_color_rgb(machine, color, pal4bit(data >> 12), pal4bit(data >> 8), pal4bit(data >> 4));
}

SCREEN_UPDATE_IND16( citycon )
{
	citycon_state *state = screen.machine().driver_data<citycon_state>();
	int offs, scroll;

	/* Update the virtual palette to support text color code changing on every scanline. */
	for (offs = 0; offs < 256; offs++)
	{
		int indx = state->m_linecolor[offs];
		int i;

		for (i = 0; i < 4; i++)
			changecolor_RRRRGGGGBBBBxxxx(screen.machine(), 640 + 4 * offs + i, 512 + 4 * indx + i);
	}


	scroll = state->m_scroll[0] * 256 + state->m_scroll[1];
	state->m_bg_tilemap->set_scrollx(0, scroll >> 1);
	for (offs = 6; offs < 32; offs++)
		state->m_fg_tilemap->set_scrollx(offs, scroll);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
