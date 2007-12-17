/***************************************************************************

    Sega System 18 hardware

***************************************************************************/

#include "driver.h"
#include "segaic16.h"
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

static mame_bitmap *tempbitmap;

static UINT8 grayscale_enable;
static UINT8 vdp_enable;
static UINT8 vdp_mixing;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

extern void start_system18_vdp(void);
extern void update_system18_vdp(mame_bitmap *bitmap, const rectangle *cliprect);



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( system18 )
{
	/* compute palette info */
	segaic16_palette_init(0x800);

	/* initialize the tile/text layers */
	segaic16_tilemap_init(0, SEGAIC16_TILEMAP_16B, 0x000, 0, 8);

	/* initialize the sprites */
	segaic16_sprites_init(0, SEGAIC16_SPRITES_16B, 0x400, 0);

	/* create the VDP */
	start_system18_vdp();

	/* create a temp bitmap to draw the VDP data into */
	tempbitmap = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, BITMAP_FORMAT_INDEXED16);
}



/*************************************
 *
 *  Miscellaneous setters
 *
 *************************************/

void system18_set_grayscale(int enable)
{
	enable = (enable != 0);
	if (enable != grayscale_enable)
	{
		video_screen_update_partial(0, video_screen_get_vpos(0));
		grayscale_enable = enable;
//      mame_printf_debug("Grayscale = %02X\n", enable);
	}
}


void system18_set_vdp_enable(int enable)
{
	enable = (enable != 0);
	if (enable != vdp_enable)
	{
		video_screen_update_partial(0, video_screen_get_vpos(0));
		vdp_enable = enable;
#if DEBUG_VDP
		mame_printf_debug("VDP enable = %02X\n", enable);
#endif
	}
}


void system18_set_vdp_mixing(int mixing)
{
	if (mixing != vdp_mixing)
	{
		video_screen_update_partial(0, video_screen_get_vpos(0));
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

static void draw_vdp(mame_bitmap *bitmap, const rectangle *cliprect, int priority)
{
	int x, y;

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
	if (input_code_pressed(KEYCODE_Q)) vdplayer = 0;
	if (input_code_pressed(KEYCODE_W)) vdplayer = 1;
	if (input_code_pressed(KEYCODE_E)) vdplayer = 2;
	if (input_code_pressed(KEYCODE_R)) vdplayer = 3;
	if (input_code_pressed(KEYCODE_A)) vdppri = 0x00;
	if (input_code_pressed(KEYCODE_S)) vdppri = 0x01;
	if (input_code_pressed(KEYCODE_D)) vdppri = 0x02;
	if (input_code_pressed(KEYCODE_F)) vdppri = 0x04;
	if (input_code_pressed(KEYCODE_G)) vdppri = 0x08;
#endif

	/* if no drawing is happening, fill with black and get out */
	if (!segaic16_display_enable)
	{
		fillbitmap(bitmap, get_black_pen(machine), cliprect);
		return 0;
	}

	/* if the VDP is enabled, update our tempbitmap */
	if (vdp_enable)
		update_system18_vdp(tempbitmap, cliprect);

	/* reset priorities */
	fillbitmap(priority_bitmap, 0, cliprect);

	/* draw background opaquely first, not setting any priorities */
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_BACKGROUND, 0 | TILEMAP_DRAW_OPAQUE, 0x00);
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_BACKGROUND, 1 | TILEMAP_DRAW_OPAQUE, 0x00);
	if (vdp_enable && vdplayer == 0) draw_vdp(bitmap, cliprect, vdppri);

	/* draw background again to draw non-transparent pixels over the VDP and set the priority */
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);
	if (vdp_enable && vdplayer == 1) draw_vdp(bitmap, cliprect, vdppri);

	/* draw foreground */
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);
	if (vdp_enable && vdplayer == 2) draw_vdp(bitmap, cliprect, vdppri);

	/* text layer */
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_TEXT, 0, 0x04);
	segaic16_tilemap_draw(machine, 0, bitmap, cliprect, SEGAIC16_TILEMAP_TEXT, 1, 0x08);
	if (vdp_enable && vdplayer == 3) draw_vdp(bitmap, cliprect, vdppri);

	/* draw the sprites */
	segaic16_sprites_draw(0, bitmap, cliprect);

#if DEBUG_VDP
	if (vdp_enable && input_code_pressed(KEYCODE_V))
	{
		fillbitmap(bitmap, get_black_pen(machine), cliprect);
		update_system18_vdp(bitmap, cliprect);
	}
	if (vdp_enable && input_code_pressed(KEYCODE_B))
	{
		FILE *f = fopen("vdp.bin", "w");
		fwrite(tempbitmap->base, 1, tempbitmap->rowpixels * (tempbitmap->bpp / 8) * tempbitmap->height, f);
		fclose(f);
	}
#endif
	return 0;
}
