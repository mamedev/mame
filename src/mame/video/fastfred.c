/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/fastfred.h"



/***************************************************************************

  Convert the color PROMs into a more useable format.

  bit 0 -- 1  kohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 220 ohm resistor  -- RED/GREEN/BLUE
  bit 3 -- 100 ohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT( fastfred )
{
	static const int resistances[4] = { 1000, 470, 220, 100 };
	double rweights[4], gweights[4], bweights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			4, resistances, rweights, 470, 0,
			4, resistances, gweights, 470, 0,
			4, resistances, bweights, 470, 0);

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x000] >> 3) & 0x01;
		r = combine_4_weights(rweights, bit0, bit1, bit2, bit3);

		/* green component */
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x100] >> 3) & 0x01;
		g = combine_4_weights(gweights, bit0, bit1, bit2, bit3);

		/* blue component */
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		b = combine_4_weights(bweights, bit0, bit1, bit2, bit3);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* characters and sprites use the same palette */
	for (i = 0; i < 0x100; i++)
		colortable_entry_set_value(machine.colortable, i, i);
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	fastfred_state *state = machine.driver_data<fastfred_state>();
	UINT8 x = tile_index & 0x1f;

	UINT16 code = state->m_charbank | state->m_videoram[tile_index];
	UINT8 color = state->m_colorbank | (state->m_attributesram[2 * x + 1] & 0x07);

	SET_TILE_INFO(0, code, color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( fastfred )
{
	fastfred_state *state = machine.driver_data<fastfred_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_tile_info,tilemap_scan_rows,8,8,32,32);

	state->m_bg_tilemap->set_transparent_pen(0);
	state->m_bg_tilemap->set_scroll_cols(32);
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( fastfred_videoram_w )
{
	fastfred_state *state = space->machine().driver_data<fastfred_state>();
	state->m_videoram[offset] = data;
	state->m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_HANDLER( fastfred_attributes_w )
{
	fastfred_state *state = space->machine().driver_data<fastfred_state>();
	if (state->m_attributesram[offset] != data)
	{
		if (offset & 0x01)
		{
			/* color change */
			int i;

			for (i = offset / 2; i < 0x0400; i += 32)
				state->m_bg_tilemap->mark_tile_dirty(i);
		}
		else
		{
			/* coloumn scroll */
			state->m_bg_tilemap->set_scrolly(offset / 2, data);
		}

		state->m_attributesram[offset] = data;
	}
}


WRITE8_HANDLER( fastfred_charbank1_w )
{
	fastfred_state *state = space->machine().driver_data<fastfred_state>();
	UINT16 new_data = (state->m_charbank & 0x0200) | ((data & 0x01) << 8);

	if (new_data != state->m_charbank)
	{
		state->m_bg_tilemap->mark_all_dirty();

		state->m_charbank = new_data;
	}
}

WRITE8_HANDLER( fastfred_charbank2_w )
{
	fastfred_state *state = space->machine().driver_data<fastfred_state>();
	UINT16 new_data = (state->m_charbank & 0x0100) | ((data & 0x01) << 9);

	if (new_data != state->m_charbank)
	{
		state->m_bg_tilemap->mark_all_dirty();

		state->m_charbank = new_data;
	}
}


WRITE8_HANDLER( fastfred_colorbank1_w )
{
	fastfred_state *state = space->machine().driver_data<fastfred_state>();
	UINT8 new_data = (state->m_colorbank & 0x10) | ((data & 0x01) << 3);

	if (new_data != state->m_colorbank)
	{
		state->m_bg_tilemap->mark_all_dirty();

		state->m_colorbank = new_data;
	}
}

WRITE8_HANDLER( fastfred_colorbank2_w )
{
	fastfred_state *state = space->machine().driver_data<fastfred_state>();
	UINT8 new_data = (state->m_colorbank & 0x08) | ((data & 0x01) << 4);

	if (new_data != state->m_colorbank)
	{
		state->m_bg_tilemap->mark_all_dirty();

		state->m_colorbank = new_data;
	}
}



WRITE8_HANDLER( fastfred_flip_screen_x_w )
{
	fastfred_state *state = space->machine().driver_data<fastfred_state>();
	if (state->flip_screen_x() != (data & 0x01))
	{
		state->flip_screen_x_set(data & 0x01);

		state->m_bg_tilemap->set_flip((state->flip_screen_x() ? TILEMAP_FLIPX : 0) | (state->flip_screen_y() ? TILEMAP_FLIPY : 0));
	}
}

WRITE8_HANDLER( fastfred_flip_screen_y_w )
{
	fastfred_state *state = space->machine().driver_data<fastfred_state>();
	if (state->flip_screen_y() != (data & 0x01))
	{
		state->flip_screen_y_set(data & 0x01);

		state->m_bg_tilemap->set_flip((state->flip_screen_x() ? TILEMAP_FLIPX : 0) | (state->flip_screen_y() ? TILEMAP_FLIPY : 0));
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle spritevisiblearea(2*8, 32*8-1, 2*8, 30*8-1);
	const rectangle spritevisibleareaflipx(0*8, 30*8-1, 2*8, 30*8-1);
	fastfred_state *state = machine.driver_data<fastfred_state>();
	int offs;

	for (offs = state->m_spriteram_size - 4; offs >= 0; offs -= 4)
	{
		UINT8 code,sx,sy;
		int flipx,flipy;

		sx = state->m_spriteram[offs + 3];
		sy = 240 - state->m_spriteram[offs];

		if (state->m_hardware_type == 3)
		{
			// Imago
			code  = (state->m_spriteram[offs + 1]) & 0x3f;
			flipx = 0;
			flipy = 0;
		}
		else if (state->m_hardware_type == 2)
		{
			// Boggy 84
			code  =  state->m_spriteram[offs + 1] & 0x7f;
			flipx =  0;
			flipy =  state->m_spriteram[offs + 1] & 0x80;
		}
		else if (state->m_hardware_type == 1)
		{
			// Fly-Boy/Fast Freddie/Red Robin
			code  =  state->m_spriteram[offs + 1] & 0x7f;
			flipx =  0;
			flipy = ~state->m_spriteram[offs + 1] & 0x80;
		}
		else
		{
			// Jump Coaster
			code  = (state->m_spriteram[offs + 1] & 0x3f) | 0x40;
			flipx = ~state->m_spriteram[offs + 1] & 0x40;
			flipy =  state->m_spriteram[offs + 1] & 0x80;
		}


		if (state->flip_screen_x())
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (state->flip_screen_y())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,state->flip_screen_x() ? spritevisibleareaflipx : spritevisiblearea,machine.gfx[1],
				code,
				state->m_colorbank | (state->m_spriteram[offs + 2] & 0x07),
				flipx,flipy,
				sx,sy,0);
	}
}


SCREEN_UPDATE_IND16( fastfred )
{
	fastfred_state *state = screen.machine().driver_data<fastfred_state>();
	bitmap.fill(*state->m_background_color, cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap, cliprect);

	return 0;
}


static TILE_GET_INFO( imago_get_tile_info_bg )
{
	fastfred_state *state = machine.driver_data<fastfred_state>();
	UINT8 x = tile_index & 0x1f;

	UINT16 code = state->m_charbank * 0x100 + state->m_videoram[tile_index];
	UINT8 color = state->m_colorbank | (state->m_attributesram[2 * x + 1] & 0x07);

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( imago_get_tile_info_fg )
{
	fastfred_state *state = machine.driver_data<fastfred_state>();
	int code = state->m_imago_fg_videoram[tile_index];
	SET_TILE_INFO(2, code, 2, 0);
}

static TILE_GET_INFO( imago_get_tile_info_web )
{
	SET_TILE_INFO(3, tile_index & 0x1ff, 0, 0);
}

WRITE8_HANDLER( imago_fg_videoram_w )
{
	fastfred_state *state = space->machine().driver_data<fastfred_state>();
	state->m_imago_fg_videoram[offset] = data;
	state->m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_HANDLER( imago_charbank_w )
{
	fastfred_state *state = space->machine().driver_data<fastfred_state>();
	if( state->m_charbank != data )
	{
		state->m_charbank = data;
		state->m_bg_tilemap->mark_all_dirty();
	}
}

VIDEO_START( imago )
{
	fastfred_state *state = machine.driver_data<fastfred_state>();
	state->m_web_tilemap = tilemap_create(machine, imago_get_tile_info_web,tilemap_scan_rows,     8,8,32,32);
	state->m_bg_tilemap   = tilemap_create(machine, imago_get_tile_info_bg, tilemap_scan_rows,8,8,32,32);
	state->m_fg_tilemap   = tilemap_create(machine, imago_get_tile_info_fg, tilemap_scan_rows,8,8,32,32);

	state->m_bg_tilemap->set_transparent_pen(0);
	state->m_fg_tilemap->set_transparent_pen(0);

	/* the game has a galaxian starfield */
	galaxold_init_stars(machine, 256);
	state->m_stars_on = 1;

	/* web colors */
	palette_set_color(machine,256+64+0,MAKE_RGB(0x50,0x00,0x00));
	palette_set_color(machine,256+64+1,MAKE_RGB(0x00,0x00,0x00));
}

SCREEN_UPDATE_IND16( imago )
{
	fastfred_state *state = screen.machine().driver_data<fastfred_state>();
	state->m_web_tilemap->draw(bitmap, cliprect, 0,0);
	galaxold_draw_stars(screen.machine(), bitmap, cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0,0);

	return 0;
}
