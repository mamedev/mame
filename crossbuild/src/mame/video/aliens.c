#include "driver.h"
#include "video/konamiic.h"


static int layer_colorbase[3],sprite_colorbase;

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static void tile_callback(int layer,int bank,int *code,int *color, int *flags, int *priority)
{
	*code |= ((*color & 0x3f) << 8) | (bank << 14);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

static void sprite_callback(int *code,int *color,int *priority_mask,int *shadow)
{
	/* The PROM allows for mixed priorities, where sprites would have */
	/* priority over text but not on one or both of the other two planes. */
	switch (*color & 0x70)
	{
		case 0x10: *priority_mask = 0x00; break;			/* over ABF */
		case 0x00: *priority_mask = 0xf0          ; break;	/* over AB, not F */
		case 0x40: *priority_mask = 0xf0|0xcc     ; break;	/* over A, not BF */
		case 0x20:
		case 0x60: *priority_mask = 0xf0|0xcc|0xaa; break;	/* over -, not ABF */
		case 0x50: *priority_mask =      0xcc     ; break;	/* over AF, not B */
		case 0x30:
		case 0x70: *priority_mask =      0xcc|0xaa; break;	/* over F, not AB */
	}
	*code |= (*color & 0x80) << 6;
	*color = sprite_colorbase + (*color & 0x0f);
	*shadow = 0;	/* shadows are not used by this game */
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( aliens )
{
	paletteram = auto_malloc(0x400);

	layer_colorbase[0] = 0;
	layer_colorbase[1] = 4;
	layer_colorbase[2] = 8;
	sprite_colorbase = 16;
	K052109_vh_start(machine,REGION_GFX1,NORMAL_PLANE_ORDER,tile_callback);
	K051960_vh_start(machine,REGION_GFX2,NORMAL_PLANE_ORDER,sprite_callback);
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( aliens )
{
	K052109_tilemap_update();

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[layer_colorbase[1] * 16],cliprect);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[1],0,1);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[2],0,2);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[0],0,4);

	K051960_sprites_draw(bitmap,cliprect,-1,-1);
	return 0;
}
