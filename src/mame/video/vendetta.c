#include "driver.h"
#include "video/konicdev.h"


static int layer_colorbase[3],bg_colorbase,sprite_colorbase;
static int layerpri[3];


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void vendetta_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x30) << 6) |
			((*color & 0x0c) << 10) | (bank << 14);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}

void esckids_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) |
			((*color & 0x0c) <<  9) | (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >>  5);
}


/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

void vendetta_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask)
{
	int pri = (*color & 0x03e0) >> 4;	/* ??????? */
	if (pri <= layerpri[2])								*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else 												*priority_mask = 0xf0|0xcc|0xaa;

	*color = sprite_colorbase + (*color & 0x001f);
}


/***************************************************************************

  Display refresh

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

VIDEO_UPDATE( vendetta )
{
	const device_config *k053246 = devtag_get_device(screen->machine, "k053246");
	const device_config *k053251 = devtag_get_device(screen->machine, "k053251");
	const device_config *k052109 = devtag_get_device(screen->machine, "k052109");
	int layer[3];


	bg_colorbase       = k053251_get_palette_index(k053251, K053251_CI0);
	sprite_colorbase   = k053251_get_palette_index(k053251, K053251_CI1);
	layer_colorbase[0] = k053251_get_palette_index(k053251, K053251_CI2);
	layer_colorbase[1] = k053251_get_palette_index(k053251, K053251_CI3);
	layer_colorbase[2] = k053251_get_palette_index(k053251, K053251_CI4);

	k052109_tilemap_update(k052109);

	layer[0] = 0;
	layerpri[0] = k053251_get_priority(k053251, K053251_CI2);
	layer[1] = 1;
	layerpri[1] = k053251_get_priority(k053251, K053251_CI3);
	layer[2] = 2;
	layerpri[2] = k053251_get_priority(k053251, K053251_CI4);

	sortlayers(layer,layerpri);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	k052109_tilemap_draw(k052109, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	k052109_tilemap_draw(k052109, bitmap, cliprect, layer[1], 0, 2);
	k052109_tilemap_draw(k052109, bitmap, cliprect, layer[2], 0, 4);

	k053247_sprites_draw(k053246, bitmap, cliprect);
	return 0;
}
