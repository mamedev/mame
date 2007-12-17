#include "driver.h"
#include "video/konamiic.h"


int spy_video_enable;

static int layer_colorbase[3],sprite_colorbase;


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static void tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*flags = (*color & 0x20) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9)
			| (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

static void sprite_callback(int *code,int *color,int *priority_mask,int *shadow)
{
	/* bit 4 = priority over layer A (0 = have priority) */
	/* bit 5 = priority over layer B (1 = have priority) */
	*priority_mask = 0x00;
	if ( *color & 0x10) *priority_mask |= 0xa;
	if (~*color & 0x20) *priority_mask |= 0xc;

	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( spy )
{
	layer_colorbase[0] = 48;
	layer_colorbase[1] = 0;
	layer_colorbase[2] = 16;
	sprite_colorbase = 32;
	K052109_vh_start(machine,REGION_GFX1,NORMAL_PLANE_ORDER,tile_callback);
	K051960_vh_start(machine,REGION_GFX2,NORMAL_PLANE_ORDER,sprite_callback);
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( spy )
{
	K052109_tilemap_update();

	fillbitmap(priority_bitmap, 0, cliprect);

	if (!spy_video_enable)
	{
		fillbitmap(bitmap,machine->pens[16 * layer_colorbase[0]],cliprect);
		return 0;
	}

	tilemap_draw(bitmap,cliprect,K052109_tilemap[1],TILEMAP_DRAW_OPAQUE,1);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[2],0,2);
	K051960_sprites_draw(bitmap,cliprect,-1,-1);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[0],0,0);
	return 0;
}
