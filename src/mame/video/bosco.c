/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/galaga.h"


#define MAX_STARS 252
#define STARS_COLOR_BASE (64*4+64*4+4)
#define VIDEO_RAM_SIZE 0x400

PALETTE_INIT( bosco )
{
	int i;

	machine->colortable = colortable_alloc(machine, 32+64);

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

		colortable_palette_set_color(machine->colortable,i,MAKE_RGB(r,g,b));
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

		colortable_palette_set_color(machine->colortable,32 + i,MAKE_RGB(r,g,b));
	}

	/* characters / sprites */
	for (i = 0;i < 64*4;i++)
	{
		colortable_entry_set_value(machine->colortable, i, (color_prom[i] & 0x0f) + 0x10);	/* chars */
		colortable_entry_set_value(machine->colortable, i+64*4, color_prom[i] & 0x0f);	/* sprites */
	}

	/* bullets lookup table */
	/* they use colors 28-31, I think - PAL 5A controls it */
	for (i = 0;i < 4;i++)
		colortable_entry_set_value(machine->colortable, 64*4+64*4+i, 31-i);

	/* now the stars */
	for (i = 0;i < 64;i++)
		colortable_entry_set_value(machine->colortable, 64*4+64*4+4+i, 32 + i);
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
	_galaga_state *state = (_galaga_state *) machine->driver_data;

	UINT8 attr = state->videoram[ram_offs + tile_index + 0x800];
	tileinfo->category = (attr & 0x20) >> 5;
	tileinfo->group = attr & 0x3f;
	SET_TILE_INFO(
			0,
			state->videoram[ram_offs + tile_index],
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
	_galaga_state *state = (_galaga_state *) machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, bg_get_tile_info,tilemap_scan_rows,8,8,32,32);
	state->fg_tilemap = tilemap_create(machine, fg_get_tile_info,fg_tilemap_scan,  8,8, 8,32);

	colortable_configure_tilemap_groups(machine->colortable, state->bg_tilemap, machine->gfx[0], 0x1f);
	colortable_configure_tilemap_groups(machine->colortable, state->fg_tilemap, machine->gfx[0], 0x1f);

	tilemap_set_scrolldx(state->bg_tilemap,3,3);

	machine->generic.spriteram_size = 0x0c;
	machine->generic.spriteram.u8 = state->videoram + 0x03d4;
	machine->generic.spriteram2.u8 = machine->generic.spriteram.u8 + 0x0800;
	state->bosco_radarx = state->videoram + 0x03f0;
	state->bosco_radary = state->bosco_radarx + 0x0800;


	state_save_register_global(machine, state->stars_scrollx);
	state_save_register_global(machine, state->stars_scrolly);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( bosco_videoram_w )
{
	_galaga_state *state = (_galaga_state *) space->machine->driver_data;

	state->videoram[offset] = data;
	if (offset & 0x400)
		tilemap_mark_tile_dirty(state->bg_tilemap,offset & 0x3ff);
	else
		tilemap_mark_tile_dirty(state->fg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( bosco_scrollx_w )
{
	_galaga_state *state = (_galaga_state *) space->machine->driver_data;

	tilemap_set_scrollx(state->bg_tilemap,0,data);
}

WRITE8_HANDLER( bosco_scrolly_w )
{
	_galaga_state *state = (_galaga_state *) space->machine->driver_data;
	tilemap_set_scrolly(state->bg_tilemap,0,data);
}

WRITE8_HANDLER( bosco_starclr_w )
{
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	UINT8 *spriteram_2 = machine->generic.spriteram2.u8;
	int offs;

	for (offs = 0;offs < machine->generic.spriteram_size;offs += 2)
	{
		int sx = spriteram[offs + 1] - 1;
		int sy = 240 - spriteram_2[offs];
		int flipx = spriteram[offs] & 1;
		int flipy = spriteram[offs] & 2;
		int color = spriteram_2[offs + 1] & 0x3f;
		if (flip_screen_get(machine)) sx += 32-2;

		drawgfx_transmask(bitmap,cliprect,machine->gfx[1],
				(spriteram[offs] & 0xfc) >> 2,
				color,
				flipx,flipy,
				sx,sy,
				colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, 0x0f));
	}
}


static void draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	_galaga_state *state = (_galaga_state *) machine->driver_data;
	int offs;

	for (offs = 4; offs < 0x10;offs++)
	{
		int x,y;

		x = state->bosco_radarx[offs] + ((~state->bosco_radarattr[offs] & 0x01) << 8);
		y = 253 - state->bosco_radary[offs];
		if (flip_screen_get(machine)) x -= 3;

		drawgfx_transmask(bitmap,cliprect,machine->gfx[2],
				((state->bosco_radarattr[offs] & 0x0e) >> 1) ^ 0x07,
				0,
				0,0,
				x,y,0xf0);
	}
}


static void draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int flip)
{
	_galaga_state *state = (_galaga_state *) machine->driver_data;

	if (1)
	{
		int star_cntr;
		int set_a, set_b;

		/* two sets of stars controlled by these bits */
		set_a = state->bosco_starblink[0] & 0x01;
		set_b = (state->bosco_starblink[1] & 0x01) |0x2;

		for (star_cntr = 0;star_cntr < MAX_STARS;star_cntr++)
		{
			int x,y;

			if   ( (set_a == star_seed_tab[star_cntr].set) ||  ( set_b == star_seed_tab[star_cntr].set) )
			{
				x = (  star_seed_tab[star_cntr].x + state->stars_scrollx) % 256;
				y = (  star_seed_tab[star_cntr].y + state->stars_scrolly) % 256;

				/* dont draw the stars that are off the screen */
				if ( x < 224 && y < 224 )
				{
					if (flip) x += 64;

					if (y >= cliprect->min_y && y <= cliprect->max_y)
						*BITMAP_ADDR16(bitmap, y, x) = STARS_COLOR_BASE + star_seed_tab[star_cntr].col;
				}
			}
		}
	}
}


VIDEO_UPDATE( bosco )
{
	_galaga_state *state = (_galaga_state *) screen->machine->driver_data;

	/* the radar tilemap is just 8x32. We rely on the tilemap code to repeat it across
       the screen, and clip it to only the position where it is supposed to be shown */
	rectangle fg_clip = *cliprect;
	rectangle bg_clip = *cliprect;
	if (flip_screen_get(screen->machine))
	{
		bg_clip.min_x = 8*8;
		fg_clip.max_x = 8*8-1;
	}
	else
	{
		bg_clip.max_x = 28*8-1;
		fg_clip.min_x = 28*8;
	}

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	draw_stars(screen->machine,bitmap,cliprect,flip_screen_get(screen->machine));

	tilemap_draw(bitmap,&bg_clip,state->bg_tilemap,0,0);
	tilemap_draw(bitmap,&fg_clip,state->fg_tilemap,0,0);

	draw_sprites(screen->machine, bitmap,cliprect);

	/* draw the high priority characters */
	tilemap_draw(bitmap,&bg_clip,state->bg_tilemap,1,0);
	tilemap_draw(bitmap,&fg_clip,state->fg_tilemap,1,0);

	draw_bullets(screen->machine, bitmap,cliprect);

	return 0;
}


VIDEO_EOF( bosco )
{
	_galaga_state *state = (_galaga_state *) machine->driver_data;
	static const int speedsx[8] = { -1, -2, -3, 0, 3, 2, 1, 0 };
	static const int speedsy[8] = { 0, -1, -2, -3, 0, 3, 2, 1 };

	state->stars_scrollx += speedsx[state->bosco_starcontrol[0] & 0x07];
	state->stars_scrolly += speedsy[(state->bosco_starcontrol[0] & 0x38) >> 3];
}
