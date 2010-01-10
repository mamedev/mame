/***************************************************************************

    Sega System 18 hardware

***************************************************************************/

#include "emu.h"
#include "segaic16.h"
#include "includes/genesis.h"
#include "includes/system16.h"



/*************************************
 *
 *  Debugging
 *
 *************************************/

#define DEBUG_VDP				(0)



/*************************************
 *
 *  Statics
 *
 *************************************/

static bitmap_t *tempbitmap;

static UINT8 grayscale_enable;
static UINT8 vdp_enable;
static UINT8 vdp_mixing;



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( system18 )
{
	int width, height;

	tempbitmap = NULL;
	grayscale_enable = 0;
	vdp_enable = 0;
	vdp_mixing = 0;

	/* compute palette info */
	segaic16_palette_init(0x800);

	/* initialize the tile/text layers */
	segaic16_tilemap_init(machine, 0, SEGAIC16_TILEMAP_16B, 0x000, 0, 8);

	/* initialize the sprites */
	segaic16_sprites_init(machine, 0, SEGAIC16_SPRITES_16B, 0x400, 0);

	/* create the VDP */
	system18_vdp_start(machine);

	/* create a temp bitmap to draw the VDP data into */
	width = video_screen_get_width(machine->primary_screen);
	height = video_screen_get_height(machine->primary_screen);
	tempbitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16);
}



/*************************************
 *
 *  Miscellaneous setters
 *
 *************************************/

void system18_set_grayscale(running_machine *machine, int enable)
{
	enable = (enable != 0);
	if (enable != grayscale_enable)
	{
		video_screen_update_partial(machine->primary_screen, video_screen_get_vpos(machine->primary_screen));
		grayscale_enable = enable;
//      mame_printf_debug("Grayscale = %02X\n", enable);
	}
}


void system18_set_vdp_enable(running_machine *machine, int enable)
{
	enable = (enable != 0);
	if (enable != vdp_enable)
	{
		video_screen_update_partial(machine->primary_screen, video_screen_get_vpos(machine->primary_screen));
		vdp_enable = enable;
#if DEBUG_VDP
		mame_printf_debug("VDP enable = %02X\n", enable);
#endif
	}
}


void system18_set_vdp_mixing(running_machine *machine, int mixing)
{
	if (mixing != vdp_mixing)
	{
		video_screen_update_partial(machine->primary_screen, video_screen_get_vpos(machine->primary_screen));
		vdp_mixing = mixing;
#if DEBUG_VDP
		mame_printf_debug("VDP mixing = %02X\n", mixing);
#endif
	}
}



/*************************************
 *
 *  VDP drawing
 *
 *************************************/

static void draw_vdp(const device_config *screen, bitmap_t *bitmap, const rectangle *cliprect, int priority)
{
	int x, y;
	bitmap_t *priority_bitmap = screen->machine->priority_bitmap;

	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *src = BITMAP_ADDR16(tempbitmap, y, 0);
		UINT16 *dst = BITMAP_ADDR16(bitmap, y, 0);
		UINT8 *pri = BITMAP_ADDR8(priority_bitmap, y, 0);

		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			UINT16 pix = src[x];
			if (pix != 0xffff)
			{
				dst[x] = pix;
				pri[x] |= priority;
			}
		}
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( system18 )
{
	int vdppri, vdplayer;

/*
    Current understanding of VDP mixing:

    mixing = 0x00:
        astorm: layer = 0, pri = 0x00 or 0x01
        lghost: layer = 0, pri = 0x00 or 0x01

    mixing = 0x01:
        ddcrew: layer = 0, pri = 0x00 or 0x01 or 0x02

    mixing = 0x02:
        never seen

    mixing = 0x03:
        ddcrew: layer = 1, pri = 0x00 or 0x01 or 0x02

    mixing = 0x04:
        astorm: layer = 2 or 3, pri = 0x00 or 0x01 or 0x02
        mwalk:  layer = 2 or 3, pri = 0x00 or 0x01 or 0x02

    mixing = 0x05:
        ddcrew: layer = 2, pri = 0x04
        wwally: layer = 2, pri = 0x04

    mixing = 0x06:
        never seen

    mixing = 0x07:
        cltchitr: layer = 1 or 2 or 3, pri = 0x02 or 0x04 or 0x08
        mwalk:    layer = 3, pri = 0x04 or 0x08
*/
	vdplayer = (vdp_mixing >> 1) & 3;
	vdppri = (vdp_mixing & 1) ? (1 << vdplayer) : 0;

#if DEBUG_VDP
	if (input_code_pressed(screen->machine, KEYCODE_Q)) vdplayer = 0;
	if (input_code_pressed(screen->machine, KEYCODE_W)) vdplayer = 1;
	if (input_code_pressed(screen->machine, KEYCODE_E)) vdplayer = 2;
	if (input_code_pressed(screen->machine, KEYCODE_R)) vdplayer = 3;
	if (input_code_pressed(screen->machine, KEYCODE_A)) vdppri = 0x00;
	if (input_code_pressed(screen->machine, KEYCODE_S)) vdppri = 0x01;
	if (input_code_pressed(screen->machine, KEYCODE_D)) vdppri = 0x02;
	if (input_code_pressed(screen->machine, KEYCODE_F)) vdppri = 0x04;
	if (input_code_pressed(screen->machine, KEYCODE_G)) vdppri = 0x08;
#endif

	/* if no drawing is happening, fill with black and get out */
	if (!segaic16_display_enable)
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	/* if the VDP is enabled, update our tempbitmap */
	if (vdp_enable)
		system18_vdp_update(tempbitmap, cliprect);

	/* reset priorities */
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	/* draw background opaquely first, not setting any priorities */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0 | TILEMAP_DRAW_OPAQUE, 0x00);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1 | TILEMAP_DRAW_OPAQUE, 0x00);
	if (vdp_enable && vdplayer == 0) draw_vdp(screen, bitmap, cliprect, vdppri);

	/* draw background again to draw non-transparent pixels over the VDP and set the priority */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);
	if (vdp_enable && vdplayer == 1) draw_vdp(screen, bitmap, cliprect, vdppri);

	/* draw foreground */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);
	if (vdp_enable && vdplayer == 2) draw_vdp(screen, bitmap, cliprect, vdppri);

	/* text layer */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 0, 0x04);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 1, 0x08);
	if (vdp_enable && vdplayer == 3) draw_vdp(screen, bitmap, cliprect, vdppri);

	/* draw the sprites */
	segaic16_sprites_draw(screen, bitmap, cliprect, 0);

#if DEBUG_VDP
	if (vdp_enable && input_code_pressed(screen->machine, KEYCODE_V))
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		update_system18_vdp(bitmap, cliprect);
	}
	if (vdp_enable && input_code_pressed(screen->machine, KEYCODE_B))
	{
		FILE *f = fopen("vdp.bin", "w");
		fwrite(tempbitmap->base, 1, tempbitmap->rowpixels * (tempbitmap->bpp / 8) * tempbitmap->height, f);
		fclose(f);
	}
#endif
	return 0;
}
