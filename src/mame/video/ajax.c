/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/konicdev.h"
#include "includes/ajax.h"


UINT8 ajax_priority;
static int layer_colorbase[3],sprite_colorbase,zoom_colorbase;


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void ajax_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= ((*color & 0x0f) << 8) | (bank << 12);
	*color = layer_colorbase[layer] + ((*color & 0xf0) >> 4);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void ajax_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow)
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

void ajax_zoom_callback(running_machine *machine, int *code,int *color,int *flags)
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
	state_save_register_global(machine, ajax_priority);
}



/***************************************************************************

    Display Refresh

***************************************************************************/

VIDEO_UPDATE( ajax )
{
	const device_config *k051316 = devtag_get_device(screen->machine, "k051316");
	const device_config *k052109 = devtag_get_device(screen->machine, "k052109");
	const device_config *k051960 = devtag_get_device(screen->machine, "k051960");

	k052109_tilemap_update(k052109);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	k052109_tilemap_draw(k052109, bitmap, cliprect, 2, 0, 1);
	if (ajax_priority)
	{
		/* basic layer order is B, zoom, A, F */
		k051316_zoom_draw(k051316, bitmap, cliprect, 0, 4);
		k052109_tilemap_draw(k052109, bitmap, cliprect, 1, 0, 2);
	}
	else
	{
		/* basic layer order is B, A, zoom, F */
		k052109_tilemap_draw(k052109, bitmap, cliprect, 1, 0, 2);
		k051316_zoom_draw(k051316, bitmap, cliprect, 0, 4);
	}
	k052109_tilemap_draw(k052109, bitmap, cliprect, 0, 0, 8);

	k051960_sprites_draw(k051960, bitmap, cliprect, -1, -1);
	return 0;
}
