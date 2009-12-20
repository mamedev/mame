#include "driver.h"
#include "video/konicdev.h"


int spy_video_enable;

static int layer_colorbase[3],sprite_colorbase;


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void spy_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*flags = (*color & 0x20) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9)
			| (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void spy_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow)
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
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( spy )
{
	const device_config *k052109 = devtag_get_device(screen->machine, "k052109");
	const device_config *k051960 = devtag_get_device(screen->machine, "k051960");

	k052109_tilemap_update(k052109);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	if (!spy_video_enable)
		bitmap_fill(bitmap,cliprect,16 * layer_colorbase[0]);
	else
	{
		k052109_tilemap_draw(k052109, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 1);
		k052109_tilemap_draw(k052109, bitmap, cliprect, 2, 0, 2);
		k051960_sprites_draw(k051960, bitmap, cliprect, -1, -1);
		k052109_tilemap_draw(k052109, bitmap, cliprect, 0, 0, 0);
	}

	return 0;
}
