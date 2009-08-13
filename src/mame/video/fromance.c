/******************************************************************************

    Video Hardware for Video System Mahjong series and Pipe Dream.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -
    and Bryan McPhail, Nicola Salmoria, Aaron Giles

******************************************************************************/

#include "driver.h"
#include "fromance.h"


static UINT8 selected_videoram;
static UINT8 *local_videoram[2];

static UINT8 selected_paletteram;
static UINT8 *local_paletteram;

static UINT32 scrollx[2], scrolly[2];
static UINT8 gfxreg;
static UINT8 flipscreen;
static UINT32 scrolly_ofs;
static UINT32 scrollx_ofs;

static UINT8 crtc_register;
static UINT8 crtc_data[0x10];
static emu_timer *crtc_timer;

static UINT8 flipscreen_old;

static tilemap *bg_tilemap, *fg_tilemap;

static TIMER_CALLBACK( crtc_interrupt_gen );


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

INLINE void get_fromance_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int layer)
{
	int tile = ((local_videoram[layer][0x0000 + tile_index] & 0x80) << 9) |
				(local_videoram[layer][0x1000 + tile_index] << 8) |
				local_videoram[layer][0x2000 + tile_index];
	int color = local_videoram[layer][tile_index] & 0x7f;

	SET_TILE_INFO(layer, tile, color, 0);
}

static TILE_GET_INFO( get_fromance_bg_tile_info ) { get_fromance_tile_info(machine,tileinfo,tile_index, 0); }
static TILE_GET_INFO( get_fromance_fg_tile_info ) { get_fromance_tile_info(machine,tileinfo,tile_index, 1); }

INLINE void get_nekkyoku_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int layer)
{
	int tile = (local_videoram[layer][0x0000 + tile_index] << 8) |
				local_videoram[layer][0x1000 + tile_index];
	int color = local_videoram[layer][tile_index + 0x2000] & 0x3f;

	SET_TILE_INFO(layer, tile, color, 0);
}

static TILE_GET_INFO( get_nekkyoku_bg_tile_info ) { get_nekkyoku_tile_info(machine, tileinfo, tile_index, 0); }
static TILE_GET_INFO( get_nekkyoku_fg_tile_info ) { get_nekkyoku_tile_info(machine, tileinfo, tile_index, 1); }



/*************************************
 *
 *  Video system start
 *
 *************************************/

static void init_common(running_machine *machine)
{
	flipscreen_old = -1;

	/* allocate local videoram */
	local_videoram[0] = auto_alloc_array(machine, UINT8, 0x1000 * 3);
	local_videoram[1] = auto_alloc_array(machine, UINT8, 0x1000 * 3);

	/* allocate local palette RAM */
	local_paletteram = auto_alloc_array(machine, UINT8, 0x800 * 2);

	/* configure tilemaps */
	tilemap_set_transparent_pen(fg_tilemap,15);

	/* reset the timer */
	crtc_timer = timer_alloc(machine, crtc_interrupt_gen, NULL);

	scrollx_ofs = 0x159;
	scrolly_ofs = 0x10;

	/* state save */
	state_save_register_global(machine, selected_videoram);
	state_save_register_global_pointer(machine, local_videoram[0], 0x1000 * 3);
	state_save_register_global_pointer(machine, local_videoram[1], 0x1000 * 3);
	state_save_register_global(machine, selected_paletteram);
	state_save_register_global_array(machine, scrollx);
	state_save_register_global_array(machine, scrolly);
	state_save_register_global(machine, gfxreg);
	state_save_register_global(machine, flipscreen);
	state_save_register_global(machine, flipscreen_old);
	state_save_register_global(machine, scrollx_ofs);
	state_save_register_global(machine, scrolly_ofs);
	state_save_register_global(machine, crtc_register);
	state_save_register_global_array(machine, crtc_data);
	state_save_register_global_pointer(machine, local_paletteram, 0x800 * 2);
}

VIDEO_START( fromance )
{
	/* allocate tilemaps */
	bg_tilemap = tilemap_create(machine, get_fromance_bg_tile_info, tilemap_scan_rows,       8,4, 64,64);
	fg_tilemap = tilemap_create(machine, get_fromance_fg_tile_info, tilemap_scan_rows,  8,4, 64,64);

	init_common(machine);
}

VIDEO_START( nekkyoku )
{
	/* allocate tilemaps */
	bg_tilemap = tilemap_create(machine, get_nekkyoku_bg_tile_info, tilemap_scan_rows,       8,4, 64,64);
	fg_tilemap = tilemap_create(machine, get_nekkyoku_fg_tile_info, tilemap_scan_rows,  8,4, 64,64);

	init_common(machine);
}

VIDEO_START( pipedrm )
{
	VIDEO_START_CALL(fromance);
	scrolly_ofs = 0x00;
}

VIDEO_START( hatris )
{
	VIDEO_START_CALL(fromance);
	scrollx_ofs = 0xB9;
	scrolly_ofs = 0x00;
}

/*************************************
 *
 *  Graphics control register
 *
 *************************************/

WRITE8_HANDLER( fromance_gfxreg_w )
{

	gfxreg = data;
	flipscreen = (data & 0x01);
	selected_videoram = (~data >> 1) & 1;
	selected_paletteram = (data >> 6) & 1;

	if (flipscreen != flipscreen_old)
	{
		flipscreen_old = flipscreen;
		tilemap_set_flip_all(space->machine, flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	}
}



/*************************************
 *
 *  Banked palette RAM
 *
 *************************************/

READ8_HANDLER( fromance_paletteram_r )
{
	/* adjust for banking and read */
	offset |= selected_paletteram << 11;
	return local_paletteram[offset];
}


WRITE8_HANDLER( fromance_paletteram_w )
{
	int palword;

	/* adjust for banking and modify */
	offset |= selected_paletteram << 11;
	local_paletteram[offset] = data;

	/* compute R,G,B */
	palword = (local_paletteram[offset | 1] << 8) | local_paletteram[offset & ~1];
	palette_set_color_rgb(space->machine, offset / 2, pal5bit(palword >> 10), pal5bit(palword >> 5), pal5bit(palword >> 0));
}



/*************************************
 *
 *  Video RAM read/write
 *
 *************************************/

READ8_HANDLER( fromance_videoram_r )
{
	return local_videoram[selected_videoram][offset];
}


WRITE8_HANDLER( fromance_videoram_w )
{
	local_videoram[selected_videoram][offset] = data;
	tilemap_mark_tile_dirty(selected_videoram ? fg_tilemap : bg_tilemap, offset & 0x0fff);
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

WRITE8_HANDLER( fromance_scroll_w )
{
	if (flipscreen)
	{
		switch (offset)
		{
			case 0:
				scrollx[1] = (data + (((gfxreg & 0x08) >> 3) * 0x100) - scrollx_ofs);
				break;
			case 1:
				scrolly[1] = (data + (((gfxreg & 0x04) >> 2) * 0x100) - scrolly_ofs); // - 0x10
				break;
			case 2:
				scrollx[0] = (data + (((gfxreg & 0x20) >> 5) * 0x100) - scrollx_ofs);
				break;
			case 3:
				scrolly[0] = (data + (((gfxreg & 0x10) >> 4) * 0x100) - scrolly_ofs);
				break;
		}
	}
	else
	{
		switch (offset)
		{
			case 0:
				scrollx[1] = (data + (((gfxreg & 0x08) >> 3) * 0x100) - 0x1f7);
				break;
			case 1:
				scrolly[1] = (data + (((gfxreg & 0x04) >> 2) * 0x100) - 0xf9);
				break;
			case 2:
				scrollx[0] = (data + (((gfxreg & 0x20) >> 5) * 0x100) - 0x1f7);
				break;
			case 3:
				scrolly[0] = (data + (((gfxreg & 0x10) >> 4) * 0x100) - 0xf9);
				break;
		}
	}
}



/*************************************
 *
 *  Fake video controller
 *
 *************************************/

static TIMER_CALLBACK( crtc_interrupt_gen )
{
	cputag_set_input_line(machine, "sub", 0, HOLD_LINE);
	if (param != 0)
		timer_adjust_periodic(crtc_timer, attotime_div(video_screen_get_frame_period(machine->primary_screen), param), 0, attotime_div(video_screen_get_frame_period(machine->primary_screen), param));
}


WRITE8_HANDLER( fromance_crtc_data_w )
{
	crtc_data[crtc_register] = data;

	switch (crtc_register)
	{
		/* only register we know about.... */
		case 0x0b:
			timer_adjust_oneshot(crtc_timer, video_screen_get_time_until_vblank_start(space->machine->primary_screen), (data > 0x80) ? 2 : 1);
			break;

		default:
			logerror("CRTC register %02X = %02X\n", crtc_register, data & 0xff);
			break;
	}
}


WRITE8_HANDLER( fromance_crtc_register_w )
{
	crtc_register = data;
}



/*************************************
 *
 *  Sprite routines (Pipe Dream)
 *
 *************************************/

static void draw_sprites(const device_config *screen, bitmap_t *bitmap, const rectangle *cliprect, int draw_priority)
{
	static const UINT8 zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };
	const rectangle *visarea = video_screen_get_visible_area(screen);
	int offs;

	/* draw the sprites */
	for (offs = 0; offs < spriteram_size; offs += 8)
	{
		int data2 = spriteram[offs + 4] | (spriteram[offs + 5] << 8);
		int priority = (data2 >> 4) & 1;

		/* turns out the sprites are the same as in aerofgt.c */
		if ((data2 & 0x80) && priority == draw_priority)
		{
			int data0 = spriteram[offs + 0] | (spriteram[offs + 1] << 8);
			int data1 = spriteram[offs + 2] | (spriteram[offs + 3] << 8);
			int data3 = spriteram[offs + 6] | (spriteram[offs + 7] << 8);
			int code = data3 & 0xfff;
			int color = data2 & 0x0f;
			int y = (data0 & 0x1ff) - 6;
			int x = (data1 & 0x1ff) - 13;
			int yzoom = (data0 >> 12) & 15;
			int xzoom = (data1 >> 12) & 15;
			int zoomed = (xzoom | yzoom);
			int ytiles = ((data2 >> 12) & 7) + 1;
			int xtiles = ((data2 >> 8) & 7) + 1;
			int yflip = (data2 >> 15) & 1;
			int xflip = (data2 >> 11) & 1;
			int xt, yt;

			/* compute the zoom factor -- stolen from aerofgt.c */
			xzoom = 16 - zoomtable[xzoom] / 8;
			yzoom = 16 - zoomtable[yzoom] / 8;

			/* wrap around */
			if (x > visarea->max_x) x -= 0x200;
			if (y > visarea->max_y) y -= 0x200;

			/* flip ? */
			if (flipscreen)
			{
				y = visarea->max_y - y - 16 * ytiles - 4;
				x = visarea->max_x - x - 16 * xtiles - 24;
				xflip=!xflip;
				yflip=!yflip;
			}

			/* normal case */
			if (!xflip && !yflip)
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[2], code, color, 0, 0,
									x + xt * 16, y + yt * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen->machine->gfx[2], code, color, 0, 0,
									x + xt * xzoom, y + yt * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}

			/* xflipped case */
			else if (xflip && !yflip)
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[2], code, color, 1, 0,
									x + (xtiles - 1 - xt) * 16, y + yt * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen->machine->gfx[2], code, color, 1, 0,
									x + (xtiles - 1 - xt) * xzoom, y + yt * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}

			/* yflipped case */
			else if (!xflip && yflip)
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[2], code, color, 0, 1,
									x + xt * 16, y + (ytiles - 1 - yt) * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen->machine->gfx[2], code, color, 0, 1,
									x + xt * xzoom, y + (ytiles - 1 - yt) * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}

			/* x & yflipped case */
			else
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[2], code, color, 1, 1,
									x + (xtiles - 1 - xt) * 16, y + (ytiles - 1 - yt) * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen->machine->gfx[2], code, color, 1, 1,
									x + (xtiles - 1 - xt) * xzoom, y + (ytiles - 1 - yt) * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}
		}
	}
}



/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

VIDEO_UPDATE( fromance )
{
	tilemap_set_scrollx(bg_tilemap, 0, scrollx[0]);
	tilemap_set_scrolly(bg_tilemap, 0, scrolly[0]);
	tilemap_set_scrollx(fg_tilemap, 0, scrollx[1]);
	tilemap_set_scrolly(fg_tilemap, 0, scrolly[1]);

	tilemap_draw(bitmap,cliprect, bg_tilemap, 0, 0);
	tilemap_draw(bitmap,cliprect, fg_tilemap, 0, 0);
	return 0;
}


VIDEO_UPDATE( pipedrm )
{
	/* there seems to be no logical mapping for the X scroll register -- maybe it's gone */
	tilemap_set_scrolly(bg_tilemap, 0, scrolly[1]);
	tilemap_set_scrolly(fg_tilemap, 0, scrolly[0]);

	tilemap_draw(bitmap,cliprect, bg_tilemap, 0, 0);
	tilemap_draw(bitmap,cliprect, fg_tilemap, 0, 0);

	draw_sprites(screen,bitmap,cliprect, 0);
	draw_sprites(screen,bitmap,cliprect, 1);
	return 0;
}
