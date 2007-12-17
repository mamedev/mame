/***************************************************************************

    Midway MCR systems

***************************************************************************/

#include "driver.h"
#include "mcr.h"


INT8 mcr12_sprite_xoffs;
INT8 mcr12_sprite_xoffs_flip;

static tilemap *bg_tilemap;


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

/*
    The 90009 board uses 1 byte per tile:

    Byte 0:
        pppppppp = picture index
 */
static TILE_GET_INFO( mcr_90009_get_tile_info )
{
	SET_TILE_INFO(0, videoram[tile_index], 0, 0);

	/* sprite color base is constant 0x10 */
	tileinfo->category = 1;
}


/*
    The 90010 board uses 2 adjacent bytes per tile:

    Byte 0:
        pppppppp = picture index (low 8 bits)

    Byte 1:
        ss------ = sprite palette bank
        ---cc--- = tile palette bank
        -----y-- = Y flip
        ------x- = X flip
        -------p = picture index (high 1 bit)
 */
static TILE_GET_INFO( mcr_90010_get_tile_info )
{
	int data = videoram[tile_index * 2] | (videoram[tile_index * 2 + 1] << 8);
	int code = data & 0x1ff;
	int color = (data >> 11) & 3;
	SET_TILE_INFO(0, code, color, TILE_FLIPYX((data >> 9) & 3));

	/* sprite color base comes from the top 2 bits */
	tileinfo->category = (data >> 14) & 3;
}


/*
    The 91490 board uses 2 adjacent bytes per tile:

    Byte 0:
        pppppppp = picture index (low 8 bits)

    Byte 1:
        ss------ = sprite palette bank (can be disabled via jumpers)
        --cc---- = tile palette bank
        ----y--- = Y flip
        -----x-- = X flip
        ------pp = picture index (high 2 bits)
 */
static TILE_GET_INFO( mcr_91490_get_tile_info )
{
	int data = videoram[tile_index * 2] | (videoram[tile_index * 2 + 1] << 8);
	int code = data & 0x3ff;
	int color = (data >> 12) & 3;
	SET_TILE_INFO(0, code, color, TILE_FLIPYX((data >> 10) & 3));

	/* sprite color base might come from the top 2 bits */
	tileinfo->category = (data >> 14) & 3;
}



/*************************************
 *
 *  Common video startup/shutdown
 *
 *************************************/

VIDEO_START( mcr )
{
	/* the tilemap callback is based on the CPU board */
	switch (mcr_cpu_board)
	{
		case 90009:
			bg_tilemap = tilemap_create(mcr_90009_get_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16,16, 32,30);
			break;

		case 90010:
			bg_tilemap = tilemap_create(mcr_90010_get_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16,16, 32,30);
			break;

		case 91475:
			bg_tilemap = tilemap_create(mcr_90010_get_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16,16, 32,30);
			break;

		case 91490:
			bg_tilemap = tilemap_create(mcr_91490_get_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16,16, 32,30);
			break;

		default:
			assert_always(0, "Unknown mcr board");
			break;
	}
}



/*************************************
 *
 *  Palette RAM writes
 *
 *************************************/

static void mcr_set_color(int index, int data)
{
	palette_set_color_rgb(Machine, index, pal3bit(data >> 6), pal3bit(data >> 0), pal3bit(data >> 3));
}


static void journey_set_color(int index, int data)
{
	/* 3 bits each, RGB */
	int r = (data >> 6) & 7;
	int g = (data >> 0) & 7;
	int b = (data >> 3) & 7;

	/* up to 8 bits */
	r = (r << 5) | (r << 1);
	g = (g << 5) | (g << 1);
	b = (b << 5) | (b << 1);

	/* set the BG color */
	palette_set_color(Machine, index, MAKE_RGB(r, g, b));

	/* if this is an odd entry in the upper palette bank, the hardware */
	/* hard-codes a low 1 bit -- this is used for better grayscales */
	if ((index & 0x31) == 0x31)
	{
		r |= 0x11;
		g |= 0x11;
		b |= 0x11;
	}

	/* set the FG color */
	palette_set_color(Machine, index + 64, MAKE_RGB(r, g, b));
}


WRITE8_HANDLER( mcr_91490_paletteram_w )
{
	paletteram[offset] = data;
	offset &= 0x7f;
	mcr_set_color((offset / 2) & 0x3f, data | ((offset & 1) << 8));
}



/*************************************
 *
 *  Video RAM writes
 *
 *************************************/

WRITE8_HANDLER( mcr_90009_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


WRITE8_HANDLER( mcr_90010_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);

	/* palette RAM is mapped into the upper 0x80 bytes here */
	if ((offset & 0x780) == 0x780)
	{
		if (mcr_cpu_board != 91475)
			mcr_set_color((offset / 2) & 0x3f, data | ((offset & 1) << 8));
		else
			journey_set_color((offset / 2) & 0x3f, data | ((offset & 1) << 8));
	}
}


READ8_HANDLER( twotiger_videoram_r )
{
	/* Two Tigers swizzles the address bits on videoram */
	int effoffs = ((offset << 1) & 0x7fe) | ((offset >> 10) & 1);
	return videoram[effoffs];
}

WRITE8_HANDLER( twotiger_videoram_w )
{
	/* Two Tigers swizzles the address bits on videoram */
	int effoffs = ((offset << 1) & 0x7fe) | ((offset >> 10) & 1);

	videoram[effoffs] = data;
	tilemap_mark_tile_dirty(bg_tilemap, effoffs / 2);

	/* palette RAM is mapped into the upper 0x80 bytes here */
	if ((effoffs & 0x780) == 0x780)
		mcr_set_color(((offset & 0x400) >> 5) | ((offset >> 1) & 0x1f), data | ((offset & 1) << 8));
}


WRITE8_HANDLER( mcr_91490_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}



/*************************************
 *
 *  91399 Video Gen sprite renderer
 *
 *  Paired with:
 *      90009 CPU -> fixed palette @ 1
 *      90010 CPU -> palette specified by tiles
 *
 *************************************/

static void render_sprites_91399(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	const gfx_element *gfx = machine->gfx[1];
	int offs;

	/* render the sprites into the bitmap, ORing together */
	for (offs = 0; offs < spriteram_size; offs += 4)
	{
		int code, x, y, sx, sy, hflip, vflip;

		/* extract the bits of information */
		code = spriteram[offs + 1] & 0x3f;
		hflip = (spriteram[offs + 1] & 0x40) ? 31 : 0;
		vflip = (spriteram[offs + 1] & 0x80) ? 31 : 0;
		sx = (spriteram[offs + 2] - 4) * 2;
		sy = (240 - spriteram[offs]) * 2;

		/* apply cocktail mode */
		if (mcr_cocktail_flip)
		{
			hflip ^= 31;
			vflip ^= 31;
			sx = 466 - sx + mcr12_sprite_xoffs_flip;
			sy = 450 - sy;
		}
		else
			sx += mcr12_sprite_xoffs;

		/* clamp within 512 */
		sx &= 0x1ff;
		sy &= 0x1ff;

		/* loop over lines in the sprite */
		for (y = 0; y < 32; y++, sy = (sy + 1) & 0x1ff)
			if (sy >= cliprect->min_y && sy <= cliprect->max_y)
			{
				UINT8 *src = gfx->gfxdata + gfx->char_modulo * code + gfx->line_modulo * (y ^ vflip);
				UINT16 *dst = BITMAP_ADDR16(bitmap, sy, 0);
				UINT8 *pri = BITMAP_ADDR8(priority_bitmap, sy, 0);

				/* loop over columns */
				for (x = 0; x < 32; x++)
				{
					int tx = (sx + x) & 0x1ff;
					int pix = pri[tx] | src[x ^ hflip];

					/* update the effective sprite pixel */
					pri[tx] = pix;

					/* only draw if the low 3 bits are set */
					if (pix & 0x07)
						dst[tx] = pix;
				}
			}
	}
}



/*************************************
 *
 *  91464 Super Video Gen sprite renderer
 *
 *  Paired with:
 *      91442 CPU -> fixed palette @ 1 (upper half) or 3 (lower half)
 *      91475 CPU -> palette specified by sprite board; sprites have extra implicit colors
 *      91490 CPU -> palette specified by sprite board or by tiles (select via jumpers)
 *      91721 CPU -> palette specified by sprite board
 *
 *************************************/

static void render_sprites_91464(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int primask, int sprmask, int colormask)
{
	const gfx_element *gfx = machine->gfx[1];
	int offs;

	/* render the sprites into the bitmap, working from topmost to bottommost */
	for (offs = spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int code, color, x, y, sx, sy, hflip, vflip;

		/* extract the bits of information */
		code = (spriteram[offs + 2] + 256 * ((spriteram[offs + 1] >> 3) & 0x01)) % gfx->total_elements;
		color = (((~spriteram[offs + 1] & 3) << 4) & sprmask) | colormask;
		hflip = (spriteram[offs + 1] & 0x10) ? 31 : 0;
		vflip = (spriteram[offs + 1] & 0x20) ? 31 : 0;
		sx = (spriteram[offs + 3] - 3) * 2;
		sy = (241 - spriteram[offs]) * 2;

		/* apply cocktail mode */
		if (mcr_cocktail_flip)
		{
			hflip ^= 31;
			vflip ^= 31;
			sx = 480 - sx;
			sy = 452 - sy;
		}

		/* clamp within 512 */
		sx &= 0x1ff;
		sy &= 0x1ff;

		/* loop over lines in the sprite */
		for (y = 0; y < 32; y++, sy = (sy + 1) & 0x1ff)
			if (sy >= 2 && sy >= cliprect->min_y && sy <= cliprect->max_y)
			{
				UINT8 *src = gfx->gfxdata + gfx->char_modulo * code + gfx->line_modulo * (y ^ vflip);
				UINT16 *dst = BITMAP_ADDR16(bitmap, sy, 0);
				UINT8 *pri = BITMAP_ADDR8(priority_bitmap, sy, 0);

				/* loop over columns */
				for (x = 0; x < 32; x++)
				{
					int tx = (sx + x) & 0x1ff;
					int pix = pri[tx];
					if (pix != 0xff)
					{
						/* compute the final value */
						pix = (pix & primask) | color | src[x ^ hflip];

						/* if non-zero, draw */
						if (pix & 0x0f)
						{
							/* mark this pixel so we don't draw there again */
							pri[tx] = 0xff;

							/* only draw if the low 3 bits are set */
							if (pix & 0x07)
								dst[tx] = pix;
						}
					}
				}
			}
	}
}



/*************************************
 *
 *  Main refresh routines
 *
 *************************************/

VIDEO_UPDATE( mcr )
{
	/* update the flip state */
	tilemap_set_flip(bg_tilemap, mcr_cocktail_flip ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

	/* draw the background */
	fillbitmap(priority_bitmap, 0, cliprect);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0x00);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 1, 0x10);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 2, 0x20);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 3, 0x30);

	/* update the sprites and render them */
	switch (mcr_sprite_board)
	{
		case 91399:
			render_sprites_91399(machine, bitmap, cliprect);
			break;

		case 91464:
			if (mcr_cpu_board == 91442)
				render_sprites_91464(machine, bitmap, cliprect, 0x00, 0x30, 0x00);
			else if (mcr_cpu_board == 91475)
				render_sprites_91464(machine, bitmap, cliprect, 0x00, 0x30, 0x40);
			else if (mcr_cpu_board == 91490)
				render_sprites_91464(machine, bitmap, cliprect, 0x00, 0x30, 0x00);
			else if (mcr_cpu_board == 91721)
				render_sprites_91464(machine, bitmap, cliprect, 0x00, 0x30, 0x00);
			break;
	}
	return 0;
}
