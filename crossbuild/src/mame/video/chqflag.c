/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/konamiic.h"
#include "cpu/z80/z80.h"

#define SPRITEROM_MEM_REGION REGION_GFX1
#define ZOOMROM0_MEM_REGION REGION_GFX2
#define ZOOMROM1_MEM_REGION REGION_GFX3

static int sprite_colorbase,zoom_colorbase[2];

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

static void sprite_callback(int *code,int *color,int *priority,int *shadow)
{
	*priority = (*color & 0x10) >> 4;
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

static void zoom_callback_0(int *code,int *color,int *flags)
{
	*code |= ((*color & 0x03) << 8);
	*color = zoom_colorbase[0] + ((*color & 0x3c) >> 2);
}

static void zoom_callback_1(int *code,int *color,int *flags)
{
	*flags = TILE_FLIPYX((*color & 0xc0) >> 6);
	*code |= ((*color & 0x0f) << 8);
	*color = zoom_colorbase[1] + ((*color & 0x10) >> 4);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( chqflag )
{
	sprite_colorbase = 0;
	zoom_colorbase[0] = 0x10;
	zoom_colorbase[1] = 0x02;

	K051960_vh_start(machine,SPRITEROM_MEM_REGION,NORMAL_PLANE_ORDER,sprite_callback);
	K051316_vh_start_0(machine,ZOOMROM0_MEM_REGION,4,FALSE,0,zoom_callback_0);
	K051316_vh_start_1(machine,ZOOMROM1_MEM_REGION,8,TRUE,0xc0,zoom_callback_1);

	K051316_set_offset(0,7,0);
	K051316_wraparound_enable(1,1);
}

/***************************************************************************

    Display Refresh

***************************************************************************/

VIDEO_UPDATE( chqflag )
{
	fillbitmap(bitmap,machine->pens[0],cliprect);

	K051316_zoom_draw_1(bitmap,cliprect,TILEMAP_DRAW_LAYER1,0);
	K051960_sprites_draw(bitmap,cliprect,0,0);
	K051316_zoom_draw_1(bitmap,cliprect,TILEMAP_DRAW_LAYER0,0);
	K051960_sprites_draw(bitmap,cliprect,1,1);
	K051316_zoom_draw_0(bitmap,cliprect,0,0);
	return 0;
}
