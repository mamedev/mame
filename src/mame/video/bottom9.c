#include "driver.h"
#include "video/konicdev.h"



int bottom9_video_enable;

static int layer_colorbase[3],sprite_colorbase,zoom_colorbase;


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void bottom9_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= (*color & 0x3f) << 8;
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void bottom9_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow)
{
	/* bit 4 = priority over zoom (0 = have priority) */
	/* bit 5 = priority over B (1 = have priority) */
	*priority = (*color & 0x30) >> 4;
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void bottom9_zoom_callback(running_machine *machine, int *code,int *color,int *flags)
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
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( bottom9 )
{
	const device_config *k052109 = devtag_get_device(screen->machine, "k052109");
	const device_config *k051960 = devtag_get_device(screen->machine, "k051960");
	const device_config *k051316 = devtag_get_device(screen->machine, "k051316");

	k052109_tilemap_update(k052109);

	/* note: FIX layer is not used */
	bitmap_fill(bitmap,cliprect,layer_colorbase[1]);
//  if (bottom9_video_enable)
	{
		k051960_sprites_draw(k051960, bitmap, cliprect, 1, 1);
		k051316_zoom_draw(k051316, bitmap, cliprect, 0, 0);
		k051960_sprites_draw(k051960, bitmap, cliprect, 0, 0);
		k052109_tilemap_draw(k052109, bitmap, cliprect, 2, 0, 0);
		/* note that priority 3 is opposite to the basic layer priority! */
		/* (it IS used, but hopefully has no effect) */
		k051960_sprites_draw(k051960, bitmap, cliprect, 2, 3);
		k052109_tilemap_draw(k052109, bitmap, cliprect, 1, 0, 0);
	}
	return 0;
}
