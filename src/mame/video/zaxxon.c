/***************************************************************************

    Sega Zaxxon hardware

***************************************************************************/

#include "driver.h"
#include "zaxxon.h"
#include "video/resnet.h"

static const UINT8 *color_codes;

static UINT8 bg_enable;
static UINT8 bg_color;
static UINT16 bg_position;
static UINT8 fg_color;

static UINT8 congo_fg_bank;
static UINT8 congo_color_bank;
static UINT8 congo_custom[4];

static tilemap *fg_tilemap;
static tilemap *bg_tilemap;



/*************************************
 *
 *  Palette conversion
 *
 *************************************/

PALETTE_INIT( zaxxon )
{
	static const int resistances[3] = { 1000, 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3,	&resistances[0], rweights, 470, 0,
			3,	&resistances[0], gweights, 470, 0,
			2,	&resistances[1], bweights, 470, 0);

	/* initialize the palette with these colors */
	for (i = 0; i < machine->drv->total_colors; i++)
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

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the character color codes */
	color_codes = &color_prom[256];
}



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	const UINT8 *source = memory_region(REGION_GFX4);
	int size = memory_region_length(REGION_GFX4) / 2;
	int eff_index = tile_index & (size - 1);
	int code = source[eff_index] + 256 * (source[eff_index + size] & 3);
	int color = source[eff_index + size] >> 4;
	SET_TILE_INFO(1, code, color, 0);
}


static TILE_GET_INFO( zaxxon_get_fg_tile_info )
{
	int sx = tile_index % 32;
	int sy = tile_index / 32;
	int code = videoram[tile_index];
	int color = color_codes[sx + 32 * (sy / 4)] & 0x0f;
	SET_TILE_INFO(0, code, color * 2, 0);
}


static TILE_GET_INFO( razmataz_get_fg_tile_info )
{
	int code = videoram[tile_index];
	int color = color_codes[code] & 0x0f;
	SET_TILE_INFO(0, code, color * 2, 0);
}


static TILE_GET_INFO( congo_get_fg_tile_info )
{
	int code = videoram[tile_index] + (congo_fg_bank << 8);
	int color = colorram[tile_index] & 0x1f;
	SET_TILE_INFO(0, code, color * 2, 0);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

static void video_start_common(running_machine *machine, tile_get_info_callback fg_tile_info)
{
	/* reset globals */
	bg_enable = 0;
	bg_color = 0;
	bg_position = 0;
	fg_color = 0;
	congo_fg_bank = 0;
	congo_color_bank = 0;
	memset(congo_custom, 0, sizeof(congo_custom));

	/* create a background and foreground tilemap */
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 32,512);
	fg_tilemap = tilemap_create(fg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 32,32);

	/* configure the foreground tilemap */
	tilemap_set_transparent_pen(fg_tilemap, 0);
	tilemap_set_scrolldx(fg_tilemap, 0, machine->screen[0].width - 256);
	tilemap_set_scrolldy(fg_tilemap, 0, machine->screen[0].height - 256);

	/* register for save states */
	state_save_register_global(bg_enable);
	state_save_register_global(bg_color);
	state_save_register_global(bg_position);
	state_save_register_global(fg_color);
}


VIDEO_START( zaxxon )
{
	video_start_common(machine, zaxxon_get_fg_tile_info);
}


VIDEO_START( razmataz )
{
	video_start_common(machine, razmataz_get_fg_tile_info);
}


VIDEO_START( congo )
{
	/* allocate our own spriteram since it is not accessible by the main CPU */
	spriteram = auto_malloc(0x100);

	/* register for save states */
	state_save_register_global(congo_fg_bank);
	state_save_register_global(congo_color_bank);
	state_save_register_global_array(congo_custom);
	state_save_register_global_pointer(spriteram, 0x100);

	video_start_common(machine, congo_get_fg_tile_info);
}



/*************************************
 *
 *  Video latches and controls
 *
 *************************************/

WRITE8_HANDLER( zaxxon_flipscreen_w )
{
	/* low bit controls flip; background and sprite flip are handled at render time */
	flip_screen = ~data & 1;
	tilemap_set_flip(fg_tilemap, flip_screen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
}


WRITE8_HANDLER( zaxxon_fg_color_w )
{
	/* low bit selects high color palette index */
	fg_color = (data & 1) * 0x80;
	tilemap_set_palette_offset(fg_tilemap, fg_color + (congo_color_bank << 8));
}


WRITE8_HANDLER( zaxxon_bg_position_w )
{
	/* 11 bits of scroll position are stored */
	if (offset == 0)
		bg_position = (bg_position & 0x700) | ((data << 0) & 0x0ff);
	else
		bg_position = (bg_position & 0x0ff) | ((data << 8) & 0x700);
}


WRITE8_HANDLER( zaxxon_bg_color_w )
{
	/* low bit selects high color palette index */
	bg_color = (data & 1) * 0x80;
}


WRITE8_HANDLER( zaxxon_bg_enable_w )
{
	/* low bit enables/disables the background layer */
	bg_enable = data & 1;
}


WRITE8_HANDLER( congo_fg_bank_w )
{
	/* low bit controls the topmost character bit */
	congo_fg_bank = data & 1;
	tilemap_mark_all_tiles_dirty(fg_tilemap);
}


WRITE8_HANDLER( congo_color_bank_w )
{
	/* low bit controls the topmost bit into the color PROM */
	congo_color_bank = data & 1;
	tilemap_set_palette_offset(fg_tilemap, fg_color + (congo_color_bank << 8));
}



/*************************************
 *
 *  Foreground tilemap access
 *
 *************************************/

WRITE8_HANDLER( zaxxon_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}


WRITE8_HANDLER( congo_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}



/*************************************
 *
 *  Congo Bongo custom sprite DMA
 *
 *************************************/

WRITE8_HANDLER( congo_sprite_custom_w )
{
	congo_custom[offset] = data;

	/* seems to trigger on a write of 1 to the 4th byte */
	if (offset == 3 && data == 0x01)
	{
		UINT16 saddr = congo_custom[0] | (congo_custom[1] << 8);
		int count = congo_custom[2];

		/* count cycles (just a guess) */
		activecpu_adjust_icount(-count * 5);

		/* this is just a guess; the chip is hardwired to the spriteram */
		while (count-- >= 0)
		{
			UINT8 daddr = program_read_byte(saddr + 0) * 4;
			spriteram[(daddr + 0) & 0xff] = program_read_byte(saddr + 1);
			spriteram[(daddr + 1) & 0xff] = program_read_byte(saddr + 2);
			spriteram[(daddr + 2) & 0xff] = program_read_byte(saddr + 3);
			spriteram[(daddr + 3) & 0xff] = program_read_byte(saddr + 4);
			saddr += 0x20;
		}
	}
}



/*************************************
 *
 *  Background pixmap drawing
 *
 *************************************/

static void draw_background(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int skew)
{
	/* only draw if enabled */
	if (bg_enable)
	{
		mame_bitmap *pixmap = tilemap_get_pixmap(bg_tilemap);
		int colorbase = bg_color + (congo_color_bank << 8);
		int xmask = pixmap->width - 1;
		int ymask = pixmap->height - 1;
		int flipmask = flip_screen ? 0xff : 0x00;
		int flipoffs = flip_screen ? 0x38 : 0x40;
		int x, y;

		/* the starting X value is offset by 1 pixel (normal) or 7 pixels */
		/* (flipped) due to a delay in the loading */
		if (!flip_screen)
			flipoffs -= 1;
		else
			flipoffs += 7;

		/* loop over visible rows */
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT16 *dst = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
			int srcx, srcy, vf;
			UINT16 *src;

			/* VF = flipped V signals */
			vf = y ^ flipmask;

			/* base of the source row comes from VF plus the scroll value */
			/* this is done by the 3 4-bit adders at U56, U74, U75 */
			srcy = vf + ((bg_position << 1) ^ 0xfff) + 1;
			src = (UINT16 *)pixmap->base + (srcy & ymask) * pixmap->rowpixels;

			/* loop over visible colums */
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				/* start with HF = flipped H signals */
				srcx = x ^ flipmask;
				if (skew)
				{
					/* position within source row is a two-stage addition */
					/* first stage is HF plus half the VF, done by the 2 4-bit */
					/* adders at U53, U54 */
					srcx += ((vf >> 1) ^ 0xff) + 1;

					/* second stage is first stage plus a constant based on the flip */
					/* value is 0x40 for non-flipped, or 0x38 for flipped */
					srcx += flipoffs;
				}

				/* store the pixel, offset by the color offset */
				dst[x] = src[srcx & xmask] + colorbase;
			}
		}
	}

	/* if not enabled, fill the background with black */
	else
		fillbitmap(bitmap, get_black_pen(machine), cliprect);
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

INLINE int find_minimum_y(UINT8 value)
{
	int flipmask = flip_screen ? 0xff : 0x00;
	int flipconst = flip_screen ? 0xef : 0xf1;
	int y;

	/* the sum of the Y position plus a constant based on the flip state */
	/* is added to the current flipped VF; if the top 3 bits are 1, we hit */

	/* first find a 16-pixel bucket where we hit */
	for (y = 0; y < 256; y += 16)
	{
		int sum = (value + flipconst + 1) + (y ^ flipmask);
		if ((sum & 0xe0) == 0xe0)
			break;
	}

	/* then scan backwards until we no longer match */
	while (1)
	{
		int sum = (value + flipconst + 1) + ((y - 1) ^ flipmask);
		if ((sum & 0xe0) != 0xe0)
			break;
		y--;
	}

	/* add one line since we draw sprites on the previous line */
	return (y + 1) & 0xff;
}


INLINE int find_minimum_x(UINT8 value)
{
	int flipmask = flip_screen ? 0xff : 0x00;
	int x;

	/* the sum of the X position plus a constant specifies the address within */
	/* the line bufer; if we're flipped, we will write backwards */
	x = (value + 0xef + 1) ^ flipmask;
	if (flipmask)
		x -= 31;
	return x & 0xff;
}


static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, UINT16 flipxmask, UINT16 flipymask)
{
	int flipmask = flip_screen ? 0xff : 0x00;
	int offs;

	/* only the lower half of sprite RAM is read during rendering */
	for (offs = 0x7c; offs >= 0; offs -= 4)
	{
		int sy = find_minimum_y(spriteram[offs]);
		int flipy = (spriteram[offs + (flipymask >> 8)] ^ flipmask) & flipymask;
		int flipx = (spriteram[offs + (flipxmask >> 8)] ^ flipmask) & flipxmask;
		int code = spriteram[offs + 1];
		int color = (spriteram[offs + 2] & 0x1f) + (congo_color_bank << 5);
		int sx = find_minimum_x(spriteram[offs + 3]);

		/* draw with 256 pixel offsets to ensure we wrap properly */
		drawgfx(bitmap, machine->gfx[2], code, color, flipx, flipy, sx, sy, cliprect, TRANSPARENCY_PEN, 0);
		drawgfx(bitmap, machine->gfx[2], code, color, flipx, flipy, sx, sy - 0x100, cliprect, TRANSPARENCY_PEN, 0);
		drawgfx(bitmap, machine->gfx[2], code, color, flipx, flipy, sx - 0x100, sy, cliprect, TRANSPARENCY_PEN, 0);
		drawgfx(bitmap, machine->gfx[2], code, color, flipx, flipy, sx - 0x100, sy - 0x100, cliprect, TRANSPARENCY_PEN, 0);
	}
}



/*************************************
 *
 *  Core video updates
 *
 *************************************/

VIDEO_UPDATE( zaxxon )
{
	draw_background(machine, bitmap, cliprect, TRUE);
	draw_sprites(machine, bitmap, cliprect, 0x140, 0x180);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}


VIDEO_UPDATE( futspy )
{
	draw_background(machine, bitmap, cliprect, TRUE);
	draw_sprites(machine, bitmap, cliprect, 0x180, 0x180);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}


VIDEO_UPDATE( razmataz )
{
	draw_background(machine, bitmap, cliprect, FALSE);
	draw_sprites(machine, bitmap, cliprect, 0x140, 0x180);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}


VIDEO_UPDATE( congo )
{
	draw_background(machine, bitmap, cliprect, TRUE);
	draw_sprites(machine, bitmap, cliprect, 0x280, 0x180);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
