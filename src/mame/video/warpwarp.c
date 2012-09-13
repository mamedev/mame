/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/warpwarp.h"


static const rgb_t geebee_palette[] =
{
	MAKE_RGB(0x00,0x00,0x00), /* black */
	MAKE_RGB(0xff,0xff,0xff), /* white */
	MAKE_RGB(0x7f,0x7f,0x7f)  /* grey  */
};

PALETTE_INIT_MEMBER(warpwarp_state,geebee)
{
	palette_set_color(machine(), 0, geebee_palette[0]);
	palette_set_color(machine(), 1, geebee_palette[1]);
	palette_set_color(machine(), 2, geebee_palette[1]);
	palette_set_color(machine(), 3, geebee_palette[0]);
	palette_set_color(machine(), 4, geebee_palette[0]);
	palette_set_color(machine(), 5, geebee_palette[2]);
	palette_set_color(machine(), 6, geebee_palette[2]);
	palette_set_color(machine(), 7, geebee_palette[0]);
}

PALETTE_INIT_MEMBER(warpwarp_state,navarone)
{
	palette_set_color(machine(), 0, geebee_palette[0]);
	palette_set_color(machine(), 1, geebee_palette[1]);
	palette_set_color(machine(), 2, geebee_palette[1]);
	palette_set_color(machine(), 3, geebee_palette[0]);
}


/***************************************************************************

  Warp Warp doesn't use PROMs - the 8-bit code is directly converted into a
  color.

  The color RAM is connected to the RGB output this way:

  bit 7 -- 390 ohm resistor  -- BLUE
        -- 820 ohm resistor  -- BLUE
        -- 390 ohm resistor  -- GREEN
        -- 820 ohm resistor  -- GREEN
        -- 1.6kohm resistor  -- GREEN
        -- 390 ohm resistor  -- RED
        -- 820 ohm resistor  -- RED
  bit 0 -- 1.6kohm resistor  -- RED

  Moreover, the bullet is pure white, obtained with three 220 ohm resistors.

***************************************************************************/

PALETTE_INIT_MEMBER(warpwarp_state,warpwarp)
{
	int i;
	static const int resistances_tiles_rg[] = { 1600, 820, 390 };
	static const int resistances_tiles_b[]  = { 820, 390 };
	static const int resistance_ball[]      = { 220 };

	double weights_tiles_rg[3], weights_tiles_b[2], weight_ball[1];

	compute_resistor_weights(0, 0xff, -1.0,
							 3, resistances_tiles_rg, weights_tiles_rg, 150, 0,
							 2, resistances_tiles_b,  weights_tiles_b,  150, 0,
							 1, resistance_ball,      weight_ball,      150, 0);

	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2;
		int r,g,b;

		/* red component */
		bit0 = (i >> 0) & 0x01;
		bit1 = (i >> 1) & 0x01;
		bit2 = (i >> 2) & 0x01;
		r = combine_3_weights(weights_tiles_rg, bit0, bit1, bit2);

		/* green component */
		bit0 = (i >> 3) & 0x01;
		bit1 = (i >> 4) & 0x01;
		bit2 = (i >> 5) & 0x01;
		g = combine_3_weights(weights_tiles_rg, bit0, bit1, bit2);

		/* blue component */
		bit0 = (i >> 6) & 0x01;
		bit1 = (i >> 7) & 0x01;
		b = combine_2_weights(weights_tiles_b, bit0, bit1);

		palette_set_color(machine(), (i * 2) + 0, RGB_BLACK);
		palette_set_color(machine(), (i * 2) + 1, MAKE_RGB(r, g, b));
	}

	palette_set_color(machine(), 0x200, MAKE_RGB(weight_ball[0], weight_ball[0], weight_ball[0]));
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 34x28 */
TILEMAP_MAPPER_MEMBER(warpwarp_state::tilemap_scan)
{
	int offs;

	row += 2;
	col--;
	if (col & 0x20)
		offs = row + ((col & 1) << 5);
	else
		offs = col + (row << 5);

	return offs;
}

TILE_GET_INFO_MEMBER(warpwarp_state::geebee_get_tile_info)
{
	int code = m_geebee_videoram[tile_index];
	int color = (m_geebee_bgw & 1) | ((code & 0x80) >> 6);
	SET_TILE_INFO_MEMBER(
			0,
			code,
			color,
			0);
}

TILE_GET_INFO_MEMBER(warpwarp_state::navarone_get_tile_info)
{
	int code = m_geebee_videoram[tile_index];
	int color = m_geebee_bgw & 1;
	SET_TILE_INFO_MEMBER(
			0,
			code,
			color,
			0);
}

TILE_GET_INFO_MEMBER(warpwarp_state::warpwarp_get_tile_info)
{
	SET_TILE_INFO_MEMBER(
			0,
			m_videoram[tile_index],
			m_videoram[tile_index + 0x400],
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(warpwarp_state,geebee)
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(warpwarp_state::geebee_get_tile_info),this),tilemap_mapper_delegate(FUNC(warpwarp_state::tilemap_scan),this),8,8,34,28);
}

VIDEO_START_MEMBER(warpwarp_state,navarone)
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(warpwarp_state::navarone_get_tile_info),this),tilemap_mapper_delegate(FUNC(warpwarp_state::tilemap_scan),this),8,8,34,28);
}

VIDEO_START_MEMBER(warpwarp_state,warpwarp)
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(warpwarp_state::warpwarp_get_tile_info),this),tilemap_mapper_delegate(FUNC(warpwarp_state::tilemap_scan),this),8,8,34,28);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(warpwarp_state::geebee_videoram_w)
{
	m_geebee_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(warpwarp_state::warpwarp_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

INLINE void geebee_plot(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, pen_t pen)
{
	if (cliprect.contains(x, y))
		bitmap.pix16(y, x) = pen;
}

static void draw_ball(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect,pen_t pen)
{
	warpwarp_state *state = machine.driver_data<warpwarp_state>();
	if (state->m_ball_on)
	{
		int x,y,i,j;

		if (state->flip_screen() & 1) {
			x = 376 - state->m_ball_h;
			y = 280 - state->m_ball_v;
		}
		else {
			x = 264 - state->m_ball_h;
			y = 240 - state->m_ball_v;
		}

		for (i = state->m_ball_sizey;i > 0;i--)
			for (j = state->m_ball_sizex;j > 0;j--)
				geebee_plot(bitmap, cliprect, x-j, y-i, pen);
	}
}

SCREEN_UPDATE_IND16( geebee )
{
	warpwarp_state *state = screen.machine().driver_data<warpwarp_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);

	draw_ball(screen.machine(), bitmap, cliprect, state->m_ball_pen);
	return 0;
}
