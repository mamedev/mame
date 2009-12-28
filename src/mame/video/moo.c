/***************************************************************************

 Wild West C.O.W.boys of Moo Mesa
 Bucky O'Hare
 (c) 1992 Konami

 Video hardware emulation.

***************************************************************************/

#include "driver.h"
#include "video/konicdev.h"

static int sprite_colorbase;
static int layer_colorbase[4], layerpri[3];
static int alpha_enabled;

#ifdef UNUSED_FUNCTION
void moo_set_alpha(int on)
{
	alpha_enabled = on;
}
#endif

void moo_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask)
{
	int pri = (*color & 0x03e0) >> 4;

	if      (pri <= layerpri[2])	*priority_mask = 0;
	else if (pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else							*priority_mask = 0xf0|0xcc|0xaa;

	*color = sprite_colorbase | (*color & 0x001f);
}

void moo_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags)
{
	*color = layer_colorbase[layer] | (*color>>2 & 0x0f);
}

VIDEO_START(moo)
{
	const device_config *k056832 = devtag_get_device(machine, "k056832");
	assert(video_screen_get_format(machine->primary_screen) == BITMAP_FORMAT_RGB32);

	alpha_enabled = 0;

	if (!strcmp(machine->gamedrv->name, "bucky") || !strcmp(machine->gamedrv->name, "buckyua") || !strcmp(machine->gamedrv->name, "buckyaa"))
	{
		// Bucky doesn't chain tilemaps
		k056832_set_layer_association(k056832, 0);

		k056832_set_layer_offs(k056832, 0, -2, 0);
		k056832_set_layer_offs(k056832, 1,  2, 0);
		k056832_set_layer_offs(k056832, 2,  4, 0);
		k056832_set_layer_offs(k056832, 3,  6, 0);
	}
	else
	{
		// other than the intro showing one blank line alignment is good through the game
		k056832_set_layer_offs(k056832, 0, -2+1, 0);
		k056832_set_layer_offs(k056832, 1,  2+1, 0);
		k056832_set_layer_offs(k056832, 2,  4+1, 0);
		k056832_set_layer_offs(k056832, 3,  6+1, 0);
	}
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

VIDEO_UPDATE(moo)
{
	const device_config *k053246 = devtag_get_device(screen->machine, "k053246");
	const device_config *k056832 = devtag_get_device(screen->machine, "k056832");
	const device_config *k053251 = devtag_get_device(screen->machine, "k053251");
	const device_config *k054338 = devtag_get_device(screen->machine, "k054338");
	static const int K053251_CI[4] = { K053251_CI1, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layers[3];
	int bg_colorbase, new_colorbase, plane, dirty, alpha;

	bg_colorbase       = k053251_get_palette_index(k053251, K053251_CI1);
	sprite_colorbase   = k053251_get_palette_index(k053251, K053251_CI0);
	layer_colorbase[0] = 0x70;

	if (k056832_get_layer_association(k056832))
	{
		for (plane = 1; plane < 4; plane++)
		{
			new_colorbase = k053251_get_palette_index(k053251, K053251_CI[plane]);
			if (layer_colorbase[plane] != new_colorbase)
			{
				layer_colorbase[plane] = new_colorbase;
				k056832_mark_plane_dirty(k056832, plane);
			}
		}
	}
	else
	{
		for (dirty = 0, plane = 1; plane < 4; plane++)
		{
			new_colorbase = k053251_get_palette_index(k053251, K053251_CI[plane]);
			if (layer_colorbase[plane] != new_colorbase)
			{
				layer_colorbase[plane] = new_colorbase;
				dirty = 1;
			}
		}
		if (dirty)
			k056832_mark_all_tmaps_dirty(k056832);
	}

	layers[0] = 1;
	layerpri[0] = k053251_get_priority(k053251, K053251_CI2);
	layers[1] = 2;
	layerpri[1] = k053251_get_priority(k053251, K053251_CI3);
	layers[2] = 3;
	layerpri[2] = k053251_get_priority(k053251, K053251_CI4);

	sortlayers(layers, layerpri);

	k054338_update_all_shadows(k054338, 0);
	k054338_fill_backcolor(k054338, bitmap, 0);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	if (layerpri[0] < k053251_get_priority(k053251, K053251_CI1))	/* bucky hides back layer behind background */
		k056832_tilemap_draw(k056832, bitmap, cliprect, layers[0], 0, 1);

	k056832_tilemap_draw(k056832, bitmap, cliprect, layers[1], 0, 2);

	// Enabling alpha improves fog and fading in Moo but causes other things to disappear.
	// There is probably a control bit somewhere to turn off alpha blending.
	alpha_enabled = k054338_register_r(k054338, K338_REG_CONTROL) & K338_CTL_MIXPRI; // DUMMY

	alpha = (alpha_enabled) ? k054338_set_alpha_level(k054338, 1) : 255;

	if (alpha > 0)
		k056832_tilemap_draw(k056832, bitmap, cliprect, layers[2], TILEMAP_DRAW_ALPHA(alpha), 4);

	k053247_sprites_draw(k053246, bitmap, cliprect);

	k056832_tilemap_draw(k056832, bitmap, cliprect, 0, 0, 0);
	return 0;
}
