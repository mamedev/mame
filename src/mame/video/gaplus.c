/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"


UINT8 *gaplus_videoram;
UINT8 *gaplus_spriteram;

static tilemap *bg_tilemap;
static colortable *gaplus_colortable;


/***************************************************************************

  Convert the color PROMs.

  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT( gaplus )
{
	int i;

	/* allocate the colortable */
	gaplus_colortable = colortable_alloc(machine, 256);

	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x100] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		colortable_palette_set_color(gaplus_colortable,i,MAKE_RGB(r,g,b));
	}

	color_prom += 0x300;
	/* color_prom now points to the beginning of the lookup table */

	/* characters use colors 0xf0-0xff */
	for (i = 0;i < machine->gfx[0]->total_colors * machine->gfx[0]->color_granularity;i++)
		colortable_entry_set_value(gaplus_colortable, machine->gfx[0]->color_base + i, 0xf0 + (*color_prom++ & 0x0f));

	/* sprites */
	for (i = 0;i < machine->gfx[1]->total_colors * machine->gfx[1]->color_granularity;i++)
	{
		colortable_entry_set_value(gaplus_colortable, machine->gfx[1]->color_base + i, (color_prom[0] & 0x0f) + ((color_prom[0x200] & 0x0f) << 4));
		color_prom++;
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
static TILEMAP_MAPPER( tilemap_scan )
{
	int offs;

	row += 2;
	col -= 2;
	if (col & 0x20)
		offs = row + ((col & 0x1f) << 5);
	else
		offs = col + (row << 5);

	return offs;
}

static TILE_GET_INFO( get_tile_info )
{
	UINT8 attr = gaplus_videoram[tile_index + 0x400];
	tileinfo->category = (attr & 0x40) >> 6;
	tileinfo->group = attr & 0x3f;
	SET_TILE_INFO(
			0,
			gaplus_videoram[tile_index] + ((attr & 0x80) << 1),
			attr & 0x3f,
			0);
}



/***************************************************************************
    Starfield information
    There's 3 sets of stars planes at different speeds.

    a000 ---> (bit 0 = 1) enable starfield.
              (bit 0 = 0) disable starfield.
    a001 ---> starfield plane 0 control
    a002 ---> starfield plane 1 control
    a003 ---> starfield plane 2 control
***************************************************************************/

#define MAX_STARS			250

/* starfield speed constants (bigger = faster) */
#define SPEED_1 0.5
#define SPEED_2 1.0
#define SPEED_3 2.0

struct star {
	float x,y;
	int col,set;
};
static struct star stars[MAX_STARS];

static UINT8 gaplus_starfield_control[4];
static int total_stars;

static void starfield_init(running_machine *machine)
{
	int generator = 0;
	int x,y;
	int set = 0;
	int width, height;

	width = machine->screen[0].width;
	height = machine->screen[0].height;

	total_stars = 0;

	/* precalculate the star background */
	/* this comes from the Galaxian hardware, Gaplus is probably different */

	for ( y = 0;y < height; y++ ) {
		for ( x = width*2 - 1; x >= 0; x--) {
			int bit1,bit2;

			generator <<= 1;
			bit1 = (~generator >> 17) & 1;
			bit2 = (generator >> 5) & 1;

			if (bit1 ^ bit2) generator |= 1;

			if ( ((~generator >> 16) & 1) && (generator & 0xff) == 0xff) {
				int color;

				color = (~(generator >> 8)) & 0x3f;
				if ( color && total_stars < MAX_STARS ) {
					stars[total_stars].x = x;
					stars[total_stars].y = y;
					stars[total_stars].col = machine->pens[color];
					stars[total_stars].set = set++;

					if ( set == 3 )
						set = 0;

					total_stars++;
				}
			}
		}
	}
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gaplus )
{
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan,TILEMAP_TYPE_PEN,8,8,36,28);

	colortable_configure_tilemap_groups(gaplus_colortable, bg_tilemap, machine->gfx[0], 0xff);

	spriteram = gaplus_spriteram + 0x780;
	spriteram_2 = spriteram + 0x800;
	spriteram_3 = spriteram_2 + 0x800;

	starfield_init(machine);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

READ8_HANDLER( gaplus_videoram_r )
{
	return gaplus_videoram[offset];
}

WRITE8_HANDLER( gaplus_videoram_w )
{
	gaplus_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( gaplus_starfield_control_w )
{
	offset &= 3;
	gaplus_starfield_control[offset] = data;
}



/***************************************************************************

    Display Refresh

***************************************************************************/

static void starfield_render(running_machine *machine, mame_bitmap *bitmap)
{
	int i;
	int width, height;

	width = machine->screen[0].width;
	height = machine->screen[0].height;

	/* check if we're running */
	if ( ( gaplus_starfield_control[0] & 1 ) == 0 )
		return;

	/* draw the starfields */
	for ( i = 0; i < total_stars; i++ )
	{
		int x, y;

		x = stars[i].x;
		y = stars[i].y;

		if ( x >=0 && x < width && y >= 0 && y < height )
		{
			*BITMAP_ADDR16(bitmap, y, x) = stars[i].col;
		}
	}
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		/* is it on? */
		if ((spriteram_3[offs+1] & 2) == 0)
		{
			static int gfx_offs[2][2] =
			{
				{ 0, 1 },
				{ 2, 3 }
			};
			int sprite = spriteram[offs] | ((spriteram_3[offs] & 0x40) << 2);
			int color = spriteram[offs+1] & 0x3f;
			int sx = spriteram_2[offs+1] + 0x100 * (spriteram_3[offs+1] & 1) - 71;
			int sy = 256 - spriteram_2[offs] - 8;
			int flipx = (spriteram_3[offs] & 0x01);
			int flipy = (spriteram_3[offs] & 0x02) >> 1;
			int sizex = (spriteram_3[offs] & 0x08) >> 3;
			int sizey = (spriteram_3[offs] & 0x20) >> 5;
			int duplicate = spriteram_3[offs] & 0x80;
			int x,y;

			if (flip_screen)
			{
				flipx ^= 1;
				flipy ^= 1;
			}

			sy -= 16 * sizey;
			sy = (sy & 0xff) - 32;	// fix wraparound

			for (y = 0;y <= sizey;y++)
			{
				for (x = 0;x <= sizex;x++)
				{
					drawgfx(bitmap,machine->gfx[1],
						sprite + (duplicate ? 0 : (gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)])),
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,
						cliprect,TRANSPARENCY_PENS,
						colortable_get_transpen_mask(gaplus_colortable, machine->gfx[1], color, 0xff));
				}
			}
		}
	}
}

VIDEO_UPDATE( gaplus )
{
	/* flip screen control is embedded in RAM */
	flip_screen_set(gaplus_spriteram[0x1f7f-0x800] & 1);

	fillbitmap(bitmap, machine->pens[0], cliprect);

	starfield_render(machine, bitmap);

	/* draw the low priority characters */
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	draw_sprites(machine, bitmap, cliprect);

	/* draw the high priority characters */
	/* (I don't know if this feature is used by Gaplus, but it's shown in the schematics) */
	tilemap_draw(bitmap,cliprect,bg_tilemap,1,0);
	return 0;
}


VIDEO_EOF( gaplus )	/* update starfields */
{
	int i;
	int width, height;

	width = machine->screen[0].width;
	height = machine->screen[0].height;

	/* check if we're running */
	if ( ( gaplus_starfield_control[0] & 1 ) == 0 )
		return;

	/* update the starfields */
	for ( i = 0; i < total_stars; i++ ) {
		switch( gaplus_starfield_control[stars[i].set + 1] ) {
			case 0x87:
				/* stand still */
			break;

			case 0x86:
				/* scroll down (speed 1) */
				stars[i].x += SPEED_1;
			break;

			case 0x85:
				/* scroll down (speed 2) */
				stars[i].x += SPEED_2;
			break;

			case 0x06:
				/* scroll down (speed 3) */
				stars[i].x += SPEED_3;
			break;

			case 0x80:
				/* scroll up (speed 1) */
				stars[i].x -= SPEED_1;
			break;

			case 0x82:
				/* scroll up (speed 2) */
				stars[i].x -= SPEED_2;
			break;

			case 0x81:
				/* scroll up (speed 3) */
				stars[i].x -= SPEED_3;
			break;

			case 0x9f:
				/* scroll left (speed 2) */
				stars[i].y += SPEED_2;
			break;

			case 0xaf:
				/* scroll left (speed 1) */
				stars[i].y += SPEED_1;
			break;
		}

		/* wrap */
		if ( stars[i].x < 0 )
			stars[i].x = ( float )( width*2 ) + stars[i].x;

		if ( stars[i].x >= ( float )( width*2 ) )
			stars[i].x -= ( float )( width*2 );

		if ( stars[i].y < 0 )
			stars[i].y = ( float )( height ) + stars[i].y;

		if ( stars[i].y >= ( float )( height ) )
			stars[i].y -= ( float )( height );
	}
}
