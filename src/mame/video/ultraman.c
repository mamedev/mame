#include "driver.h"
#include "video/konamiic.h"

#define SPRITEROM_MEM_REGION REGION_GFX1
#define ZOOMROM0_MEM_REGION REGION_GFX2
#define ZOOMROM1_MEM_REGION REGION_GFX3
#define ZOOMROM2_MEM_REGION REGION_GFX4

static int sprite_colorbase, zoom_colorbase[3];
static int bank0,bank1,bank2;


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

static void sprite_callback(int *code,int *color,int *priority,int *shadow)
{
	*priority = (*color & 0x80) >> 7;
	*color = sprite_colorbase + ((*color & 0x7e) >> 1);
	*shadow = 0;
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

static void zoom_callback_0(int *code,int *color,int *flags)
{
	*code |= ((*color & 0x07) << 8) | (bank0 << 11);
	*color = zoom_colorbase[0] + ((*color & 0xf8) >> 3);
}

static void zoom_callback_1(int *code,int *color,int *flags)
{
	*code |= ((*color & 0x07) << 8) | (bank1 << 11);
	*color = zoom_colorbase[1] + ((*color & 0xf8) >> 3);
}

static void zoom_callback_2(int *code,int *color,int *flags)
{
	*code |= ((*color & 0x07) << 8) | (bank2 << 11);
	*color = zoom_colorbase[2] + ((*color & 0xf8) >> 3);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( ultraman )
{
	sprite_colorbase = 192;
	zoom_colorbase[0] = 0;
	zoom_colorbase[1] = 64;
	zoom_colorbase[2] = 128;

	K051960_vh_start(machine,SPRITEROM_MEM_REGION,NORMAL_PLANE_ORDER,sprite_callback);
	K051316_vh_start_0(machine,ZOOMROM0_MEM_REGION,4,FALSE,0,zoom_callback_0);
	K051316_vh_start_1(machine,ZOOMROM1_MEM_REGION,4,FALSE,0,zoom_callback_1);
	K051316_vh_start_2(machine,ZOOMROM2_MEM_REGION,4,TRUE,0,zoom_callback_2);

	K051316_set_offset(0, 8, 0);
	K051316_set_offset(1, 8, 0);
	K051316_set_offset(2, 8, 0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( ultraman_gfxctrl_w )
{
	if (ACCESSING_LSB)
	{
		/*  bit 0: enable wraparound for scr #1
            bit 1: msb of code for scr #1
            bit 2: enable wraparound for scr #2
            bit 3: msb of code for scr #2
            bit 4: enable wraparound for scr #3
            bit 5: msb of code for scr #3
            bit 6: coin counter 1
            bit 7: coin counter 2 */
		K051316_wraparound_enable(0,data & 0x01);
		if (bank0 != ((data & 0x02) >> 1))
		{
			bank0 = (data & 0x02) >> 1;
			tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);	/* should mark only zoom0 */
		}
		K051316_wraparound_enable(1,data & 0x04);
		if (bank1 != ((data & 0x08) >> 3))
		{
			bank1 = (data & 0x08) >> 3;
			tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);	/* should mark only zoom1 */
		}
		K051316_wraparound_enable(2,data & 0x10);
		if (bank2 != ((data & 0x20) >> 5))
		{
			bank2 = (data & 0x20) >> 5;
			tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);	/* should mark only zoom2 */
		}
		coin_counter_w(0, data & 0x40);
		coin_counter_w(1, data & 0x80);
	}
}



/***************************************************************************

    Display Refresh

***************************************************************************/

VIDEO_UPDATE( ultraman )
{
	K051316_zoom_draw_2(bitmap,cliprect,0,0);
	K051316_zoom_draw_1(bitmap,cliprect,0,0);
	K051960_sprites_draw(bitmap,cliprect,0,0);
	K051316_zoom_draw_0(bitmap,cliprect,0,0);
	K051960_sprites_draw(bitmap,cliprect,1,1);
	return 0;
}
