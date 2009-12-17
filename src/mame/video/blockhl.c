#include "driver.h"
#include "video/konicdev.h"



static int layer_colorbase[3],sprite_colorbase;

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void blockhl_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color, int *flags, int *priority)
{
	*code |= ((*color & 0x0f) << 8);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void blockhl_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow)
{
	if(*color & 0x10)
		*priority = 0xfe; // under K052109_tilemap[0]
	else
		*priority = 0xfc; // under K052109_tilemap[1]

	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( blockhl )
{
	layer_colorbase[0] = 0;
	layer_colorbase[1] = 16;
	layer_colorbase[2] = 32;
	sprite_colorbase = 48;
}


VIDEO_UPDATE( blockhl )
{
	const device_config *k052109 = devtag_get_device(screen->machine, "k052109");
	const device_config *k051960 = devtag_get_device(screen->machine, "k051960");

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	k052109_tilemap_update(k052109);

	k052109_tilemap_draw(k052109, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE, 0);	// tile 2
	k052109_tilemap_draw(k052109, bitmap, cliprect, 1, 0, 1);	// tile 1
	k052109_tilemap_draw(k052109, bitmap, cliprect, 0, 0, 2);	// tile 0

	k051960_sprites_draw(k051960, bitmap,cliprect,0,-1);
	return 0;
}
