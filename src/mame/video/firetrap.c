/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/firetrap.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Fire Trap has one 256x8 and one 256x4 palette PROMs.
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT( firetrap )
{
	int i;


	for (i = 0; i < machine.total_colors(); i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;


		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		bit3 = (color_prom[i] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[i + machine.total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[i + machine.total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[i + machine.total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[i + machine.total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( get_fg_memory_offset )
{
	return (row ^ 0x1f) + (col << 5);
}

static TILEMAP_MAPPER( get_bg_memory_offset )
{
	return ((row & 0x0f) ^ 0x0f) | ((col & 0x0f) << 4) |
			/* hole at bit 8 */
			((row & 0x10) << 5) | ((col & 0x10) << 6);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	firetrap_state *state = machine.driver_data<firetrap_state>();
	int code = state->m_fgvideoram[tile_index];
	int color = state->m_fgvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			code | ((color & 0x01) << 8),
			color >> 4,
			0);
}

INLINE void get_bg_tile_info(running_machine &machine, tile_data &tileinfo, int tile_index, UINT8 *bgvideoram, int gfx_region)
{
	int code = bgvideoram[tile_index];
	int color = bgvideoram[tile_index + 0x100];
	SET_TILE_INFO(
			gfx_region,
			code + ((color & 0x03) << 8),
			(color & 0x30) >> 4,
			TILE_FLIPXY((color & 0x0c) >> 2));
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	firetrap_state *state = machine.driver_data<firetrap_state>();
	get_bg_tile_info(machine, tileinfo, tile_index, state->m_bg1videoram, 1);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	firetrap_state *state = machine.driver_data<firetrap_state>();
	get_bg_tile_info(machine, tileinfo, tile_index, state->m_bg2videoram, 2);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( firetrap )
{
	firetrap_state *state = machine.driver_data<firetrap_state>();
	state->m_fg_tilemap  = tilemap_create(machine, get_fg_tile_info, get_fg_memory_offset, 8, 8, 32, 32);
	state->m_bg1_tilemap = tilemap_create(machine, get_bg1_tile_info, get_bg_memory_offset, 16, 16, 32, 32);
	state->m_bg2_tilemap = tilemap_create(machine, get_bg2_tile_info, get_bg_memory_offset, 16, 16, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_bg1_tilemap->set_transparent_pen(0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(firetrap_state::firetrap_fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(firetrap_state::firetrap_bg1videoram_w)
{
	m_bg1videoram[offset] = data;
	m_bg1_tilemap->mark_tile_dirty(offset & 0x6ff);
}

WRITE8_MEMBER(firetrap_state::firetrap_bg2videoram_w)
{
	m_bg2videoram[offset] = data;
	m_bg2_tilemap->mark_tile_dirty(offset & 0x6ff);
}


WRITE8_MEMBER(firetrap_state::firetrap_bg1_scrollx_w)
{
	m_scroll1_x[offset] = data;
	m_bg1_tilemap->set_scrollx(0, m_scroll1_x[0] | (m_scroll1_x[1] << 8));
}

WRITE8_MEMBER(firetrap_state::firetrap_bg1_scrolly_w)
{
	m_scroll1_y[offset] = data;
	m_bg1_tilemap->set_scrolly(0, -(m_scroll1_y[0] | (m_scroll1_y[1] << 8)));
}

WRITE8_MEMBER(firetrap_state::firetrap_bg2_scrollx_w)
{
	m_scroll2_x[offset] = data;
	m_bg2_tilemap->set_scrollx(0, m_scroll2_x[0] | (m_scroll2_x[1] << 8));
}

WRITE8_MEMBER(firetrap_state::firetrap_bg2_scrolly_w)
{
	m_scroll2_y[offset] = data;
	m_bg2_tilemap->set_scrolly(0, -(m_scroll2_y[0] | (m_scroll2_y[1] << 8)));
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	firetrap_state *state = machine.driver_data<firetrap_state>();
	int offs;

	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		int sx, sy, flipx, flipy, code, color;


		/* the meaning of bit 3 of [offs] is unknown */

		sy = state->m_spriteram[offs];
		sx = state->m_spriteram[offs + 2];
		code = state->m_spriteram[offs + 3] + 4 * (state->m_spriteram[offs + 1] & 0xc0);
		color = ((state->m_spriteram[offs + 1] & 0x08) >> 2) | (state->m_spriteram[offs + 1] & 0x01);
		flipx = state->m_spriteram[offs + 1] & 0x04;
		flipy = state->m_spriteram[offs + 1] & 0x02;
		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (state->m_spriteram[offs + 1] & 0x10)	/* double width */
		{
			if (flip_screen_get(machine)) sy -= 16;

			drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
					code & ~1,
					color,
					flipx,flipy,
					sx,flipy ? sy : sy + 16,0);
			drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
					code | 1,
					color,
					flipx,flipy,
					sx,flipy ? sy + 16 : sy,0);

			/* redraw with wraparound */
			drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
					code & ~1,
					color,
					flipx,flipy,
					sx - 256,flipy ? sy : sy + 16,0);
			drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
					code | 1,
					color,
					flipx,flipy,
					sx - 256,flipy ? sy + 16 : sy,0);
		}
		else
		{
			drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
					code,
					color,
					flipx,flipy,
					sx,sy,0);

			/* redraw with wraparound */
			drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
					code,
					color,
					flipx,flipy,
					sx - 256,sy,0);
		}
	}
}

SCREEN_UPDATE_IND16( firetrap )
{
	firetrap_state *state = screen.machine().driver_data<firetrap_state>();
	state->m_bg2_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
