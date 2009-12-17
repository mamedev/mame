#include "driver.h"
#include "video/konicdev.h"


static int layer_colorbase[3],sprite_colorbase;

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void aliens_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color, int *flags, int *priority)
{
	*code |= ((*color & 0x3f) << 8) | (bank << 14);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void aliens_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow)
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
	machine->generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x400);

	layer_colorbase[0] = 0;
	layer_colorbase[1] = 4;
	layer_colorbase[2] = 8;
	sprite_colorbase = 16;
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( aliens )
{
	const device_config *k052109 = devtag_get_device(screen->machine, "k052109");
	const device_config *k051960 = devtag_get_device(screen->machine, "k051960");

	k052109_tilemap_update(k052109);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,layer_colorbase[1] * 16);

	k052109_tilemap_draw(k052109, bitmap, cliprect, 1, 0,1);
	k052109_tilemap_draw(k052109, bitmap, cliprect, 2, 0,2);
	k052109_tilemap_draw(k052109, bitmap, cliprect, 0, 0,4);

	k051960_sprites_draw(k051960, bitmap,cliprect,-1,-1);
	return 0;
}
