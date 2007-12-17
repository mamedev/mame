/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/konamiic.h"


UINT8 ajax_priority;
static int layer_colorbase[3],sprite_colorbase,zoom_colorbase;


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static void tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= ((*color & 0x0f) << 8) | (bank << 12);
	*color = layer_colorbase[layer] + ((*color & 0xf0) >> 4);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

static void sprite_callback(int *code,int *color,int *priority,int *shadow)
{
	/* priority bits:
       4 over zoom (0 = have priority)
       5 over B    (0 = have priority)
       6 over A    (1 = have priority)
       never over F
    */
	*priority = 0xff00;							/* F = 8 */
	if ( *color & 0x10) *priority |= 0xf0f0;	/* Z = 4 */
	if (~*color & 0x40) *priority |= 0xcccc;	/* A = 2 */
	if ( *color & 0x20) *priority |= 0xaaaa;	/* B = 1 */
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

static void zoom_callback(int *code,int *color,int *flags)
{
	*code |= ((*color & 0x07) << 8);
	*color = zoom_colorbase + ((*color & 0x08) >> 3);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( ajax )
{
	layer_colorbase[0] = 64;
	layer_colorbase[1] = 0;
	layer_colorbase[2] = 32;
	sprite_colorbase = 16;
	zoom_colorbase = 6;	/* == 48 since it's 7-bit graphics */
	K052109_vh_start(machine,REGION_GFX1,NORMAL_PLANE_ORDER,tile_callback);
	K051960_vh_start(machine,REGION_GFX2,NORMAL_PLANE_ORDER,sprite_callback);
	K051316_vh_start_0(machine,REGION_GFX3,7,FALSE,0,zoom_callback);
}



/***************************************************************************

    Display Refresh

***************************************************************************/

VIDEO_UPDATE( ajax )
{
	K052109_tilemap_update();

	fillbitmap(priority_bitmap,0,cliprect);

	fillbitmap(bitmap,get_black_pen(machine),cliprect);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[2],0,1);
	if (ajax_priority)
	{
		/* basic layer order is B, zoom, A, F */
		K051316_zoom_draw_0(bitmap,cliprect,0,4);
		tilemap_draw(bitmap,cliprect,K052109_tilemap[1],0,2);
	}
	else
	{
		/* basic layer order is B, A, zoom, F */
		tilemap_draw(bitmap,cliprect,K052109_tilemap[1],0,2);
		K051316_zoom_draw_0(bitmap,cliprect,0,4);
	}
	tilemap_draw(bitmap,cliprect,K052109_tilemap[0],0,8);

	K051960_sprites_draw(bitmap,cliprect,-1,-1);
	return 0;
}
