/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/warpwarp.h"


UINT8 *geebee_videoram,*warpwarp_videoram;
int geebee_handleoverlay;
int geebee_bgw;
int warpwarp_ball_on;
int warpwarp_ball_h,warpwarp_ball_v;
int warpwarp_ball_sizex, warpwarp_ball_sizey;

static tilemap_t *bg_tilemap;


static const rgb_t geebee_palette[] =
{
	MAKE_RGB(0x00,0x00,0x00), /* black */
	MAKE_RGB(0xff,0xff,0xff), /* white */
	MAKE_RGB(0x7f,0x7f,0x7f)  /* grey  */
};

PALETTE_INIT( geebee )
{
	palette_set_color(machine, 0, geebee_palette[0]);
	palette_set_color(machine, 1, geebee_palette[1]);
	palette_set_color(machine, 2, geebee_palette[1]);
	palette_set_color(machine, 3, geebee_palette[0]);
	palette_set_color(machine, 4, geebee_palette[0]);
	palette_set_color(machine, 5, geebee_palette[2]);
	palette_set_color(machine, 6, geebee_palette[2]);
	palette_set_color(machine, 7, geebee_palette[0]);
}

PALETTE_INIT( navarone )
{
	palette_set_color(machine, 0, geebee_palette[0]);
	palette_set_color(machine, 1, geebee_palette[2]);
	palette_set_color(machine, 2, geebee_palette[2]);
	palette_set_color(machine, 3, geebee_palette[0]);
	palette_set_color(machine, 4, geebee_palette[1]);
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

PALETTE_INIT( warpwarp )
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

		palette_set_color(machine, (i * 2) + 0, RGB_BLACK);
		palette_set_color(machine, (i * 2) + 1, MAKE_RGB(r, g, b));
	}

	palette_set_color(machine, 0x200, MAKE_RGB(weight_ball[0], weight_ball[0], weight_ball[0]));
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 34x28 */
static UINT32 tilemap_scan(UINT32 col,UINT32 row,UINT32 num_cols,UINT32 num_rows)
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

static TILE_GET_INFO( geebee_get_tile_info )
{
	int code = geebee_videoram[tile_index];
	int color = (geebee_bgw & 1) | ((code & 0x80) >> 6);
	SET_TILE_INFO(
			0,
			code,
			color,
			0);
}

static TILE_GET_INFO( navarone_get_tile_info )
{
	int code = geebee_videoram[tile_index];
	int color = geebee_bgw & 1;
	SET_TILE_INFO(
			0,
			code,
			color,
			0);
}

static TILE_GET_INFO( warpwarp_get_tile_info )
{
	SET_TILE_INFO(
			0,
			warpwarp_videoram[tile_index],
			warpwarp_videoram[tile_index + 0x400],
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( geebee )
{
	bg_tilemap = tilemap_create(machine, geebee_get_tile_info,tilemap_scan,8,8,34,28);
}

VIDEO_START( navarone )
{
	bg_tilemap = tilemap_create(machine, navarone_get_tile_info,tilemap_scan,8,8,34,28);
}

VIDEO_START( warpwarp )
{
	bg_tilemap = tilemap_create(machine, warpwarp_get_tile_info,tilemap_scan,8,8,34,28);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( geebee_videoram_w )
{
	geebee_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( warpwarp_videoram_w )
{
	warpwarp_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

INLINE void geebee_plot(bitmap_t *bitmap, const rectangle *cliprect, int x, int y, pen_t pen)
{
	if (x >= cliprect->min_x && x <= cliprect->max_x && y >= cliprect->min_y && y <= cliprect->max_y)
		*BITMAP_ADDR16(bitmap, y, x) = pen;
}

static void draw_ball(bitmap_t *bitmap, const rectangle *cliprect,pen_t pen)
{
	if (warpwarp_ball_on)
	{
		int x = 256+8 - warpwarp_ball_h;
		int y = 240 - warpwarp_ball_v;
		int i,j;

		for (i = warpwarp_ball_sizey;i > 0;i--)
			for (j = warpwarp_ball_sizex;j > 0;j--)
				geebee_plot(bitmap, cliprect, x-j, y-i, pen);
	}
}

VIDEO_UPDATE( geebee )
{
	/* use an overlay only in upright mode */
	if (geebee_handleoverlay)
		output_set_value("overlay", (input_port_read(screen->machine, "DSW2") & 0x01) == 0);

	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	draw_ball(bitmap,cliprect,1);
	return 0;
}


VIDEO_UPDATE( navarone )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	draw_ball(bitmap,cliprect,4);
	return 0;
}


VIDEO_UPDATE( warpwarp )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	draw_ball(bitmap,cliprect,0x200);
	return 0;
}
