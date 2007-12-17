#include "driver.h"
#include "video/konamiic.h"



int gbusters_priority;
static int layer_colorbase[3],sprite_colorbase;

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static void tile_callback(int layer,int bank,int *code,int *color,int *flags, int *priority)
{
	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x0d) << 8) | ((*color & 0x10) << 5) | (bank << 12);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

static void sprite_callback(int *code,int *color,int *priority,int *shadow)
{
	*priority = (*color & 0x30) >> 4;
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gbusters )
{
	layer_colorbase[0] = 48;
	layer_colorbase[1] = 0;
	layer_colorbase[2] = 16;
	sprite_colorbase = 32;

	K052109_vh_start(machine,REGION_GFX1,NORMAL_PLANE_ORDER,tile_callback);
	K051960_vh_start(machine,REGION_GFX2,NORMAL_PLANE_ORDER,sprite_callback);
}


VIDEO_UPDATE( gbusters )
{
	K052109_tilemap_update();

	/* sprite priority 3 = disable */
	if (gbusters_priority)
	{
//      K051960_sprites_draw(bitmap,cliprect,1,1);  /* are these used? */
		tilemap_draw(bitmap,cliprect,K052109_tilemap[2],TILEMAP_DRAW_OPAQUE,0);
		K051960_sprites_draw(bitmap,cliprect,2,2);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[1],0,0);
		K051960_sprites_draw(bitmap,cliprect,0,0);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[0],0,0);
	}
	else
	{
//      K051960_sprites_draw(bitmap,cliprect,1,1);  /* are these used? */
		tilemap_draw(bitmap,cliprect,K052109_tilemap[1],TILEMAP_DRAW_OPAQUE,0);
		K051960_sprites_draw(bitmap,cliprect,2,2);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[2],0,0);
		K051960_sprites_draw(bitmap,cliprect,0,0);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[0],0,0);
	}
	return 0;
}
