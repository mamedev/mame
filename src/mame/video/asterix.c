#include "driver.h"
#include "video/konicdev.h"


static int sprite_colorbase;
static int layer_colorbase[4], layerpri[3];
static UINT16 spritebank;
static int tilebanks[4];
static int spritebanks[4];

static void reset_spritebank( running_machine *machine )
{
	const device_config *k053244 = devtag_get_device(machine, "k053244");
	k053244_bankselect(k053244, spritebank & 7);
	spritebanks[0] = (spritebank << 12) & 0x7000;
	spritebanks[1] = (spritebank <<  9) & 0x7000;
	spritebanks[2] = (spritebank <<  6) & 0x7000;
	spritebanks[3] = (spritebank <<  3) & 0x7000;
}

WRITE16_HANDLER( asterix_spritebank_w )
{
	COMBINE_DATA(&spritebank);
	reset_spritebank(space->machine);
}

void asterix_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask)
{
	int pri = (*color & 0x00e0) >> 2;
	if (pri <= layerpri[2])								*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else												*priority_mask = 0xf0|0xcc|0xaa;
	*color = sprite_colorbase | (*color & 0x001f);
	*code = (*code & 0xfff) | spritebanks[(*code >> 12) & 3];
}


void asterix_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags)
{
	*flags = *code & 0x1000 ? TILE_FLIPX : 0;
	*color = (layer_colorbase[layer] + ((*code & 0xe000) >> 13)) & 0x7f;
	*code = (*code & 0x03ff) | tilebanks[(*code >> 10) & 3];
}

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

VIDEO_UPDATE( asterix )
{
	static const int K053251_CI[4] = { K053251_CI0, K053251_CI2, K053251_CI3, K053251_CI4 };
	const device_config *k053244 = devtag_get_device(screen->machine, "k053244");
	const device_config *k056832 = devtag_get_device(screen->machine, "k056832");
	const device_config *k053251 = devtag_get_device(screen->machine, "k053251");
	int layer[3], plane, new_colorbase;

	/* Layer offsets are different if horizontally flipped */
	if (k056832_read_register(k056832, 0x0) & 0x10)
	{
		k056832_set_layer_offs(k056832, 0, 89 - 176, 0);
		k056832_set_layer_offs(k056832, 1, 91 - 176, 0);
		k056832_set_layer_offs(k056832, 2, 89 - 176, 0);
		k056832_set_layer_offs(k056832, 3, 95 - 176, 0);
	}
	else
	{
		k056832_set_layer_offs(k056832, 0, 89, 0);
		k056832_set_layer_offs(k056832, 1, 91, 0);
		k056832_set_layer_offs(k056832, 2, 89, 0);
		k056832_set_layer_offs(k056832, 3, 95, 0);
	}


	tilebanks[0] = (k056832_get_lookup(k056832, 0) << 10);
	tilebanks[1] = (k056832_get_lookup(k056832, 1) << 10);
	tilebanks[2] = (k056832_get_lookup(k056832, 2) << 10);
	tilebanks[3] = (k056832_get_lookup(k056832, 3) << 10);

	// update color info and refresh tilemaps
	sprite_colorbase = k053251_get_palette_index(k053251, K053251_CI1);

	for (plane = 0; plane < 4; plane++)
	{
		new_colorbase = k053251_get_palette_index(k053251, K053251_CI[plane]);
		if (layer_colorbase[plane] != new_colorbase)
		{
			layer_colorbase[plane] = new_colorbase;
			k056832_mark_plane_dirty(k056832, plane);
		}
	}

	layer[0] = 0;
	layerpri[0] = k053251_get_priority(k053251, K053251_CI0);
	layer[1] = 1;
	layerpri[1] = k053251_get_priority(k053251, K053251_CI2);
	layer[2] = 3;
	layerpri[2] = k053251_get_priority(k053251, K053251_CI4);

	sortlayers(layer, layerpri);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 0);

	k056832_tilemap_draw(k056832, bitmap, cliprect, layer[0], 0, 1);
	k056832_tilemap_draw(k056832, bitmap, cliprect, layer[1], 0, 2);
	k056832_tilemap_draw(k056832, bitmap, cliprect, layer[2], 0, 4);

/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
    pdrawgfx_shadow_lowpri = 1; fix shadows in front of feet */
	k053245_sprites_draw(k053244, bitmap, cliprect);

	k056832_tilemap_draw(k056832, bitmap, cliprect, 2, 0, 0);
	return 0;
}
