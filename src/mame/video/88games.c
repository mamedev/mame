#include "driver.h"
#include "video/konicdev.h"


int k88games_priority;

static int layer_colorbase[3],sprite_colorbase,zoom_colorbase;



/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void _88games_tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= ((*color & 0x0f) << 8) | (bank << 12);
	*color = layer_colorbase[layer] + ((*color & 0xf0) >> 4);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void _88games_sprite_callback(int *code,int *color,int *priority,int *shadow)
{
	*priority = (*color & 0x20) >> 5;	/* ??? */
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void _88games_zoom_callback(int *code,int *color,int *flags)
{
	*flags = (*color & 0x40) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x07) << 8);
	*color = zoom_colorbase + ((*color & 0x38) >> 3) + ((*color & 0x80) >> 4);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( 88games )
{
	layer_colorbase[0] = 64;
	layer_colorbase[1] = 0;
	layer_colorbase[2] = 16;
	sprite_colorbase = 32;
	zoom_colorbase = 48;

	state_save_register_global_array(machine, layer_colorbase);
	state_save_register_global(machine, k88games_priority);
	state_save_register_global(machine, sprite_colorbase);
	state_save_register_global(machine, zoom_colorbase);
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( 88games )
{
	const device_config *k052109 = devtag_get_device(screen->machine, "k052109");
	const device_config *k051960 = devtag_get_device(screen->machine, "k051960");
	const device_config *k051316 = devtag_get_device(screen->machine, "k051316");

	k052109_tilemap_update(k052109);

	if (k88games_priority)
	{
		k052109_tilemap_draw(k052109, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);	// tile 0
		k051960_sprites_draw(k051960, bitmap,cliprect, 1, 1);
		k052109_tilemap_draw(k052109, bitmap, cliprect, 2, 0, 0);	// tile 2
		k052109_tilemap_draw(k052109, bitmap, cliprect, 1, 0, 0);	// tile 1
		k051960_sprites_draw(k051960, bitmap, cliprect, 0, 0);
		k051316_zoom_draw(k051316, bitmap, cliprect, 0, 0);
	}
	else
	{
		k052109_tilemap_draw(k052109, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE, 0);	// tile 2
		k051316_zoom_draw(k051316, bitmap, cliprect, 0, 0);
		k051960_sprites_draw(k051960, bitmap, cliprect, 0, 0);
		k052109_tilemap_draw(k052109, bitmap, cliprect, 1, 0, 0);	// tile 1
		k051960_sprites_draw(k051960, bitmap, cliprect, 1, 1);
		k052109_tilemap_draw(k052109, bitmap, cliprect, 0, 0, 0);	// tile 0
	}
	return 0;
}
