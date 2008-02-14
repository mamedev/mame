#include "driver.h"
#include "video/konamiic.h"


int scontra_priority;
static int layer_colorbase[3],sprite_colorbase;

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static void tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= ((*color & 0x1f) << 8) | (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

static void sprite_callback(int *code,int *color,int *priority_mask,int *shadow)
{
	/* Sprite priority 1 means appear behind background, used only to mask sprites */
	/* in the foreground */
	/* Sprite priority 3 means don't draw (not used) */
	switch (*color & 0x30)
	{
		case 0x00: *priority_mask = 0xf0; break;
		case 0x10: *priority_mask = 0xf0|0xcc|0xaa; break;
		case 0x20: *priority_mask = 0xf0|0xcc; break;
		case 0x30: *priority_mask = 0xffff; break;
	}

	*color = sprite_colorbase + (*color & 0x0f);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( scontra )
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

VIDEO_UPDATE( scontra )
{
	K052109_tilemap_update();

	fillbitmap(priority_bitmap,0,cliprect);

	/* The background color is always from layer 1 - but it's always black anyway */
//  fillbitmap(bitmap,machine->pens[16 * layer_colorbase[1]],cliprect);
	if (scontra_priority)
	{
		tilemap_draw(bitmap,cliprect,K052109_tilemap[2],TILEMAP_DRAW_OPAQUE,1);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[1],0,2);
	}
	else
	{
		tilemap_draw(bitmap,cliprect,K052109_tilemap[1],TILEMAP_DRAW_OPAQUE,1);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[2],0,2);
	}
	tilemap_draw(bitmap,cliprect,K052109_tilemap[0],0,4);

	K051960_sprites_draw(bitmap,cliprect,-1,-1);
	return 0;
}
