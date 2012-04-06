/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/bagman.h"


WRITE8_MEMBER(bagman_state::bagman_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bagman_state::bagman_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Bagman has two 32 bytes palette PROMs, connected to the RGB output this
  way:

  bit 7 -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- BLUE

        -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- GREEN
        -- 1  kohm resistor  -- /

        -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- RED
  bit 0 -- 1  kohm resistor  -- /

***************************************************************************/
PALETTE_INIT( bagman )
{
	int i;
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[2];


	compute_resistor_weights(0,	255,	-1.0,
			3,	resistances_rg,	weights_r,	470,	0,
			3,	resistances_rg,	weights_g,	470,	0,
			2,	resistances_b,	weights_b,	470,	0);


	for (i = 0; i < machine.total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_g, bit0, bit1, bit2);
		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

WRITE8_MEMBER(bagman_state::bagman_flipscreen_w)
{
	if ((flip_screen_get(machine()) ^ data) & 1)
	{
		flip_screen_set(machine(), data & 0x01);
		m_bg_tilemap->mark_all_dirty();
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	bagman_state *state = machine.driver_data<bagman_state>();
	int gfxbank = (machine.gfx[2] && (state->m_colorram[tile_index] & 0x10)) ? 2 : 0;
	int code = state->m_videoram[tile_index] + 8 * (state->m_colorram[tile_index] & 0x20);
	int color = state->m_colorram[tile_index] & 0x0f;

	SET_TILE_INFO(gfxbank, code, color, 0);
}

VIDEO_START( bagman )
{
	bagman_state *state = machine.driver_data<bagman_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	state->m_bg_tilemap->set_scrolldy(-1, -1);
}


static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bagman_state *state = machine.driver_data<bagman_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = state->m_spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int sx,sy,flipx,flipy;

		sx = spriteram[offs + 3];
		sy = 255 - spriteram[offs + 2] - 16;
		flipx = spriteram[offs] & 0x40;
		flipy = spriteram[offs] & 0x80;
		if (flip_screen_x_get(machine))
		{
			sx = bitmap.width() - sx - 15;
			flipx = !flipx;
		}
		if (flip_screen_y_get(machine))
		{
			sy = bitmap.height() - sy - 15;
			flipy = !flipy;
		}

		if (spriteram[offs + 2] && spriteram[offs + 3])
			drawgfx_transpen(bitmap,
					cliprect,machine.gfx[1],
					(spriteram[offs] & 0x3f) + 2 * (spriteram[offs + 1] & 0x20),
					spriteram[offs + 1] & 0x1f,
					flipx,flipy,
					sx,sy,0);
	}
}

SCREEN_UPDATE_IND16( bagman )
{
	bagman_state *state = screen.machine().driver_data<bagman_state>();
	bitmap.fill(0, cliprect);
	if (*state->m_video_enable == 0)
		return 0;

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
