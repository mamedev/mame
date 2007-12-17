#include "driver.h"
#include "video/konamiic.h"



int bottom9_video_enable;

static int layer_colorbase[3],sprite_colorbase,zoom_colorbase;


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static void tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= (*color & 0x3f) << 8;
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

static void sprite_callback(int *code,int *color,int *priority,int *shadow)
{
	/* bit 4 = priority over zoom (0 = have priority) */
	/* bit 5 = priority over B (1 = have priority) */
	*priority = (*color & 0x30) >> 4;
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

static void zoom_callback(int *code,int *color,int *flags)
{
	*flags = (*color & 0x40) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8);
	*color = zoom_colorbase + ((*color & 0x3c) >> 2);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( bottom9 )
{
	layer_colorbase[0] = 0;	/* not used */
	layer_colorbase[1] = 0;
	layer_colorbase[2] = 16;
	sprite_colorbase = 32;
	zoom_colorbase = 48;
	K052109_vh_start(machine,REGION_GFX1,NORMAL_PLANE_ORDER,tile_callback);
	K051960_vh_start(machine,REGION_GFX2,NORMAL_PLANE_ORDER,sprite_callback);
	K051316_vh_start_0(machine,REGION_GFX3,4,FALSE,0,zoom_callback);
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( bottom9 )
{
	K052109_tilemap_update();

	/* note: FIX layer is not used */
	fillbitmap(bitmap,machine->pens[layer_colorbase[1]],cliprect);
//  if (bottom9_video_enable)
	{
		K051960_sprites_draw(bitmap,cliprect,1,1);
		K051316_zoom_draw_0(bitmap,cliprect,0,0);
		K051960_sprites_draw(bitmap,cliprect,0,0);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[2],0,0);
		/* note that priority 3 is opposite to the basic layer priority! */
		/* (it IS used, but hopefully has no effect) */
		K051960_sprites_draw(bitmap,cliprect,2,3);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[1],0,0);
	}
	return 0;
}
