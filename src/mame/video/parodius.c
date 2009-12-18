#include "driver.h"
#include "video/konicdev.h"


static int layer_colorbase[3],sprite_colorbase,bg_colorbase;
static int layerpri[3];


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void parodius_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

/***************************************************************************

  Callbacks for the K053245

***************************************************************************/

void parodius_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= layerpri[2])								*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else 												*priority_mask = 0xf0|0xcc|0xaa;

	*color = sprite_colorbase + (*color & 0x1f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

/* useful function to sort the three tile layers by priority order */
static void sortlayers(int *layer,int *pri)
{
#define SWAP(a,b) \
	if (pri[a] < pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0,1)
	SWAP(0,2)
	SWAP(1,2)
}

VIDEO_UPDATE( parodius )
{
	const device_config *k053245 = devtag_get_device(screen->machine, "k053245");
	const device_config *k053251 = devtag_get_device(screen->machine, "k053251");
	const device_config *k052109 = devtag_get_device(screen->machine, "k052109");
	int layer[3];

	bg_colorbase       = k053251_get_palette_index(k053251, K053251_CI0);
	sprite_colorbase   = k053251_get_palette_index(k053251, K053251_CI1);
	layer_colorbase[0] = k053251_get_palette_index(k053251, K053251_CI2);
	layer_colorbase[1] = k053251_get_palette_index(k053251, K053251_CI4);
	layer_colorbase[2] = k053251_get_palette_index(k053251, K053251_CI3);

	k052109_tilemap_update(k052109);

	layer[0] = 0;
	layerpri[0] = k053251_get_priority(k053251, K053251_CI2);
	layer[1] = 1;
	layerpri[1] = k053251_get_priority(k053251, K053251_CI4);
	layer[2] = 2;
	layerpri[2] = k053251_get_priority(k053251, K053251_CI3);

	sortlayers(layer, layerpri);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,16 * bg_colorbase);
	k052109_tilemap_draw(k052109, bitmap, cliprect, layer[0], 0,1);
	k052109_tilemap_draw(k052109, bitmap, cliprect, layer[1], 0,2);
	k052109_tilemap_draw(k052109, bitmap, cliprect, layer[2], 0,4);

	k053245_sprites_draw(k053245, bitmap, cliprect);
	return 0;
}
