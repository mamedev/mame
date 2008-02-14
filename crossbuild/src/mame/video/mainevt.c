/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/konamiic.h"


static int layer_colorbase[3],sprite_colorbase;



/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static void mainevt_tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*flags = (*color & 0x02) ? TILE_FLIPX : 0;

	/* priority relative to HALF priority sprites */
	*priority = (layer == 2) ? (*color & 0x20) >> 5 : 0;
	*code |= ((*color & 0x01) << 8) | ((*color & 0x1c) << 7);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}

static void dv_tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x01) << 8) | ((*color & 0x3c) << 7);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

static void mainevt_sprite_callback(int *code,int *color,int *priority_mask,int *shadow)
{
	/* bit 5 = priority over layer B (has precedence) */
	/* bit 6 = HALF priority over layer B (used for crowd when you get out of the ring) */
	if (*color & 0x20)		*priority_mask = 0xff00;
	else if (*color & 0x40)	*priority_mask = 0xff00|0xf0f0;
	else					*priority_mask = 0xff00|0xf0f0|0xcccc;
	/* bit 7 is shadow, not used */

	*color = sprite_colorbase + (*color & 0x03);
}

static void dv_sprite_callback(int *code,int *color,int *priority,int *shadow)
{
	/* TODO: the priority/shadow handling (bits 5-7) seems to be quite complex (see PROM) */
	*color = sprite_colorbase + (*color & 0x07);
}


/*****************************************************************************/

VIDEO_START( mainevt )
{
	layer_colorbase[0] = 0;
	layer_colorbase[1] = 8;
	layer_colorbase[2] = 4;
	sprite_colorbase = 12;

	K052109_vh_start(machine,REGION_GFX1,NORMAL_PLANE_ORDER,mainevt_tile_callback);
	K051960_vh_start(machine,REGION_GFX2,NORMAL_PLANE_ORDER,mainevt_sprite_callback);
}

VIDEO_START( dv )
{
	layer_colorbase[0] = 0;
	layer_colorbase[1] = 0;
	layer_colorbase[2] = 4;
	sprite_colorbase = 8;

	K052109_vh_start(machine,REGION_GFX1,NORMAL_PLANE_ORDER,dv_tile_callback);
	K051960_vh_start(machine,REGION_GFX2,NORMAL_PLANE_ORDER,dv_sprite_callback);
}

/*****************************************************************************/

VIDEO_UPDATE( mainevt )
{
	K052109_tilemap_update();

	fillbitmap(priority_bitmap,0,cliprect);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[1],TILEMAP_DRAW_OPAQUE,1);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[2],1,2);	/* low priority part of layer */
	tilemap_draw(bitmap,cliprect,K052109_tilemap[2],0,4);	/* high priority part of layer */
	tilemap_draw(bitmap,cliprect,K052109_tilemap[0],0,8);

	K051960_sprites_draw(bitmap,cliprect,-1,-1);
	return 0;
}

VIDEO_UPDATE( dv )
{
	K052109_tilemap_update();

	tilemap_draw(bitmap,cliprect,K052109_tilemap[1],TILEMAP_DRAW_OPAQUE,0);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[2],0,0);
	K051960_sprites_draw(bitmap,cliprect,0,0);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[0],0,0);
	return 0;
}
