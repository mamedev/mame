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
drawn over high priority tiles), then drawing the pullets again in
TRANSPARENCY_PEN_TABLE mode, so that bullets not covered by sprites remain
the same while the others alter the sprite color.


The tile/sprite priority is controlled by the top bit of the tile color code.
This feature seems to be disabled in Jungler, probably because that game
needs more color combination to render its graphics.

***************************************************************************/

#include "driver.h"



UINT8 *rallyx_videoram,*rallyx_radarattr;

static UINT8 *rallyx_radarx,*rallyx_radary;
static int video_type, spriteram_base;

static tilemap *bg_tilemap,*fg_tilemap;

#define MAX_STARS 1000
#define STARS_COLOR_BASE 32

static int stars_enable;

struct star
{
	int x,y,color;
};
static struct star stars[MAX_STARS];
static int total_stars;


enum
{
	TYPE_RALLYX,
	TYPE_JUNGLER,
	TYPE_TACTCIAN,
	TYPE_LOCOMOTN,
	TYPE_COMMSEGA
};


DRIVER_INIT( rallyx )
{
	video_type = TYPE_RALLYX;
}

DRIVER_INIT( jungler )
{
	video_type = TYPE_JUNGLER;
}

DRIVER_INIT( tactcian )
{
	video_type = TYPE_TACTCIAN;
}

DRIVER_INIT( locomotn )
{
	video_type = TYPE_LOCOMOTN;
}

DRIVER_INIT( commsega )
{
	video_type = TYPE_COMMSEGA;
}



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
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < 32;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		if (video_type == TYPE_RALLYX)
		{
			bit0 = 0;
			bit1 = (*color_prom >> 6) & 0x01;
			bit2 = (*color_prom >> 7) & 0x01;
			b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		}
		else
		{
			bit0 = (*color_prom >> 6) & 0x01;
			bit1 = (*color_prom >> 7) & 0x01;
			b = 0x50 * bit0 + 0xab * bit1;
		}

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		color_prom++;
	}

	/* color_prom now points to the beginning of the lookup table */

	/* character lookup table */
	/* sprites use the same color lookup table as characters */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = *(color_prom++) & 0x0f;

	/* radar dots lookup table */
	/* they use colors 16-19 */
	for (i = 0;i < 4;i++)
		COLOR(2,i) = 16 + i;

	/* Rally X doesn't have the optional starfield generator */
	if (video_type != TYPE_RALLYX)
	{
		/* now the stars */
		for (i = 0;i < 64;i++)
		{
			int bits,r,g,b;
			static const int map[4] = { 0x00, 0x47, 0x97, 0xde };

			bits = (i >> 0) & 0x03;
			r = map[bits];
			bits = (i >> 2) & 0x03;
			g = map[bits];
			bits = (i >> 4) & 0x03;
			b = map[bits];

			palette_set_color(machine,i + 32,MAKE_RGB(r,g,b));
		}
	}
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

VIDEO_START( rallyx )
{
	int i;

	if (video_type == TYPE_RALLYX || video_type == TYPE_JUNGLER)
	{
		bg_tilemap = tilemap_create(rallyx_bg_get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
		fg_tilemap = tilemap_create(rallyx_fg_get_tile_info,fg_tilemap_scan,  TILEMAP_TYPE_PEN,8,8, 8,32);
	}
	else
	{
		bg_tilemap = tilemap_create(locomotn_bg_get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
		fg_tilemap = tilemap_create(locomotn_fg_get_tile_info,fg_tilemap_scan,  TILEMAP_TYPE_PEN,8,8, 8,32);
	}

	/* the scrolling tilemap is slightly misplaced in Rally X */
	if (video_type == TYPE_RALLYX)
		tilemap_set_scrolldx(bg_tilemap,3,3);

	/* commsega has more sprites and bullets than the other games */
	if (video_type == TYPE_COMMSEGA)
		spriteram_base = 0x00;
	else
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
		gfx_drawmode_table[i] = DRAWMODE_SHADOW;
	gfx_drawmode_table[3] = DRAWMODE_NONE;


	/* Rally X doesn't have the optional starfield generator */
	if (video_type != TYPE_RALLYX)
	{
		int generator;
		int x,y;

		/* precalculate the star background */
		/* this comes from the Galaxian hardware, Bosconian is probably different */
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
						stars[total_stars].color = machine->pens[color + STARS_COLOR_BASE];

						total_stars++;
					}
				}
			}
		}
	}
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



static void plot_star(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int x, int y, int color)
{
	int bpen = machine->pens[0];

	if (y < cliprect->min_y ||
		y > cliprect->max_y ||
		x < cliprect->min_x ||
		x > cliprect->max_x)
		return;

	if (flip_screen_x)
	{
		x = 255 - x;
	}
	if (flip_screen_y)
	{
		y = 255 - y;
	}

	if (*BITMAP_ADDR16(bitmap, y, x) == bpen)
		*BITMAP_ADDR16(bitmap, y, x) = machine->pens[STARS_COLOR_BASE + color];
}

static void draw_stars(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = 0;offs < total_stars;offs++)
	{
		int x,y;


		x = stars[offs].x;
		y = stars[offs].y;

		if ((y & 0x01) ^ ((x >> 3) & 0x01))
		{
			plot_star(machine, bitmap, cliprect, x, y, stars[offs].color);
		}
	}
}


static void rallyx_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int displacement )
{
	int offs;

	for (offs = 0x20-2;offs >= spriteram_base;offs -= 2)
	{
		int sx = spriteram[offs + 1] + ((spriteram_2[offs + 1] & 0x80) << 1) - displacement;
		int sy = 241 - spriteram_2[offs] - displacement;
		int flipx = spriteram[offs] & 1;
		int flipy = spriteram[offs] & 2;
		if (flip_screen) sx -= 2*displacement;

		pdrawgfx(bitmap,machine->gfx[1],
				(spriteram[offs] & 0xfc) >> 2,
				spriteram_2[offs + 1] & 0x3f,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_COLOR,0,0x02);
	}
}

static void locomotn_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int displacement )
{
	int offs;

	for (offs = 0x20-2;offs >= spriteram_base;offs -= 2)
	{
		int sx = spriteram[offs + 1] + ((spriteram_2[offs + 1] & 0x80) << 1);
		int sy = 241 - spriteram_2[offs] - displacement;
		int flip = spriteram[offs] & 2;

		/* handle reduced visible area in some games */
		if (flip_screen && machine->screen[0].visarea.max_x == 32*8-1) sx += 32;

		pdrawgfx(bitmap,machine->gfx[1],
				((spriteram[offs] & 0x7c) >> 2) + 0x20*(spriteram[offs] & 0x01) + ((spriteram[offs] & 0x80) >> 1),
				spriteram_2[offs + 1] & 0x3f,
				flip,flip,
				sx,sy,
				cliprect,TRANSPARENCY_COLOR,0,0x02);
	}
}

static void rallyx_draw_bullets(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int transparency )
{
	int offs;

	for (offs = spriteram_base; offs < 0x20;offs++)
	{
		int x,y;

		x = rallyx_radarx[offs] + ((~rallyx_radarattr[offs & 0x0f] & 0x01) << 8);
		y = 253 - rallyx_radary[offs];
		if (flip_screen) x -= 3;

		drawgfx(bitmap,machine->gfx[2],
				((rallyx_radarattr[offs & 0x0f] & 0x0e) >> 1) ^ 0x07,
				0,
				0,0,
				x,y,
				cliprect,transparency,3);
	}
}

static void jungler_draw_bullets(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int transparency )
{
	int offs;

	for (offs = spriteram_base; offs < 0x20;offs++)
	{
		int x,y;

		x = rallyx_radarx[offs] + ((~rallyx_radarattr[offs & 0x0f] & 0x08) << 5);
		y = 253 - rallyx_radary[offs];

		drawgfx(bitmap,machine->gfx[2],
				(rallyx_radarattr[offs & 0x0f] & 0x07) ^ 0x07,
				0,
				0,0,
				x,y,
				cliprect,transparency,3);
	}
}

static void locomotn_draw_bullets(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int transparency )
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

		/* handle reduced visible area in some games */
		if (flip_screen && machine->screen[0].visarea.max_x == 32*8-1) x += 32;

		drawgfx(bitmap,machine->gfx[2],
				(rallyx_radarattr[offs & 0x0f] & 0x07) ^ 0x07,
				0,
				0,0,
				x,y,
				cliprect,transparency,3);
	}
}


VIDEO_UPDATE( rallyx )
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

	fillbitmap(priority_bitmap,0,cliprect);

	tilemap_draw(bitmap,&bg_clip,bg_tilemap,0,0);
	tilemap_draw(bitmap,&fg_clip,fg_tilemap,0,0);
	/* tile priority doesn't seem to be supported in Jungler */
	tilemap_draw(bitmap,&bg_clip,bg_tilemap,1,video_type == TYPE_JUNGLER ? 0 : 1);
	tilemap_draw(bitmap,&fg_clip,fg_tilemap,1,video_type == TYPE_JUNGLER ? 0 : 1);

	switch (video_type)
	{
		case TYPE_RALLYX:
			rallyx_draw_bullets(machine, bitmap,cliprect,TRANSPARENCY_PEN);
			rallyx_draw_sprites(machine, bitmap,cliprect,1);
			rallyx_draw_bullets(machine, bitmap,cliprect,TRANSPARENCY_PEN_TABLE);
			break;

		case TYPE_JUNGLER:
			jungler_draw_bullets(machine, bitmap,cliprect,TRANSPARENCY_PEN);
			rallyx_draw_sprites(machine, bitmap,cliprect,0);
			jungler_draw_bullets(machine, bitmap,cliprect,TRANSPARENCY_PEN_TABLE);
			break;

		case TYPE_TACTCIAN:
		case TYPE_LOCOMOTN:
		case TYPE_COMMSEGA:
			locomotn_draw_bullets(machine, bitmap,cliprect,TRANSPARENCY_PEN);
			locomotn_draw_sprites(machine, bitmap,cliprect,0);
			locomotn_draw_bullets(machine, bitmap,cliprect,TRANSPARENCY_PEN_TABLE);
			break;
	}

	/* Rally X doesn't have the optional starfield generator */
	if (video_type != TYPE_RALLYX)
		if (stars_enable) draw_stars(machine, bitmap,cliprect);
	return 0;
}
