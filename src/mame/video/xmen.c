#include "driver.h"
#include "video/konicdev.h"
#include "includes/xmen.h"


static int layer_colorbase[3],sprite_colorbase,bg_colorbase;
static int layerpri[3];


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void xmen_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	/* (color & 0x02) is flip y handled internally by the 052109 */
	if (layer == 0)
		*color = layer_colorbase[layer] + ((*color & 0xf0) >> 4);
	else
		*color = layer_colorbase[layer] + ((*color & 0x7c) >> 2);
}

/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

void xmen_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask)
{
	int pri = (*color & 0x00e0) >> 4;	/* ??????? */
	if (pri <= layerpri[2])								*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else												*priority_mask = 0xf0|0xcc|0xaa;

	*color = sprite_colorbase + (*color & 0x001f);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static bitmap_t * screen_right;
static bitmap_t * screen_left;
static UINT16 *K053247_ram;

VIDEO_START( xmen6p )
{
	const device_config *k053246 = devtag_get_device(machine, "k053246");

	k053247_get_ram(k053246, &K053247_ram);

	screen_left = auto_bitmap_alloc(machine, 64*8, 32*8, BITMAP_FORMAT_INDEXED16);
	screen_right = auto_bitmap_alloc(machine, 64*8, 32*8, BITMAP_FORMAT_INDEXED16);
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


VIDEO_UPDATE( xmen )
{
	const device_config *k052109 = devtag_get_device(screen->machine, "k052109");
	const device_config *k053246 = devtag_get_device(screen->machine, "k053246");
	const device_config *k053251 = devtag_get_device(screen->machine, "k053251");
	int layer[3];

	bg_colorbase       = k053251_get_palette_index(k053251, K053251_CI4);
	sprite_colorbase   = k053251_get_palette_index(k053251, K053251_CI1);
	layer_colorbase[0] = k053251_get_palette_index(k053251, K053251_CI3);
	layer_colorbase[1] = k053251_get_palette_index(k053251, K053251_CI0);
	layer_colorbase[2] = k053251_get_palette_index(k053251, K053251_CI2);

	k052109_tilemap_update(k052109);

	layer[0] = 0;
	layerpri[0] = k053251_get_priority(k053251, K053251_CI3);
	layer[1] = 1;
	layerpri[1] = k053251_get_priority(k053251, K053251_CI0);
	layer[2] = 2;
	layerpri[2] = k053251_get_priority(k053251, K053251_CI2);

	sortlayers(layer,layerpri);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	/* note the '+1' in the background color!!! */
	bitmap_fill(bitmap,cliprect,16 * bg_colorbase+1);
	k052109_tilemap_draw(k052109, bitmap, cliprect, layer[0], 0, 1);
	k052109_tilemap_draw(k052109, bitmap, cliprect, layer[1], 0, 2);
	k052109_tilemap_draw(k052109, bitmap, cliprect, layer[2], 0, 4);

/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
    pdrawgfx_shadow_lowpri = 1; fix shadows of boulders in front of feet */
	k053247_sprites_draw(k053246, bitmap, cliprect);
	return 0;
}


VIDEO_UPDATE( xmen6p )
{
	const device_config *left_screen   = devtag_get_device(screen->machine, "lscreen");
	const device_config *right_screen  = devtag_get_device(screen->machine, "rscreen");
	int x,y;

	if (screen == left_screen)
		for(y = 0; y < 32 * 8; y++)
		{
			UINT16* line_dest = BITMAP_ADDR16(bitmap, y, 0);
			UINT16* line_src = BITMAP_ADDR16(screen_left, y, 0);

			for (x = 12 * 8; x < 52 * 8; x++)
				line_dest[x] = line_src[x];
		}
	else if (screen == right_screen)
		for(y = 0; y < 32 * 8; y++)
		{
			UINT16* line_dest = BITMAP_ADDR16(bitmap, y, 0);
			UINT16* line_src = BITMAP_ADDR16(screen_right, y, 0);

			for (x = 12 * 8; x < 52 * 8; x++)
				line_dest[x] = line_src[x];
		}

	return 0;
}

/* my lefts and rights are mixed up in several places.. */
VIDEO_EOF( xmen6p )
{
	const device_config *k052109 = devtag_get_device(machine, "k052109");
	const device_config *k053246 = devtag_get_device(machine, "k053246");
	const device_config *k053251 = devtag_get_device(machine, "k053251");
	int layer[3];
	bitmap_t * renderbitmap;
	rectangle cliprect;
	int offset;

	xmen_current_frame ^= 0x01;

//  const rectangle *visarea = video_screen_get_visible_area(machine->primary_screen);
//  cliprect.min_x = visarea->min_x;
//  cliprect.max_x = visarea->max_x;
//  cliprect.min_y = visarea->min_y;
//  cliprect.max_y = visarea->max_y;

	cliprect.min_x = 0;
	cliprect.max_x = 64 * 8 - 1;
	cliprect.min_y = 2 * 8;
	cliprect.max_y = 30 * 8 - 1;


	if (xmen_current_frame & 0x01)
	{

		/* copy the desired spritelist to the chip */
		memcpy(K053247_ram, xmen6p_spriteramright, 0x1000);

		/* we write the entire content of the tileram to the chip to ensure
           everything gets marked as dirty and the desired tilemap is rendered
           this is not very efficient!
           */
		for (offset = 0; offset < (0xc000 / 2); offset++)
		{
//          K052109_lsb_w
			k052109_w(k052109, offset, xmen6p_tilemapright[offset] & 0x00ff);
		}


		renderbitmap = screen_right;
	}
	else
	{
		/* copy the desired spritelist to the chip */
		memcpy(K053247_ram, xmen6p_spriteramleft, 0x1000);

		/* we write the entire content of the tileram to the chip to ensure
           everything gets marked as dirty and the desired tilemap is rendered

           this is not very efficient!
           */
		for (offset = 0; offset < (0xc000 / 2); offset++)
		{
//          K052109_lsb_w
			k052109_w(k052109, offset, xmen6p_tilemapleft[offset] & 0x00ff);
		}


		renderbitmap = screen_left;
	}


	bg_colorbase       = k053251_get_palette_index(k053251, K053251_CI4);
	sprite_colorbase   = k053251_get_palette_index(k053251, K053251_CI1);
	layer_colorbase[0] = k053251_get_palette_index(k053251, K053251_CI3);
	layer_colorbase[1] = k053251_get_palette_index(k053251, K053251_CI0);
	layer_colorbase[2] = k053251_get_palette_index(k053251, K053251_CI2);

	k052109_tilemap_update(k052109);

	layer[0] = 0;
	layerpri[0] = k053251_get_priority(k053251, K053251_CI3);
	layer[1] = 1;
	layerpri[1] = k053251_get_priority(k053251, K053251_CI0);
	layer[2] = 2;
	layerpri[2] = k053251_get_priority(k053251, K053251_CI2);

	sortlayers(layer,layerpri);

	bitmap_fill(machine->priority_bitmap, &cliprect, 0);
	/* note the '+1' in the background color!!! */
	bitmap_fill(renderbitmap, &cliprect, 16 * bg_colorbase + 1);
	k052109_tilemap_draw(k052109, renderbitmap, &cliprect, layer[0], 0, 1);
	k052109_tilemap_draw(k052109, renderbitmap, &cliprect, layer[1], 0, 2);
	k052109_tilemap_draw(k052109, renderbitmap, &cliprect, layer[2], 0, 4);

/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
    pdrawgfx_shadow_lowpri = 1; fix shadows of boulders in front of feet */
	k053247_sprites_draw(k053246, renderbitmap, &cliprect);
}
