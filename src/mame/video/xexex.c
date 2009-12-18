#include "driver.h"
#include "video/konicdev.h"

static int sprite_colorbase;
static int layer_colorbase[4], layerpri[4];
static int cur_alpha;

void xexex_set_alpha(int on)
{
	cur_alpha = on;
}

void xexex_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask)
{
	int pri, c = *color;

	// Xexex doesn't seem to use bit8 and 9 as effect selectors so this should be safe.
	// (pdrawgfx() still needs change to fix Elaine's end-game graphics)
	pri = (c & 0x3e0) >> 4;

	if (pri <= layerpri[3])                       *priority_mask = 0; else
	if (pri >  layerpri[3] && pri <= layerpri[2]) *priority_mask = 0xff00; else
	if (pri >  layerpri[2] && pri <= layerpri[1]) *priority_mask = 0xff00|0xf0f0; else
	if (pri >  layerpri[1] && pri <= layerpri[0]) *priority_mask = 0xff00|0xf0f0|0xcccc; else
	*priority_mask = 0xff00|0xf0f0|0xcccc|0xaaaa;

	*color = sprite_colorbase | (c & 0x001f);
}

void xexex_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags)
{
	*color = layer_colorbase[layer] | (*color>>2 & 0x0f);
}

VIDEO_START( xexex )
{
	const device_config *k056832 = devtag_get_device(machine, "k056832");

	assert(video_screen_get_format(machine->primary_screen) == BITMAP_FORMAT_RGB32);

	cur_alpha = 0;

	// Xexex has relative plane offsets of -2,2,4,6 vs. -2,0,2,3 in MW and GX.
	k056832_set_layer_offs(k056832, 0, -2, 16);
	k056832_set_layer_offs(k056832, 1,  2, 16);
	k056832_set_layer_offs(k056832, 2,  4, 16);
	k056832_set_layer_offs(k056832, 3,  6, 16);
}

/* useful function to sort the four tile layers by priority order */
/* suboptimal, but for such a size who cares ? */
static void sortlayers(int *layer, int *pri)
{
#define SWAP(a,b) \
	if (pri[a] < pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0, 1)
	SWAP(0, 2)
	SWAP(0, 3)
	SWAP(1, 2)
	SWAP(1, 3)
	SWAP(2, 3)
}

VIDEO_UPDATE( xexex )
{
	static const int K053251_CI[4] = { K053251_CI1, K053251_CI2, K053251_CI3, K053251_CI4 };
	const device_config *k056832 = devtag_get_device(screen->machine, "k056832");
	const device_config *k053246 = devtag_get_device(screen->machine, "k053246");
	const device_config *k053251 = devtag_get_device(screen->machine, "k053251");
	const device_config *k053250 = devtag_get_device(screen->machine, "k053250");
	const device_config *k054338 = devtag_get_device(screen->machine, "k054338");
	int layer[4];
	int bg_colorbase, new_colorbase, plane, alpha;

	sprite_colorbase   = k053251_get_palette_index(k053251, K053251_CI0);
	bg_colorbase       = k053251_get_palette_index(k053251, K053251_CI1);
	layer_colorbase[0] = 0x70;

	for (plane = 1; plane < 4; plane++)
	{
		new_colorbase = k053251_get_palette_index(k053251, K053251_CI[plane]);
		if (layer_colorbase[plane] != new_colorbase)
		{
			layer_colorbase[plane] = new_colorbase;
			k056832_mark_plane_dirty(k056832, plane);
		}
	}

	layer[0] = 1;
	layerpri[0] = k053251_get_priority(k053251, K053251_CI2);
	layer[1] = 2;
	layerpri[1] = k053251_get_priority(k053251, K053251_CI3);
	layer[2] = 3;
	layerpri[2] = k053251_get_priority(k053251, K053251_CI4);
	layer[3] = -1;
	layerpri[3] = k053251_get_priority(k053251, K053251_CI1);

	sortlayers(layer, layerpri);

	k054338_update_all_shadows(k054338, 0);
	k054338_fill_backcolor(k054338, bitmap, 0);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	for (plane=0; plane<4; plane++)
	{
		if (layer[plane] < 0)
		{
			k053250_draw(k053250, bitmap, cliprect, bg_colorbase, 0, 1 << plane);
		}
		else if (!cur_alpha || layer[plane] != 1)
		{
			k056832_tilemap_draw(k056832, bitmap, cliprect, layer[plane], 0, 1 << plane);
		}
	}

	k053247_sprites_draw(k053246, bitmap, cliprect);

	if (cur_alpha)
	{
		alpha = k054338_set_alpha_level(k054338, 1);

		if (alpha > 0)
		{
			k056832_tilemap_draw(k056832, bitmap, cliprect, 1, TILEMAP_DRAW_ALPHA(alpha), 0);
		}
	}

	k056832_tilemap_draw(k056832, bitmap, cliprect, 0, 0, 0);
	return 0;
}
