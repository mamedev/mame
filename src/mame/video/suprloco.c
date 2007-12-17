/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *suprloco_videoram;

static tilemap *bg_tilemap;
static int control;

#define SPR_Y_TOP		0
#define SPR_Y_BOTTOM	1
#define SPR_X			2
#define SPR_COL			3
#define SPR_SKIP_LO		4
#define SPR_SKIP_HI		5
#define SPR_GFXOFS_LO	6
#define SPR_GFXOFS_HI	7


/***************************************************************************

  Convert the color PROMs into a more useable format.

  I'm not sure about the resistor values, I'm using the Galaxian ones.

***************************************************************************/
PALETTE_INIT( suprloco )
{
	int i;


	for (i = 0;i < 512;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		/* hack: generate a second bank of sprite palette with red changed to purple */
		if (i >= 256)
		{
			if ((i & 0x0f) == 0x09)
				palette_set_color(machine,i+256,MAKE_RGB(r,g,0xff));
			else
				palette_set_color(machine,i+256,MAKE_RGB(r,g,b));
		}
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	UINT8 attr = suprloco_videoram[2*tile_index+1];
	SET_TILE_INFO(
			0,
			suprloco_videoram[2*tile_index] | ((attr & 0x03) << 8),
			(attr & 0x1c) >> 2,
			0);
	tileinfo->category = (attr & 0x20) >> 5;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( suprloco )
{
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_scroll_rows(bg_tilemap,32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( suprloco_videoram_w )
{
	suprloco_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset/2);
}

static int suprloco_scrollram[32];

WRITE8_HANDLER( suprloco_scrollram_w )
{
	int adj = flip_screen ? -8 : 8;

	suprloco_scrollram[offset] = data;
	tilemap_set_scrollx(bg_tilemap,offset, data - adj);
}

READ8_HANDLER( suprloco_scrollram_r )
{
	return suprloco_scrollram[offset];
}

WRITE8_HANDLER( suprloco_control_w )
{
	/* There is probably a palette select in here */

   	/* Bit 0   - coin counter A */
	/* Bit 1   - coin counter B (only used if coinage differs from A) */
	/* Bit 2-3 - probably unused */
	/* Bit 4   - ??? */
	/* Bit 5   - pulsated when loco turns "super" */
	/* Bit 6   - probably unused */
	/* Bit 7   - flip screen */

	if ((control & 0x10) != (data & 0x10))
	{
		/*logerror("Bit 4 = %d\n", (data >> 4) & 1); */
	}

	coin_counter_w(0, data & 0x01);
	coin_counter_w(1, data & 0x02);

	flip_screen_set(data & 0x80);
	tilemap_set_scrolly(bg_tilemap,0,flip_screen ? -32 : 0);

	control = data;
}


READ8_HANDLER( suprloco_control_r )
{
	return control;
}



INLINE void draw_pixel(mame_bitmap *bitmap,const rectangle *cliprect,int x,int y,int color)
{
	if (flip_screen)
	{
		x = bitmap->width - x - 1;
		y = bitmap->height - y - 1;
	}

	if (x < cliprect->min_x ||
		x > cliprect->max_x ||
		y < cliprect->min_y ||
		y > cliprect->max_y)
		return;

	*BITMAP_ADDR16(bitmap, y, x) = color;
}


static void draw_sprite(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int spr_number)
{
	int sx,sy,col,row,height,src,adjy,dy;
	UINT8 *spr_reg;
	const pen_t *spr_palette;
	short skip;	/* bytes to skip before drawing each row (can be negative) */


	spr_reg	= spriteram + 0x10 * spr_number;

	src = spr_reg[SPR_GFXOFS_LO] + (spr_reg[SPR_GFXOFS_HI] << 8);
	skip = spr_reg[SPR_SKIP_LO] + (spr_reg[SPR_SKIP_HI] << 8);

	height		= spr_reg[SPR_Y_BOTTOM] - spr_reg[SPR_Y_TOP];
	spr_palette	= machine->remapped_colortable + 0x100 + 0x10 * (spr_reg[SPR_COL]&0x03) + ((control & 0x20)?0x100:0);
	sx = spr_reg[SPR_X];
	sy = spr_reg[SPR_Y_TOP] + 1;

	if (!flip_screen)
	{
		adjy = sy;
		dy = 1;
	}
	else
	{
		adjy = sy + height + 30;  /* some of the sprites are still off by a pixel */
		dy = -1;
	}

	for (row = 0;row < height;row++,adjy+=dy)
	{
		int color1,color2,flipx;
		UINT8 data;
		UINT8 *gfx;

		src += skip;

		col = 0;

		/* get pointer to packed sprite data */
		gfx = &(memory_region(REGION_GFX2)[src & 0x7fff]);
		flipx = src & 0x8000;   /* flip x */

		while (1)
		{
			if (flipx)	/* flip x */
			{
				data = *gfx--;
				color1 = data & 0x0f;
				color2 = data >> 4;
			}
			else
			{
				data = *gfx++;
				color1 = data >> 4;
				color2 = data & 0x0f;
			}

			if (color1 == 15) break;
			if (color1)
				draw_pixel(bitmap,cliprect,sx+col,  adjy,spr_palette[color1]);

			if (color2 == 15) break;
			if (color2)
				draw_pixel(bitmap,cliprect,sx+col+1,adjy,spr_palette[color2]);

			col += 2;
		}
	}
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int spr_number;
	UINT8 *spr_reg;


	for (spr_number = 0;spr_number < (spriteram_size >> 4);spr_number++)
	{
		spr_reg = spriteram + 0x10 * spr_number;
		if (spr_reg[SPR_X] != 0xff)
			draw_sprite(machine, bitmap, cliprect, spr_number);
	}
}

VIDEO_UPDATE( suprloco )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,1,0);
	return 0;
}
