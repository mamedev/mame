/***************************************************************************

    Midway MCR-68k system

***************************************************************************/

#include "emu.h"
#include "includes/mcr.h"


#define LOW_BYTE(x) ((x) & 0xff)


UINT8 mcr68_sprite_clip;
INT8 mcr68_sprite_xoffset;

static tilemap_t *bg_tilemap;
static tilemap_t *fg_tilemap;



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int data = LOW_BYTE(machine->generic.videoram.u16[tile_index * 2]) | (LOW_BYTE(machine->generic.videoram.u16[tile_index * 2 + 1]) << 8);
	int code = (data & 0x3ff) | ((data >> 4) & 0xc00);
	int color = (~data >> 12) & 3;
	SET_TILE_INFO(0, code, color, TILE_FLIPYX((data >> 10) & 3));
	if (machine->gfx[0]->total_elements < 0x1000)
		tileinfo->category = (data >> 15) & 1;
}


static TILE_GET_INFO( zwackery_get_bg_tile_info )
{
	int data = machine->generic.videoram.u16[tile_index];
	int color = (data >> 13) & 7;
	SET_TILE_INFO(0, data & 0x3ff, color, TILE_FLIPYX((data >> 11) & 3));
}


static TILE_GET_INFO( zwackery_get_fg_tile_info )
{
	int data = machine->generic.videoram.u16[tile_index];
	int color = (data >> 13) & 7;
	SET_TILE_INFO(2, data & 0x3ff, color, TILE_FLIPYX((data >> 11) & 3));
	tileinfo->category = (color != 0);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( mcr68 )
{
	/* initialize the background tilemap */
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,  16,16, 32,32);
	tilemap_set_transparent_pen(bg_tilemap, 0);
}


VIDEO_START( zwackery )
{
	const UINT8 *colordatabase = (const UINT8 *)memory_region(machine, "gfx3");
	gfx_element *gfx0 = machine->gfx[0];
	gfx_element *gfx2 = machine->gfx[2];
	UINT8 *srcdata0, *dest0;
	UINT8 *srcdata2, *dest2;
	int code, y, x;

	/* initialize the background tilemap */
	bg_tilemap = tilemap_create(machine, zwackery_get_bg_tile_info, tilemap_scan_rows,  16,16, 32,32);

	/* initialize the foreground tilemap */
	fg_tilemap = tilemap_create(machine, zwackery_get_fg_tile_info, tilemap_scan_rows,  16,16, 32,32);
	tilemap_set_transparent_pen(fg_tilemap, 0);

	/* allocate memory for the assembled gfx data */
	srcdata0 = auto_alloc_array(machine, UINT8, gfx0->total_elements * gfx0->width * gfx0->height);
	srcdata2 = auto_alloc_array(machine, UINT8, gfx2->total_elements * gfx2->width * gfx2->height);

	/* "colorize" each code */
	dest0 = srcdata0;
	dest2 = srcdata2;
	for (code = 0; code < gfx0->total_elements; code++)
	{
		const UINT8 *coldata = colordatabase + code * 32;
		const UINT8 *gfxdata0 = gfx_element_get_data(gfx0, code);
		const UINT8 *gfxdata2 = gfx_element_get_data(gfx2, code);

		/* assume 16 rows */
		for (y = 0; y < 16; y++)
		{
			const UINT8 *gd0 = gfxdata0;
			const UINT8 *gd2 = gfxdata2;

			/* 16 columns */
			for (x = 0; x < 16; x++, gd0++, gd2++)
			{
				int coloffs = (y & 0x0c) | ((x >> 2) & 0x03);
				int pen0 = coldata[coloffs * 2 + 0];
				int pen1 = coldata[coloffs * 2 + 1];
				int tp0, tp1;

				/* every 4 pixels gets its own foreground/background colors */
				*dest0++ = *gd0 ? pen1 : pen0;

				/* for gfx 2, we convert all low-priority pens to 0 */
				tp0 = (pen0 & 0x80) ? pen0 : 0;
				tp1 = (pen1 & 0x80) ? pen1 : 0;
				*dest2++ = *gd2 ? tp1 : tp0;
			}

			/* advance */
			gfxdata0 += gfx0->line_modulo;
			gfxdata2 += gfx2->line_modulo;
		}
	}

	/* create a simple target layout */
	gfx0->layout.planes = gfx2->layout.planes = 8;
	for (x = 0; x < 8; x++)
		gfx0->layout.planeoffset[x] = gfx2->layout.planeoffset[x] = x;
	for (x = 0; x < gfx0->width; x++)
		gfx0->layout.xoffset[x] = gfx2->layout.xoffset[x] = 8 * x;
	for (y = 0; y < gfx0->height; y++)
		gfx0->layout.yoffset[y] = gfx2->layout.yoffset[y] = 8 * y * gfx0->width;
	gfx0->layout.charincrement = gfx2->layout.charincrement = 8 * gfx0->width * gfx0->height;

	/* make the assembled data our new source data */
	gfx_element_set_source(gfx0, srcdata0);
	gfx_element_set_source(gfx2, srcdata2);
}



/*************************************
 *
 *  Palette RAM writes
 *
 *************************************/

WRITE16_HANDLER( mcr68_paletteram_w )
{
	int newword;

	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	newword = space->machine->generic.paletteram.u16[offset];
	palette_set_color_rgb(space->machine, offset, pal3bit(newword >> 6), pal3bit(newword >> 0), pal3bit(newword >> 3));
}


WRITE16_HANDLER( zwackery_paletteram_w )
{
	int newword;

	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	newword = space->machine->generic.paletteram.u16[offset];
	palette_set_color_rgb(space->machine, offset, pal5bit(~newword >> 10), pal5bit(~newword >> 0), pal5bit(~newword >> 5));
}



/*************************************
 *
 *  Video RAM writes
 *
 *************************************/

WRITE16_HANDLER( mcr68_videoram_w )
{
	COMBINE_DATA(&space->machine->generic.videoram.u16[offset]);
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}


WRITE16_HANDLER( zwackery_videoram_w )
{
	COMBINE_DATA(&space->machine->generic.videoram.u16[offset]);
	tilemap_mark_tile_dirty(bg_tilemap, offset);
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}


WRITE16_HANDLER( zwackery_spriteram_w )
{
	/* yech -- Zwackery relies on the upper 8 bits of a spriteram read being $ff! */
	/* to make this happen we always write $ff in the upper 8 bits */
	COMBINE_DATA(&space->machine->generic.spriteram.u16[offset]);
	space->machine->generic.spriteram.u16[offset] |= 0xff00;
}



/*************************************
 *
 *  Sprite update
 *
 *************************************/

static void mcr68_update_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority)
{
	rectangle sprite_clip = machine->primary_screen->visible_area();
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int offs;

	/* adjust for clipping */
	sprite_clip.min_x += mcr68_sprite_clip;
	sprite_clip.max_x -= mcr68_sprite_clip;
	sect_rect(&sprite_clip, cliprect);

	bitmap_fill(machine->priority_bitmap,&sprite_clip,1);

	/* loop over sprite RAM */
	for (offs = machine->generic.spriteram_size / 2 - 4;offs >= 0;offs -= 4)
	{
		int code, color, flipx, flipy, x, y, flags;

		flags = LOW_BYTE(spriteram16[offs + 1]);
		code = LOW_BYTE(spriteram16[offs + 2]) + 256 * ((flags >> 3) & 0x01) + 512 * ((flags >> 6) & 0x03);

		/* skip if zero */
		if (code == 0)
			continue;

		/* also skip if this isn't the priority we're drawing right now */
		if (((flags >> 2) & 1) != priority)
			continue;

		/* extract the bits of information */
		color = ~flags & 0x03;
		flipx = flags & 0x10;
		flipy = flags & 0x20;
		x = LOW_BYTE(spriteram16[offs + 3]) * 2 + mcr68_sprite_xoffset;
		y = (241 - LOW_BYTE(spriteram16[offs])) * 2;

		/* allow sprites to clip off the left side */
		if (x > 0x1f0) x -= 0x200;

		/* sprites use color 0 for background pen and 8 for the 'under tile' pen.
            The color 8 is used to cover over other sprites. */

		/* first draw the sprite, visible */
		pdrawgfx_transmask(bitmap, &sprite_clip, machine->gfx[1], code, color, flipx, flipy, x, y,
				machine->priority_bitmap, 0x00, 0x0101);

		/* then draw the mask, behind the background but obscuring following sprites */
		pdrawgfx_transmask(bitmap, &sprite_clip, machine->gfx[1], code, color, flipx, flipy, x, y,
				machine->priority_bitmap, 0x02, 0xfeff);
	}
}


static void zwackery_update_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority)
{
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int offs;

	bitmap_fill(machine->priority_bitmap,cliprect,1);

	/* loop over sprite RAM */
	for (offs = machine->generic.spriteram_size / 2 - 4;offs >= 0;offs -= 4)
	{
		int code, color, flipx, flipy, x, y, flags;

		/* get the code and skip if zero */
		code = LOW_BYTE(spriteram16[offs + 2]);
		if (code == 0)
			continue;

		/* extract the flag bits and determine the color */
		flags = LOW_BYTE(spriteram16[offs + 1]);
		color = ((~flags >> 2) & 0x0f) | ((flags & 0x02) << 3);

		/* for low priority, draw everything but color 7 */
		if (!priority)
		{
			if (color == 7)
				continue;
		}

		/* for high priority, only draw color 7 */
		else
		{
			if (color != 7)
				continue;
		}

		/* determine flipping and coordinates */
		flipx = ~flags & 0x40;
		flipy = flags & 0x80;
		x = (231 - LOW_BYTE(spriteram16[offs + 3])) * 2;
		y = (241 - LOW_BYTE(spriteram16[offs])) * 2;

		if (x <= -32) x += 512;

		/* sprites use color 0 for background pen and 8 for the 'under tile' pen.
            The color 8 is used to cover over other sprites. */

		/* first draw the sprite, visible */
		pdrawgfx_transmask(bitmap, cliprect, machine->gfx[1], code, color, flipx, flipy, x, y,
				machine->priority_bitmap, 0x00, 0x0101);

		/* then draw the mask, behind the background but obscuring following sprites */
		pdrawgfx_transmask(bitmap, cliprect, machine->gfx[1], code, color, flipx, flipy, x, y,
				machine->priority_bitmap, 0x02, 0xfeff);
	}
}



/*************************************
 *
 *  General MCR/68k update
 *
 *************************************/

VIDEO_UPDATE( mcr68 )
{
	/* draw the background */
	tilemap_draw(bitmap, cliprect, bg_tilemap, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);

	/* draw the low-priority sprites */
	mcr68_update_sprites(screen->machine, bitmap, cliprect, 0);

    /* redraw tiles with priority over sprites */
	tilemap_draw(bitmap, cliprect, bg_tilemap, 1, 0);

	/* draw the high-priority sprites */
	mcr68_update_sprites(screen->machine, bitmap, cliprect, 1);
	return 0;
}


VIDEO_UPDATE( zwackery )
{
	/* draw the background */
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	/* draw the low-priority sprites */
	zwackery_update_sprites(screen->machine, bitmap, cliprect, 0);

    /* redraw tiles with priority over sprites */
	tilemap_draw(bitmap, cliprect, fg_tilemap, 1, 0);

	/* draw the high-priority sprites */
	zwackery_update_sprites(screen->machine, bitmap, cliprect, 1);
	return 0;
}
