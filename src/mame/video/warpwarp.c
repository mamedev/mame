/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "includes/warpwarp.h"


UINT8 *geebee_videoram,*warpwarp_videoram;
int geebee_handleoverlay;
int geebee_bgw;
int warpwarp_ball_on;
int warpwarp_ball_h,warpwarp_ball_v;
int warpwarp_ball_sizex, warpwarp_ball_sizey;

static tilemap *bg_tilemap;


static const rgb_t geebee_palette[] =
{
	MAKE_RGB(0x00,0x00,0x00), /* black */
	MAKE_RGB(0xff,0xff,0xff), /* white */
	MAKE_RGB(0x7f,0x7f,0x7f)  /* grey  */
};

static UINT16 geebee_colortable[] =
{
	 0, 1,
	 1, 0,
	 0, 2,
	 2, 0
};

static UINT16 navarone_colortable[] =
{
	 0, 2,
	 2, 0,
};


/* Initialise the palette */
PALETTE_INIT( geebee )
{
	int i;
	for (i = 0; i < sizeof(geebee_palette)/sizeof(geebee_palette[0]); i++)
		palette_set_color(machine,i,geebee_palette[i]);
	memcpy(colortable, geebee_colortable, sizeof (geebee_colortable));
}

/* Initialise the palette */
PALETTE_INIT( navarone )
{
	int i;
	for (i = 0; i < sizeof(geebee_palette)/sizeof(geebee_palette[0]); i++)
		palette_set_color(machine,i,geebee_palette[i]);
	memcpy(colortable, navarone_colortable, sizeof (navarone_colortable));
}


/***************************************************************************

  Warp Warp doesn't use PROMs - the 8-bit code is directly converted into a
  color.

  The color RAM is connected to the RGB output this way (I think - schematics
  are fuzzy):

  bit 7 -- 300 ohm resistor  -- BLUE
        -- 820 ohm resistor  -- BLUE
        -- 300 ohm resistor  -- GREEN
        -- 820 ohm resistor  -- GREEN
        -- 1.6kohm resistor  -- GREEN
        -- 300 ohm resistor  -- RED
        -- 820 ohm resistor  -- RED
  bit 0 -- 1.6kohm resistor  -- RED

  Moreover, the bullet is pure white, obtained with three 220 ohm resistors.

***************************************************************************/

PALETTE_INIT( warpwarp )
{
	int i;

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (i >> 0) & 0x01;
		bit1 = (i >> 1) & 0x01;
		bit2 = (i >> 2) & 0x01;
		r = 0x1f * bit0 + 0x3c * bit1 + 0xa4 * bit2;
		/* green component */
		bit0 = (i >> 3) & 0x01;
		bit1 = (i >> 4) & 0x01;
		bit2 = (i >> 5) & 0x01;
		g = 0x1f * bit0 + 0x3c * bit1 + 0xa4 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (i >> 6) & 0x01;
		bit2 = (i >> 7) & 0x01;
		b = 0x1f * bit0 + 0x3c * bit1 + 0xa4 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	for (i = 0;i < machine->drv->color_table_len;i += 2)
	{
		colortable[i] = 0;			/* black background */
		colortable[i + 1] = i / 2;	/* colored foreground */
	}
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
	bg_tilemap = tilemap_create(geebee_get_tile_info,tilemap_scan,TILEMAP_TYPE_PEN,8,8,34,28);
}

VIDEO_START( navarone )
{
	bg_tilemap = tilemap_create(navarone_get_tile_info,tilemap_scan,TILEMAP_TYPE_PEN,8,8,34,28);
}

VIDEO_START( warpwarp )
{
	bg_tilemap = tilemap_create(warpwarp_get_tile_info,tilemap_scan,TILEMAP_TYPE_PEN,8,8,34,28);
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

INLINE void geebee_plot(mame_bitmap *bitmap, const rectangle *cliprect, int x, int y, int pen)
{
	if (x >= cliprect->min_x && x <= cliprect->max_x && y >= cliprect->min_y && y <= cliprect->max_y)
		*BITMAP_ADDR16(bitmap, y, x) = pen;
}

static void draw_ball(mame_bitmap *bitmap, const rectangle *cliprect,int color)
{
	if (warpwarp_ball_on)
	{
		int x = 256+8 - warpwarp_ball_h;
		int y = 240 - warpwarp_ball_v;
		int i,j;

		for (i = warpwarp_ball_sizey;i > 0;i--)
		{
			for (j = warpwarp_ball_sizex;j > 0;j--)
			{
				geebee_plot(bitmap, cliprect, x-j, y-i, color);
			}
		}
	}
}

VIDEO_UPDATE( geebee )
{
	/* use an overlay only in upright mode */
	if (geebee_handleoverlay)
		output_set_value("overlay", (readinputport(2) & 0x01) == 0);

	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	draw_ball(bitmap,cliprect,1);
	return 0;
}



VIDEO_UPDATE( warpwarp )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	draw_ball(bitmap,cliprect,0xf6);
	return 0;
}
