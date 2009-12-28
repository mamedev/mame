/***************************************************************************

    Midway MCR-III system

***************************************************************************/

#include "driver.h"
#include "includes/mcr.h"



/*************************************
 *
 *  Global variables
 *
 *************************************/

/* Spy Hunter hardware extras */
UINT8 spyhunt_sprite_color_mask;
INT16 spyhunt_scrollx, spyhunt_scrolly;
INT16 spyhunt_scroll_offset;

UINT8 *spyhunt_alpharam;
//size_t spyhunt_alpharam_size;



/*************************************
 *
 *  Local variables
 *
 *************************************/

static tilemap_t *bg_tilemap;
static tilemap_t *alpha_tilemap;



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

#ifdef UNUSED_FUNCTION
static TILE_GET_INFO( get_bg_tile_info )
{
	int data = machine->generic.videoram.u8[tile_index * 2] | (machine->generic.videoram.u8[tile_index * 2 + 1] << 8);
	int code = (data & 0x3ff) | ((data >> 4) & 0x400);
	int color = (data >> 12) & 3;
	SET_TILE_INFO(0, code, color, TILE_FLIPYX((data >> 10) & 3));
}
#endif


static TILE_GET_INFO( mcrmono_get_bg_tile_info )
{
	int data = machine->generic.videoram.u8[tile_index * 2] | (machine->generic.videoram.u8[tile_index * 2 + 1] << 8);
	int code = (data & 0x3ff) | ((data >> 4) & 0x400);
	int color = ((data >> 12) & 3) ^ 3;
	SET_TILE_INFO(0, code, color, TILE_FLIPYX((data >> 10) & 3));
}


static TILEMAP_MAPPER( spyhunt_bg_scan )
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) | ((col & 0x3f) << 4) | ((row & 0x10) << 6);
}


static TILE_GET_INFO( spyhunt_get_bg_tile_info )
{
	int data = machine->generic.videoram.u8[tile_index];
	int code = (data & 0x3f) | ((data >> 1) & 0x40);
	SET_TILE_INFO(0, code, 0, (data & 0x40) ? TILE_FLIPY : 0);
}


static TILE_GET_INFO( spyhunt_get_alpha_tile_info )
{
	SET_TILE_INFO(2, spyhunt_alpharam[tile_index], 0, 0);
}



/*************************************
 *
 *  Spy Hunter-specific palette init
 *
 *************************************/

PALETTE_INIT( spyhunt )
{
	/* alpha colors are hard-coded */
	palette_set_color(machine,4*16+0,MAKE_RGB(0x00,0x00,0x00));
	palette_set_color(machine,4*16+1,MAKE_RGB(0x00,0xff,0x00));
	palette_set_color(machine,4*16+2,MAKE_RGB(0x00,0x00,0xff));
	palette_set_color(machine,4*16+3,MAKE_RGB(0xff,0xff,0xff));
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

#ifdef UNUSED_FUNCTION
VIDEO_START( mcr3 )
{
	/* initialize the background tilemap */
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,  16,16, 32,30);
}
#endif


VIDEO_START( mcrmono )
{
	/* initialize the background tilemap */
	bg_tilemap = tilemap_create(machine, mcrmono_get_bg_tile_info, tilemap_scan_rows,  16,16, 32,30);
}


VIDEO_START( spyhunt )
{
	/* initialize the background tilemap */
	bg_tilemap = tilemap_create(machine, spyhunt_get_bg_tile_info, spyhunt_bg_scan,  64,32, 64,32);

	/* initialize the text tilemap */
	alpha_tilemap = tilemap_create(machine, spyhunt_get_alpha_tile_info, tilemap_scan_cols,  16,16, 32,32);
	tilemap_set_transparent_pen(alpha_tilemap, 0);
	tilemap_set_scrollx(alpha_tilemap, 0, 16);

	state_save_register_global(machine, spyhunt_sprite_color_mask);
	state_save_register_global(machine, spyhunt_scrollx);
	state_save_register_global(machine, spyhunt_scrolly);
	state_save_register_global(machine, spyhunt_scroll_offset);
}



/*************************************
 *
 *  Palette RAM writes
 *
 *************************************/

WRITE8_HANDLER( mcr3_paletteram_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	offset &= 0x7f;

	/* high bit of red comes from low bit of address */
	palette_set_color_rgb(space->machine, offset / 2, pal3bit(((offset & 1) << 2) + (data >> 6)), pal3bit(data >> 0), pal3bit(data >> 3));
}



/*************************************
 *
 *  Video RAM writes
 *
 *************************************/

WRITE8_HANDLER( mcr3_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}


WRITE8_HANDLER( spyhunt_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


WRITE8_HANDLER( spyhunt_alpharam_w )
{
	spyhunt_alpharam[offset] = data;
	tilemap_mark_tile_dirty(alpha_tilemap, offset);
}



/*************************************
 *
 *  Sprite update
 *
 *************************************/

static void mcr3_update_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int color_mask, int code_xor, int dx, int dy)
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs;

	bitmap_fill(machine->priority_bitmap, cliprect, 1);

	/* loop over sprite RAM */
	for (offs = machine->generic.spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int code, color, flipx, flipy, sx, sy, flags;

		/* skip if zero */
		if (spriteram[offs] == 0)
			continue;

/*
    monoboard:
        flags.d0 -> ICG0~ -> PCG0~/PCG2~/PCG4~/PCG6~ -> bit 4 of linebuffer
        flags.d1 -> ICG1~ -> PCG1~/PCG3~/PCG5~/PCG7~ -> bit 5 of linebuffer
        flags.d2 -> IPPR  -> PPR0 /PPR1 /PPR2 /PPR3  -> bit 6 of linebuffer
        flags.d3 -> IRA15 ----------------------------> address line 15 of FG ROMs
        flags.d4 -> HFLIP
        flags.d5 -> VFLIP

*/

		/* extract the bits of information */
		flags = spriteram[offs + 1];
		code = spriteram[offs + 2] + 256 * ((flags >> 3) & 0x01);
		color = ~flags & color_mask;
		flipx = flags & 0x10;
		flipy = flags & 0x20;
		sx = (spriteram[offs + 3] - 3) * 2;
		sy = (241 - spriteram[offs]) * 2;

		code ^= code_xor;

		sx += dx;
		sy += dy;

		/* sprites use color 0 for background pen and 8 for the 'under tile' pen.
            The color 8 is used to cover over other sprites. */
		if (!mcr_cocktail_flip)
		{
			/* first draw the sprite, visible */
			pdrawgfx_transmask(bitmap, cliprect, machine->gfx[1], code, color, flipx, flipy, sx, sy,
					machine->priority_bitmap, 0x00, 0x0101);

			/* then draw the mask, behind the background but obscuring following sprites */
			pdrawgfx_transmask(bitmap, cliprect, machine->gfx[1], code, color, flipx, flipy, sx, sy,
					machine->priority_bitmap, 0x02, 0xfeff);
		}
		else
		{
			/* first draw the sprite, visible */
			pdrawgfx_transmask(bitmap, cliprect, machine->gfx[1], code, color, !flipx, !flipy, 480 - sx, 452 - sy,
					machine->priority_bitmap, 0x00, 0x0101);

			/* then draw the mask, behind the background but obscuring following sprites */
			pdrawgfx_transmask(bitmap, cliprect, machine->gfx[1], code, color, !flipx, !flipy, 480 - sx, 452 - sy,
					machine->priority_bitmap, 0x02, 0xfeff);
		}
	}
}



/*************************************
 *
 *  Generic MCR3 redraw
 *
 *************************************/

VIDEO_UPDATE( mcr3 )
{
	/* update the flip state */
	tilemap_set_flip(bg_tilemap, mcr_cocktail_flip ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

	/* draw the background */
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	/* draw the sprites */
	mcr3_update_sprites(screen->machine, bitmap, cliprect, 0x03, 0, 0, 0);
	return 0;
}


VIDEO_UPDATE( spyhunt )
{
	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	tilemap_set_scrollx(bg_tilemap, 0, spyhunt_scrollx * 2 + spyhunt_scroll_offset);
	tilemap_set_scrolly(bg_tilemap, 0, spyhunt_scrolly * 2);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	/* draw the sprites */
	mcr3_update_sprites(screen->machine, bitmap, cliprect, spyhunt_sprite_color_mask, 0, -12, 0);

	/* render any characters on top */
	tilemap_draw(bitmap, cliprect, alpha_tilemap, 0, 0);
	return 0;
}
