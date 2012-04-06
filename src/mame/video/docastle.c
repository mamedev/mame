/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  (Cocktail mode implemented by Chad Hendrickson Aug 1, 1999)

***************************************************************************/

#include "emu.h"
#include "includes/docastle.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mr. Do's Castle / Wild Ride / Run Run have a 256 bytes palette PROM which
  is connected to the RGB output this way:

  bit 7 -- 200 ohm resistor  -- RED
        -- 390 ohm resistor  -- RED
        -- 820 ohm resistor  -- RED
        -- 200 ohm resistor  -- GREEN
        -- 390 ohm resistor  -- GREEN
        -- 820 ohm resistor  -- GREEN
        -- 200 ohm resistor  -- BLUE
  bit 0 -- 390 ohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT( docastle )
{
	int i;

	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (*color_prom >> 5) & 0x01;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		r = 0x23 * bit0 + 0x4b * bit1 + 0x91 * bit2;
		/* green component */
		bit0 = (*color_prom >> 2) & 0x01;
		bit1 = (*color_prom >> 3) & 0x01;
		bit2 = (*color_prom >> 4) & 0x01;
		g = 0x23 * bit0 + 0x4b * bit1 + 0x91 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 0) & 0x01;
		bit2 = (*color_prom >> 1) & 0x01;
		b = 0x23 * bit0 + 0x4b * bit1 + 0x91 * bit2;

		/* because the graphics are decoded as 4bpp with the top bit used for transparency
           or priority, we create matching 3bpp sets of palette entries, which effectively
           ignores the value of the top bit */
		palette_set_color(machine, ((i & 0xf8) << 1) | 0x00 | (i & 0x07), MAKE_RGB(r,g,b));
		palette_set_color(machine, ((i & 0xf8) << 1) | 0x08 | (i & 0x07), MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_MEMBER(docastle_state::docastle_videoram_w)
{
	m_videoram[offset] = data;
	m_do_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(docastle_state::docastle_colorram_w)
{
	m_colorram[offset] = data;
	m_do_tilemap->mark_tile_dirty(offset);
}

READ8_MEMBER(docastle_state::docastle_flipscreen_off_r)
{
	flip_screen_set(machine(), 0);
	m_do_tilemap->mark_all_dirty();
	return 0;
}

READ8_MEMBER(docastle_state::docastle_flipscreen_on_r)
{
	flip_screen_set(machine(), 1);
	m_do_tilemap->mark_all_dirty();
	return 1;
}

WRITE8_MEMBER(docastle_state::docastle_flipscreen_off_w)
{
	flip_screen_set(machine(), 0);
	m_do_tilemap->mark_all_dirty();
}

WRITE8_MEMBER(docastle_state::docastle_flipscreen_on_w)
{
	flip_screen_set(machine(), 1);
	m_do_tilemap->mark_all_dirty();
}

static TILE_GET_INFO( get_tile_info )
{
	docastle_state *state = machine.driver_data<docastle_state>();
	int code = state->m_videoram[tile_index] + 8 * (state->m_colorram[tile_index] & 0x20);
	int color = state->m_colorram[tile_index] & 0x1f;

	SET_TILE_INFO(0, code, color, 0);
}

static void video_start_common( running_machine &machine, UINT32 tile_transmask )
{
	docastle_state *state = machine.driver_data<docastle_state>();
	state->m_do_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	state->m_do_tilemap->set_transmask(0, tile_transmask, 0x0000);
}

VIDEO_START( docastle )
{
	video_start_common(machine, 0x00ff);
}

VIDEO_START( dorunrun )
{
	video_start_common(machine, 0xff00);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	docastle_state *state = machine.driver_data<docastle_state>();
	int offs;

	machine.priority_bitmap.fill(1);

	for (offs = state->m_spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int sx, sy, flipx, flipy, code, color;

		if (machine.gfx[1]->total_elements > 256)
		{
			/* spriteram

             indoor soccer appears to have a slightly different spriteram
             format to the other games, allowing a larger number of sprite
             tiles

             yyyy yyyy  xxxx xxxx  TX-T pppp  tttt tttt

             y = ypos
             x = xpos
             X = x-flip
             T = extra tile number bits
             p = palette
             t = tile number

             */

			code = state->m_spriteram[offs + 3];
			color = state->m_spriteram[offs + 2] & 0x0f;
			sx = ((state->m_spriteram[offs + 1] + 8) & 0xff) - 8;
			sy = state->m_spriteram[offs];
			flipx = state->m_spriteram[offs + 2] & 0x40;
			flipy = 0;
			if (state->m_spriteram[offs + 2] & 0x10) code += 0x100;
			if (state->m_spriteram[offs + 2] & 0x80) code += 0x200;
		}
		else
		{
			/* spriteram

            this is the standard spriteram layout, used by most games

             yyyy yyyy  xxxx xxxx  YX-p pppp  tttt tttt

             y = ypos
             x = xpos
             X = x-flip
             Y = y-flip
             p = palette
             t = tile number

             */

			code = state->m_spriteram[offs + 3];
			color = state->m_spriteram[offs + 2] & 0x1f;
			sx = ((state->m_spriteram[offs + 1] + 8) & 0xff) - 8;
			sy = state->m_spriteram[offs];
			flipx = state->m_spriteram[offs + 2] & 0x40;
			flipy = state->m_spriteram[offs + 2] & 0x80;
		}

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		/* first draw the sprite, visible */
		pdrawgfx_transmask(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,flipy,
				sx,sy,
				machine.priority_bitmap,
				0x00,0x80ff);

		/* then draw the mask, behind the background but obscuring following sprites */
		pdrawgfx_transmask(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,flipy,
				sx,sy,
				machine.priority_bitmap,
				0x02,0x7fff);
	}
}

SCREEN_UPDATE_IND16( docastle )
{
	docastle_state *state = screen.machine().driver_data<docastle_state>();

	state->m_do_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_do_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}
