/*************************************************************************

    Atari Football hardware

*************************************************************************/

#include "driver.h"
#include "includes/atarifb.h"


/* local */
UINT8 *atarifb_alphap1_videoram;
UINT8 *atarifb_alphap2_videoram;
UINT8 *atarifb_field_videoram;
UINT8 *atarifb_scroll_register;

static tilemap *alpha1_tilemap;
static tilemap *alpha2_tilemap;
static tilemap *field_tilemap;



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static void get_tile_info_common(running_machine *machine, tile_data *tileinfo,
								 tilemap_memory_index tile_index, UINT8 *alpha_videoram)
{
	int code = alpha_videoram[tile_index] & 0x3f;
	int flip = alpha_videoram[tile_index] & 0x40;
	int disable = alpha_videoram[tile_index] & 0x80;

	if (disable)
		code = 0;	/* I *know* this is a space */

	SET_TILE_INFO(0, code, 0, (flip ? TILE_FLIPX | TILE_FLIPY : 0));
}


static TILE_GET_INFO( alpha1_get_tile_info )
{
	get_tile_info_common(machine, tileinfo, tile_index, atarifb_alphap1_videoram);
}


static TILE_GET_INFO( alpha2_get_tile_info )
{
	get_tile_info_common(machine, tileinfo, tile_index, atarifb_alphap2_videoram);
}


static TILE_GET_INFO( field_get_tile_info )
{
	int code = atarifb_field_videoram[tile_index] & 0x3f;
	int flipyx = atarifb_field_videoram[tile_index] >> 6;

	SET_TILE_INFO(1, code, 0, TILE_FLIPYX(flipyx));
}




/*************************************
 *
 *  Video RAM writes
 *
 *************************************/

WRITE8_HANDLER( atarifb_alpha1_videoram_w )
{
	atarifb_alphap1_videoram[offset] = data;
	tilemap_mark_tile_dirty(alpha1_tilemap, offset);
}


WRITE8_HANDLER( atarifb_alpha2_videoram_w )
{
	atarifb_alphap2_videoram[offset] = data;
	tilemap_mark_tile_dirty(alpha2_tilemap, offset);
}


WRITE8_HANDLER( atarifb_field_videoram_w )
{
	atarifb_field_videoram[offset] = data;
	tilemap_mark_tile_dirty(field_tilemap, offset);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( atarifb )
{
	alpha1_tilemap = tilemap_create(machine, alpha1_get_tile_info, tilemap_scan_cols,  8,8,  3,32);
	alpha2_tilemap = tilemap_create(machine, alpha2_get_tile_info, tilemap_scan_cols,  8,8,  3,32);
	field_tilemap  = tilemap_create(machine, field_get_tile_info,  tilemap_scan_rows,  8,8, 32,32);
}



static void draw_playfield_and_alpha(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,
									 int playfield_x_offset, int playfield_y_offset)
{
	static const rectangle bigfield_area = {  4*8, 34*8-1, 0*8, 32*8-1 };

	int scroll_x[1];
	int scroll_y[1];

	scroll_x[0] = - *atarifb_scroll_register + 32 + playfield_x_offset;
	scroll_y[0] = 8 + playfield_y_offset;

	copybitmap(bitmap, tilemap_get_pixmap(alpha1_tilemap), 0, 0, 35*8, 1*8, NULL);
	copybitmap(bitmap, tilemap_get_pixmap(alpha2_tilemap), 0, 0,  0*8, 1*8, NULL);
	copyscrollbitmap(bitmap, tilemap_get_pixmap(field_tilemap),  1, scroll_x, 1, scroll_y, &bigfield_area);
}


static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,
						 int gfx, int is_soccer)
{
	static const rectangle bigfield_area = {  4*8, 34*8-1, 0*8, 32*8-1 };

	int obj;

	for (obj=0;obj<16;obj++)
	{
		int charcode;
		int flipx,flipy;
		int sx,sy;
		int shade = 0;

		sy = 255 - spriteram[obj*2 + 1];
		if (sy == 255) continue;

		charcode = spriteram[obj*2] & 0x3f;
		flipx = (spriteram[obj*2] & 0x40);
		flipy = (spriteram[obj*2] & 0x80);
		sx = spriteram[obj*2 + 0x20] + 8*3;

		/* Note on Atari Soccer: */
		/* There are 3 sets of 2 bits each, where the 2 bits represent */
		/* black, dk grey, grey and white. I think the 3 sets determine the */
		/* color of each bit in the sprite, but I haven't implemented it that way. */
		if (is_soccer)
		{
			shade = ((spriteram[obj*2+1 + 0x20]) & 0x07);

			drawgfx_transpen(bitmap,&bigfield_area,machine->gfx[gfx + 1],
				charcode, shade,
				flipx,flipy,sx,sy,0);

			shade = ((spriteram[obj*2+1 + 0x20]) & 0x08) >> 3;
		}

		drawgfx_transpen(bitmap,&bigfield_area,machine->gfx[gfx],
				charcode, shade,
				flipx,flipy,sx,sy,0);

		/* If this isn't soccer, handle the multiplexed sprites */
		if (!is_soccer)
		{
			/* The down markers are multiplexed by altering the y location during */
			/* mid-screen. We'll fake it by essentially doing the same thing here. */
			if ((charcode == 0x11) && (sy == 0x07))
			{
				sy = 0xf1; /* When multiplexed, it's 0x10...why? */
				drawgfx_transpen(bitmap,&bigfield_area,machine->gfx[gfx],
					charcode, 0,
					flipx,flipy,sx,sy,0);
			}
		}
	}
}


VIDEO_UPDATE( atarifb )
{
	draw_playfield_and_alpha(screen->machine, bitmap, cliprect, 0, 0);

	draw_sprites(screen->machine, bitmap, cliprect, 1, 0);

	return 0;
}


VIDEO_UPDATE( abaseb )
{
	draw_playfield_and_alpha(screen->machine, bitmap, cliprect, -8, 0);

	draw_sprites(screen->machine, bitmap, cliprect, 1, 0);

	return 0;
}


VIDEO_UPDATE( soccer )
{
	draw_playfield_and_alpha(screen->machine, bitmap, cliprect, 0, 8);

	draw_sprites(screen->machine, bitmap, cliprect, 2, 1);

	return 0;
}
