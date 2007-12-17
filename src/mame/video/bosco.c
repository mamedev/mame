/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "includes/galaga.h"


#define MAX_STARS 252
#define STARS_COLOR_BASE (64*4+64*4+4)

static UINT32 stars_scrollx;
static UINT32 stars_scrolly;

static INT32 bosco_starcontrol,bosco_starblink[2];

static tilemap *bg_tilemap,*fg_tilemap;

#define VIDEO_RAM_SIZE 0x400

UINT8 *bosco_videoram;
UINT8 *bosco_radarattr;
static UINT8 *bosco_radarx,*bosco_radary;

static colortable *bosco_colortable;


PALETTE_INIT( bosco )
{
	int i;

	bosco_colortable = colortable_alloc(machine, 32+64);

	/* core palette */
	for (i = 0;i < 32;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		bit0 = ((*color_prom) >> 0) & 0x01;
		bit1 = ((*color_prom) >> 1) & 0x01;
		bit2 = ((*color_prom) >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = ((*color_prom) >> 3) & 0x01;
		bit1 = ((*color_prom) >> 4) & 0x01;
		bit2 = ((*color_prom) >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = ((*color_prom) >> 6) & 0x01;
		bit2 = ((*color_prom) >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(bosco_colortable,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* palette for the stars */
	for (i = 0;i < 64;i++)
	{
		int bits,r,g,b;
		static const int map[4] = { 0x00, 0x47, 0x97 ,0xde };

		bits = (i >> 0) & 0x03;
		r = map[bits];
		bits = (i >> 2) & 0x03;
		g = map[bits];
		bits = (i >> 4) & 0x03;
		b = map[bits];

		colortable_palette_set_color(bosco_colortable,32 + i,MAKE_RGB(r,g,b));
	}

	/* characters / sprites */
	for (i = 0;i < 64*4;i++)
	{
		colortable_entry_set_value(bosco_colortable, i, (color_prom[i] & 0x0f) + 0x10);	/* chars */
		colortable_entry_set_value(bosco_colortable, i+64*4, color_prom[i] & 0x0f);	/* sprites */
	}

	/* bullets lookup table */
	/* they use colors 28-31, I think - PAL 5A controls it */
	for (i = 0;i < 4;i++)
		colortable_entry_set_value(bosco_colortable, 64*4+64*4+i, 31-i);

	/* now the stars */
	for (i = 0;i < 64;i++)
		colortable_entry_set_value(bosco_colortable, 64*4+64*4+4+i, 32 + i);
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* the video RAM has space for 32x32 tiles and is only partially used for the radar */
static TILEMAP_MAPPER( fg_tilemap_scan )
{
	return col + (row << 5);
}


INLINE void get_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int ram_offs)
{
	UINT8 attr = bosco_videoram[ram_offs + tile_index + 0x800];
	tileinfo->category = (attr & 0x20) >> 5;
	tileinfo->group = attr & 0x3f;
	SET_TILE_INFO(
			0,
			bosco_videoram[ram_offs + tile_index],
			attr & 0x3f,
			TILE_FLIPYX(attr >> 6) ^ TILE_FLIPX);
}

static TILE_GET_INFO( bg_get_tile_info )
{
	get_tile_info(machine,tileinfo,tile_index,0x400);
}

static TILE_GET_INFO( fg_get_tile_info )
{
	get_tile_info(machine,tileinfo,tile_index,0x000);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( bosco )
{
	bg_tilemap = tilemap_create(bg_get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	fg_tilemap = tilemap_create(fg_get_tile_info,fg_tilemap_scan,  TILEMAP_TYPE_PEN,8,8, 8,32);

	colortable_configure_tilemap_groups(bosco_colortable, bg_tilemap, machine->gfx[0], 0x1f);
	colortable_configure_tilemap_groups(bosco_colortable, fg_tilemap, machine->gfx[0], 0x1f);

	tilemap_set_scrolldx(bg_tilemap,3,3);

	spriteram_size = 0x0c;
	spriteram = bosco_videoram + 0x03d4;
	spriteram_2 = spriteram + 0x0800;
	bosco_radarx = bosco_videoram + 0x03f0;
	bosco_radary = bosco_radarx + 0x0800;


	state_save_register_global(stars_scrollx);
	state_save_register_global(stars_scrolly);
	state_save_register_global(bosco_starcontrol);
	state_save_register_global_array(bosco_starblink);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

READ8_HANDLER( bosco_videoram_r )
{
	return bosco_videoram[offset];
}

WRITE8_HANDLER( bosco_videoram_w )
{
	bosco_videoram[offset] = data;
	if (offset & 0x400)
		tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
	else
		tilemap_mark_tile_dirty(fg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( bosco_scrollx_w )
{
	tilemap_set_scrollx(bg_tilemap,0,data);
}

WRITE8_HANDLER( bosco_scrolly_w )
{
	tilemap_set_scrolly(bg_tilemap,0,data);
}

WRITE8_HANDLER( bosco_starcontrol_w )
{
	bosco_starcontrol = data;
}

WRITE8_HANDLER( bosco_starblink_w )
{
	bosco_starblink[offset] = data & 1;
}

WRITE8_HANDLER( bosco_starclr_w )
{
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = 0;offs < spriteram_size;offs += 2)
	{
		int sx = spriteram[offs + 1] - 1;
		int sy = 240 - spriteram_2[offs];
		int flipx = spriteram[offs] & 1;
		int flipy = spriteram[offs] & 2;
		int color = spriteram_2[offs + 1] & 0x3f;
		if (flip_screen) sx += 32-2;

		drawgfx(bitmap,machine->gfx[1],
				(spriteram[offs] & 0xfc) >> 2,
				color,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PENS,
				colortable_get_transpen_mask(bosco_colortable, machine->gfx[1], color, 0x0f));
	}
}


static void draw_bullets(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = 4; offs < 0x10;offs++)
	{
		int x,y;

		x = bosco_radarx[offs] + ((~bosco_radarattr[offs] & 0x01) << 8);
		y = 253 - bosco_radary[offs];
		if (flip_screen) x -= 3;

		drawgfx(bitmap,machine->gfx[2],
				((bosco_radarattr[offs] & 0x0e) >> 1) ^ 0x07,
				0,
				0,0,
				x,y,
				cliprect,TRANSPARENCY_PENS,0xf0);
	}
}


static void draw_stars(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	if (1)
	{
		int star_cntr;
		int set_a, set_b;

		/* two sets of stars controlled by these bits */
		set_a = bosco_starblink[0];
		set_b = bosco_starblink[1] |0x2;

		for (star_cntr = 0;star_cntr < MAX_STARS;star_cntr++)
		{
			int x,y;

			if   ( (set_a == star_seed_tab[star_cntr].set) ||  ( set_b == star_seed_tab[star_cntr].set) )
			{
				x = (  star_seed_tab[star_cntr].x + stars_scrollx) % 256;
				y = (  star_seed_tab[star_cntr].y + stars_scrolly) % 256;

				/* dont draw the stars that are off the screen */
				if ( x < 224 && y < 224 )
				{
					if (flip_screen) x += 64;

					if (y >= machine->screen[0].visarea.min_y && y <= machine->screen[0].visarea.max_y)
						*BITMAP_ADDR16(bitmap, y, x) = STARS_COLOR_BASE + star_seed_tab[star_cntr].col;
				 }
			}
		}
	}
}


VIDEO_UPDATE( bosco )
{
	/* the radar tilemap is just 8x32. We rely on the tilemap code to repeat it across
       the screen, and clip it to only the position where it is supposed to be shown */
	rectangle fg_clip = *cliprect;
	rectangle bg_clip = *cliprect;
	if (flip_screen)
	{
		bg_clip.min_x = 8*8;
		fg_clip.max_x = 8*8-1;
	}
	else
	{
		bg_clip.max_x = 28*8-1;
		fg_clip.min_x = 28*8;
	}

	fillbitmap(bitmap,get_black_pen(machine),cliprect);
	draw_stars(machine, bitmap,cliprect);

	tilemap_draw(bitmap,&bg_clip,bg_tilemap,0,0);
	tilemap_draw(bitmap,&fg_clip,fg_tilemap,0,0);

	draw_sprites(machine, bitmap,cliprect);

	/* draw the high priority characters */
	tilemap_draw(bitmap,&bg_clip,bg_tilemap,1,0);
	tilemap_draw(bitmap,&fg_clip,fg_tilemap,1,0);

	draw_bullets(machine, bitmap,cliprect);

	return 0;
}


VIDEO_EOF( bosco )
{
	static const int speedsx[8] = { -1, -2, -3, 0, 3, 2, 1, 0 };
	static const int speedsy[8] = { 0, -1, -2, -3, 0, 3, 2, 1 };

	stars_scrollx += speedsx[bosco_starcontrol & 0x07];
	stars_scrolly += speedsy[(bosco_starcontrol & 0x38) >> 3];
}
