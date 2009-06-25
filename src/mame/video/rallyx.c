/***************************************************************************

The video mixer of this hardware is peculiar.

There are two 16-colors palette banks: the first is used for characters and
sprites, the second for "bullets".

When a bullet is on screen, it selects the second palette bank and replaces
the bottom 2 bits of the tile palette entry with its own, while leaving the
other 2 bits untouched. Therefore, in theory a bullet could have 4 different
colors depending on the color of the background it is drawn over; but none
of the games use this peculiarity, since the bullet palette is just the same
colors repeated four time. This is NOT emulated.

When there is a sprite under the bullet, the palette bank is changed, but the
palette entry number is NOT changed; therefore, the sprite pixels that are
covered by the bullet just change bank. This is emulated by first drawing the
bullets normally, then drawing the sprites (with pdrawgfx so they are not
drawn over high priority tiles), then drawing the pullets again with
drawgfx_transtable mode, so that bullets not covered by sprites remain
the same while the others alter the sprite color.


The tile/sprite priority is controlled by the top bit of the tile color code.
This feature seems to be disabled in Jungler, probably because that game
needs more color combination to render its graphics.

***************************************************************************/

#include "driver.h"
#include "video/resnet.h"



UINT8 *rallyx_videoram,*rallyx_radarattr;

static UINT8 *rallyx_radarx,*rallyx_radary;
static int spriteram_base;

static tilemap *bg_tilemap,*fg_tilemap;

#define MAX_STARS 1000
#define STARS_COLOR_BASE	(0x104)

static int stars_enable;

struct star
{
	int x,y,color;
};
static struct star stars[MAX_STARS];
static int total_stars;
static UINT8 drawmode_table[4];


/***************************************************************************

  Convert the color PROMs.

  Rally X has one 32x8 palette PROM and one 256x4 color lookup table PROM.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  In Rally-X there is a 1 kohm pull-down on B only, in Locomotion the
  1 kohm pull-down is an all three RGB outputs.

***************************************************************************/
PALETTE_INIT( rallyx )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, &resistances_rg[0], rweights,    0, 0,
			3, &resistances_rg[0], gweights,    0, 0,
			2, &resistances_b[0],  bweights, 1000, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* character/sprites lookup table */
	for (i = 0x000; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* bullets use colors 0x10-0x13 */
	for (i = 0x100; i < 0x104; i++)
		colortable_entry_set_value(machine->colortable, i, (i - 0x100) | 0x10);
}


PALETTE_INIT( jungler )
{
	static const int resistances_rg[3]   = { 1000, 470, 220 };
	static const int resistances_b [2]   = { 470, 220 };
	static const int resistances_star[3] = { 150, 100 };
	double rweights[3], gweights[3], bweights[2];
	double rweights_star[2], gweights_star[2], bweights_star[2];
	int i;

	/* compute the color output resistor weights */
	double scale = compute_resistor_weights(0,	255, -1.0,
						2, resistances_star, rweights_star, 0, 0,
						2, resistances_star, gweights_star, 0, 0,
						2, resistances_star, bweights_star, 0, 0);

				   compute_resistor_weights(0,	255, scale,
						3, resistances_rg, rweights, 1000, 0,
						3, resistances_rg, gweights, 1000, 0,
						2, resistances_b,  bweights, 1000, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x60);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* star pens */
	for (i = 0x20; i < 0x60; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = ((i - 0x20) >> 0) & 0x01;
		bit1 = ((i - 0x20) >> 1) & 0x01;
		r = combine_2_weights(rweights_star, bit0, bit1);

		/* green component */
		bit0 = ((i - 0x20) >> 2) & 0x01;
		bit1 = ((i - 0x20) >> 3) & 0x01;
		g = combine_2_weights(gweights_star, bit0, bit1);

		/* blue component */
		bit0 = ((i - 0x20) >> 4) & 0x01;
		bit1 = ((i - 0x20) >> 5) & 0x01;
		b = combine_2_weights(bweights_star, bit0, bit1);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* character/sprites lookup table */
	for (i = 0x000; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* bullets use colors 0x10-0x13 */
	for (i = 0x100; i < 0x104; i++)
		colortable_entry_set_value(machine->colortable, i, (i - 0x100) | 0x10);

	/* stars */
	for (i = 0x104; i < 0x144; i++)
		colortable_entry_set_value(machine->colortable, i, (i - 0x104) + 0x20);
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* the video RAM has space for 32x32 tiles and is only partially used for the radar */
static TILEMAP_MAPPER( fg_tilemap_scan )
{
	return col + (row << 5);
}


INLINE void rallyx_get_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int ram_offs)
{
	UINT8 attr = rallyx_videoram[ram_offs + tile_index + 0x800];
	tileinfo->category = (attr & 0x20) >> 5;
	SET_TILE_INFO(
			0,
			rallyx_videoram[ram_offs + tile_index],
			attr & 0x3f,
			TILE_FLIPYX(attr >> 6) ^ TILE_FLIPX);
}

static TILE_GET_INFO( rallyx_bg_get_tile_info )
{
	rallyx_get_tile_info(machine,tileinfo,tile_index,0x400);
}

static TILE_GET_INFO( rallyx_fg_get_tile_info )
{
	rallyx_get_tile_info(machine,tileinfo,tile_index,0x000);
}


INLINE void locomotn_get_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int ram_offs)
{
	UINT8 attr = rallyx_videoram[ram_offs + tile_index + 0x800];
	int code = rallyx_videoram[ram_offs + tile_index];
	code = (code & 0x7f) + 2*(attr & 0x40) + 2*(code & 0x80);
	tileinfo->category = (attr & 0x20) >> 5;
	SET_TILE_INFO(
			0,
			code,
			attr & 0x3f,
			(attr & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}

static TILE_GET_INFO( locomotn_bg_get_tile_info )
{
	locomotn_get_tile_info(machine,tileinfo,tile_index,0x400);
}

static TILE_GET_INFO( locomotn_fg_get_tile_info )
{
	locomotn_get_tile_info(machine,tileinfo,tile_index,0x000);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void calculate_star_field(void)
{
	int generator;
	int x,y;

	/* precalculate the star background */
	total_stars = 0;
	generator = 0;

	for (y = 0;y < 256;y++)
	{
		for (x = 0;x < 288;x++)
		{
			int bit1,bit2;

			generator <<= 1;
			bit1 = (~generator >> 17) & 1;
			bit2 = (generator >> 5) & 1;

			if (bit1 ^ bit2) generator |= 1;

			if (((~generator >> 16) & 1) &&
					(generator & 0xfe) == 0xfe)
			{
				int color;

				color = (~(generator >> 8)) & 0x3f;

				if (color && total_stars < MAX_STARS)
				{
					stars[total_stars].x = x;
					stars[total_stars].y = y;
					stars[total_stars].color = color;

					total_stars++;
				}
			}
		}
	}
}


VIDEO_START( rallyx )
{
	int i;

	bg_tilemap = tilemap_create(machine, rallyx_bg_get_tile_info,tilemap_scan_rows,8,8,32,32);
	fg_tilemap = tilemap_create(machine, rallyx_fg_get_tile_info,fg_tilemap_scan,  8,8, 8,32);

	/* the scrolling tilemap is slightly misplaced in Rally X */
	tilemap_set_scrolldx(bg_tilemap,3,3);

	spriteram_base = 0x14;

	spriteram = rallyx_videoram + 0x00;
	spriteram_2 = spriteram + 0x800;
	rallyx_radarx = rallyx_videoram + 0x20;
	rallyx_radary = rallyx_radarx + 0x800;

	for (i = 0;i < 16;i++)
		machine->shadow_table[i] = i+16;

	for (i = 16;i < 32;i++)
		machine->shadow_table[i] = i;

	for (i = 0;i < 3;i++)
		drawmode_table[i] = DRAWMODE_SHADOW;

	drawmode_table[3] = DRAWMODE_NONE;
}


VIDEO_START( jungler )
{
	int i;

	bg_tilemap = tilemap_create(machine, rallyx_bg_get_tile_info,tilemap_scan_rows,8,8,32,32);
	fg_tilemap = tilemap_create(machine, rallyx_fg_get_tile_info,fg_tilemap_scan,  8,8, 8,32);

	spriteram_base = 0x14;

	spriteram = rallyx_videoram + 0x00;
	spriteram_2 = spriteram + 0x800;
	rallyx_radarx = rallyx_videoram + 0x20;
	rallyx_radary = rallyx_radarx + 0x800;

	for (i = 0;i < 16;i++)
		machine->shadow_table[i] = i+16;

	for (i = 16;i < 32;i++)
		machine->shadow_table[i] = i;

	for (i = 0;i < 3;i++)
		drawmode_table[i] = DRAWMODE_SHADOW;

	drawmode_table[3] = DRAWMODE_NONE;

	calculate_star_field();
}


VIDEO_START( locomotn )
{
	int i;

	bg_tilemap = tilemap_create(machine, locomotn_bg_get_tile_info,tilemap_scan_rows,8,8,32,32);
	fg_tilemap = tilemap_create(machine, locomotn_fg_get_tile_info,fg_tilemap_scan,  8,8, 8,32);

	/* handle reduced visible area in some games */
	if (video_screen_get_visible_area(machine->primary_screen)->max_x == 32*8-1)
	{
		tilemap_set_scrolldx(bg_tilemap,0,32);
		tilemap_set_scrolldx(fg_tilemap,0,32);
	}

	spriteram_base = 0x14;

	spriteram = rallyx_videoram + 0x00;
	spriteram_2 = spriteram + 0x800;
	rallyx_radarx = rallyx_videoram + 0x20;
	rallyx_radary = rallyx_radarx + 0x800;

	for (i = 0;i < 16;i++)
		machine->shadow_table[i] = i+16;

	for (i = 16;i < 32;i++)
		machine->shadow_table[i] = i;

	for (i = 0;i < 3;i++)
		drawmode_table[i] = DRAWMODE_SHADOW;

	drawmode_table[3] = DRAWMODE_NONE;

	calculate_star_field();
}


VIDEO_START( commsega )
{
	int i;

	bg_tilemap = tilemap_create(machine, locomotn_bg_get_tile_info,tilemap_scan_rows,8,8,32,32);
	fg_tilemap = tilemap_create(machine, locomotn_fg_get_tile_info,fg_tilemap_scan,  8,8, 8,32);

	/* handle reduced visible area in some games */
	if (video_screen_get_visible_area(machine->primary_screen)->max_x == 32*8-1)
	{
		tilemap_set_scrolldx(bg_tilemap,0,32);
		tilemap_set_scrolldx(fg_tilemap,0,32);
	}

	/* commsega has more sprites and bullets than the other games */
	spriteram_base = 0x00;

	spriteram = rallyx_videoram + 0x00;
	spriteram_2 = spriteram + 0x800;
	rallyx_radarx = rallyx_videoram + 0x20;
	rallyx_radary = rallyx_radarx + 0x800;

	for (i = 0;i < 16;i++)
		machine->shadow_table[i] = i+16;

	for (i = 16;i < 32;i++)
		machine->shadow_table[i] = i;

	for (i = 0;i < 3;i++)
		drawmode_table[i] = DRAWMODE_SHADOW;

	drawmode_table[3] = DRAWMODE_NONE;

	calculate_star_field();
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( rallyx_videoram_w )
{
	rallyx_videoram[offset] = data;
	if (offset & 0x400)
		tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
	else
		tilemap_mark_tile_dirty(fg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( rallyx_scrollx_w )
{
	tilemap_set_scrollx(bg_tilemap,0,data);
}

WRITE8_HANDLER( rallyx_scrolly_w )
{
	tilemap_set_scrolly(bg_tilemap,0,data);
}

WRITE8_HANDLER( tactcian_starson_w )
{
	stars_enable = data & 1;
}



static void plot_star(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int x, int y, int color)
{
	if (y < cliprect->min_y ||
		y > cliprect->max_y ||
		x < cliprect->min_x ||
		x > cliprect->max_x)
		return;

	if (flip_screen_x_get(machine))
		x = 255 - x;

	if (flip_screen_y_get(machine))
		y = 255 - y;

	if (colortable_entry_get_value(machine->colortable, *BITMAP_ADDR16(bitmap, y, x) % 0x144) == 0)
		*BITMAP_ADDR16(bitmap, y, x) = STARS_COLOR_BASE + color;
}

static void draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = 0;offs < total_stars;offs++)
	{
		int x = stars[offs].x;
		int y = stars[offs].y;

		if ((y & 0x01) ^ ((x >> 3) & 0x01))
			plot_star(machine, bitmap, cliprect, x, y, stars[offs].color);
	}
}


static void rallyx_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int displacement )
{
	int offs;

	for (offs = 0x20-2;offs >= spriteram_base;offs -= 2)
	{
		int sx = spriteram[offs + 1] + ((spriteram_2[offs + 1] & 0x80) << 1) - displacement;
		int sy = 241 - spriteram_2[offs] - displacement;
		int color = spriteram_2[offs + 1] & 0x3f;
		int flipx = spriteram[offs] & 1;
		int flipy = spriteram[offs] & 2;
		if (flip_screen_get(machine)) sx -= 2*displacement;

		pdrawgfx_transmask(bitmap,cliprect,machine->gfx[1],
				(spriteram[offs] & 0xfc) >> 2,
				color,
				flipx,flipy,
				sx,sy,
				priority_bitmap,0x02,
				colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, 0));
	}
}

static void locomotn_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int displacement )
{
	int offs;

	for (offs = 0x20-2;offs >= spriteram_base;offs -= 2)
	{
		int sx = spriteram[offs + 1] + ((spriteram_2[offs + 1] & 0x80) << 1);
		int sy = 241 - spriteram_2[offs] - displacement;
		int color = spriteram_2[offs + 1] & 0x3f;
		int flip = spriteram[offs] & 2;

		pdrawgfx_transmask(bitmap,cliprect,machine->gfx[1],
				((spriteram[offs] & 0x7c) >> 2) + 0x20*(spriteram[offs] & 0x01) + ((spriteram[offs] & 0x80) >> 1),
				color,
				flip,flip,
				sx,sy,
				priority_bitmap,0x02,
				colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, 0));
	}
}

static void rallyx_draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int transpen )
{
	int offs;

	for (offs = spriteram_base; offs < 0x20;offs++)
	{
		int x,y;

		x = rallyx_radarx[offs] + ((~rallyx_radarattr[offs & 0x0f] & 0x01) << 8);
		y = 253 - rallyx_radary[offs];
		if (flip_screen_get(machine)) x -= 3;

		if (transpen)
			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					((rallyx_radarattr[offs & 0x0f] & 0x0e) >> 1) ^ 0x07,
					0,
					0,0,
					x,y,
					3);
		else
			drawgfx_transtable(bitmap,cliprect,machine->gfx[2],
					((rallyx_radarattr[offs & 0x0f] & 0x0e) >> 1) ^ 0x07,
					0,
					0,0,
					x,y,
					drawmode_table,machine->shadow_table);
	}
}

static void jungler_draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int transpen )
{
	int offs;

	for (offs = spriteram_base; offs < 0x20;offs++)
	{
		int x,y;

		x = rallyx_radarx[offs] + ((~rallyx_radarattr[offs & 0x0f] & 0x08) << 5);
		y = 253 - rallyx_radary[offs];

		if (transpen)
			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					(rallyx_radarattr[offs & 0x0f] & 0x07) ^ 0x07,
					0,
					0,0,
					x,y,
					3);
		else
			drawgfx_transtable(bitmap,cliprect,machine->gfx[2],
					(rallyx_radarattr[offs & 0x0f] & 0x07) ^ 0x07,
					0,
					0,0,
					x,y,
					drawmode_table,machine->shadow_table);
	}
}

static void locomotn_draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int transpen )
{
	int offs;

	for (offs = spriteram_base; offs < 0x20;offs++)
	{
		int x,y;


		/* it looks like in commsega the addresses used are
           a000-a003  a004-a00f
           8020-8023  8034-803f
           8820-8823  8834-883f
           so 8024-8033 and 8824-8833 are not used
        */

		x = rallyx_radarx[offs] + ((~rallyx_radarattr[offs & 0x0f] & 0x08) << 5);
		y = 252 - rallyx_radary[offs];

		if (transpen)
			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					(rallyx_radarattr[offs & 0x0f] & 0x07) ^ 0x07,
					0,
					0,0,
					x,y,
					3);
		else
			drawgfx_transtable(bitmap,cliprect,machine->gfx[2],
					(rallyx_radarattr[offs & 0x0f] & 0x07) ^ 0x07,
					0,
					0,0,
					x,y,
					drawmode_table,machine->shadow_table);
	}
}


VIDEO_UPDATE( rallyx )
{
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

	bitmap_fill(priority_bitmap,cliprect,0);

	tilemap_draw(bitmap,&bg_clip,bg_tilemap,0,0);
	tilemap_draw(bitmap,&fg_clip,fg_tilemap,0,0);
	tilemap_draw(bitmap,&bg_clip,bg_tilemap,1,1);
	tilemap_draw(bitmap,&fg_clip,fg_tilemap,1,1);

	rallyx_draw_bullets(screen->machine,bitmap,cliprect,TRUE);
	rallyx_draw_sprites(screen->machine,bitmap,cliprect,1);
	rallyx_draw_bullets(screen->machine,bitmap,cliprect,FALSE);

	return 0;
}


VIDEO_UPDATE( jungler )
{
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

	bitmap_fill(priority_bitmap,cliprect,0);

	/* tile priority doesn't seem to be supported in Jungler */
	tilemap_draw(bitmap,&bg_clip,bg_tilemap,0,0);
	tilemap_draw(bitmap,&fg_clip,fg_tilemap,0,0);
	tilemap_draw(bitmap,&bg_clip,bg_tilemap,1,0);
	tilemap_draw(bitmap,&fg_clip,fg_tilemap,1,0);

	jungler_draw_bullets(screen->machine,bitmap,cliprect,TRUE);
	rallyx_draw_sprites(screen->machine,bitmap,cliprect,0);
	jungler_draw_bullets(screen->machine,bitmap,cliprect,FALSE);

	if (stars_enable)
		draw_stars(screen->machine, bitmap, cliprect);

	return 0;
}


VIDEO_UPDATE( locomotn )
{
	/* the radar tilemap is just 8x32. We rely on the tilemap code to repeat it across
       the screen, and clip it to only the position where it is supposed to be shown */
	rectangle fg_clip = *cliprect;
	rectangle bg_clip = *cliprect;
	if (flip_screen_get(screen->machine))
	{
		/* handle reduced visible area in some games */
		if (video_screen_get_visible_area(screen)->max_x == 32*8-1)
		{
			bg_clip.min_x = 4*8;
			fg_clip.max_x = 4*8-1;
		}
		else
		{
			bg_clip.min_x = 8*8;
			fg_clip.max_x = 8*8-1;
		}
	}
	else
	{
		bg_clip.max_x = 28*8-1;
		fg_clip.min_x = 28*8;
	}

	bitmap_fill(priority_bitmap,cliprect,0);

	tilemap_draw(bitmap,&bg_clip,bg_tilemap,0,0);
	tilemap_draw(bitmap,&fg_clip,fg_tilemap,0,0);
	tilemap_draw(bitmap,&bg_clip,bg_tilemap,1,1);
	tilemap_draw(bitmap,&fg_clip,fg_tilemap,1,1);

	locomotn_draw_bullets(screen->machine,bitmap,cliprect,TRUE);
	locomotn_draw_sprites(screen->machine,bitmap,cliprect,0);
	locomotn_draw_bullets(screen->machine,bitmap,cliprect,FALSE);

	if (stars_enable)
		draw_stars(screen->machine, bitmap, cliprect);

	return 0;
}

