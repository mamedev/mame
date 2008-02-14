#include "driver.h"
#include "video/konamiic.h"


static int layer_colorbase[3],sprite_colorbase;

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static void tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*flags = (*color & 0x20) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x1f) << 8) | (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

static void sprite_callback(int *code,int *color,int *priority,int *shadow)
{
	/* Weird priority scheme. Why use three bits when two would suffice? */
	/* The PROM allows for mixed priorities, where sprites would have */
	/* priority over text but not on one or both of the other two planes. */
	/* Luckily, this isn't used by the game. */
	switch (*color & 0x70)
	{
		case 0x10: *priority = 0; break;
		case 0x00: *priority = 1; break;
		case 0x40: *priority = 2; break;
		case 0x20: *priority = 3; break;
		/*   0x60 == 0x20 */
		/*   0x50 priority over F and A, but not over B */
		/*   0x30 priority over F, but not over A and B */
		/*   0x70 == 0x30 */
	}
	/* bit 7 is on in the "Game Over" sprites, meaning unknown */
	/* in Aliens it is the top bit of the code, but that's not needed here */
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( crimfght )
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

VIDEO_UPDATE( crimfght )
{
	K052109_tilemap_update();

	tilemap_draw(bitmap,cliprect,K052109_tilemap[1],TILEMAP_DRAW_OPAQUE,0);
	K051960_sprites_draw(bitmap,cliprect,2,2);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[2],0,0);
	K051960_sprites_draw(bitmap,cliprect,1,1);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[0],0,0);
	K051960_sprites_draw(bitmap,cliprect,0,0);
	return 0;
}
