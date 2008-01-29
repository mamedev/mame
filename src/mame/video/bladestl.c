#include "driver.h"
#include "deprecat.h"
#include "video/konamiic.h"

static int layer_colorbase[2];
extern int bladestl_spritebank;


WRITE8_HANDLER( bladestl_palette_ram_w )
{
	paletteram_xBBBBBGGGGGRRRRR_be_w(offset, data);

	/* if it's a sprite color, modify the pens that reference this color */
	if (offset >= 0x40)
	{
		int i;

		UINT8 *lookup_prom = memory_region(REGION_PROMS);

		for (i = 0; i < 0x100; i++)
			if ((lookup_prom[i] & 0x0f) == ((offset >> 1) & 0x0f))
				palette_set_color(Machine, i + 0x30, palette_get_color(Machine, offset >> 1));
	}
}


/***************************************************************************

  Callback for the K007342

***************************************************************************/

static void tile_callback(int layer, int bank, int *code, int *color, int *flags)
{
	*code |= ((*color & 0x0f) << 8) | ((*color & 0x40) << 6);
	*color = layer_colorbase[layer];
}

/***************************************************************************

  Callback for the K007420

***************************************************************************/

static void sprite_callback(int *code,int *color)
{
	*code |= ((*color & 0xc0) << 2) + bladestl_spritebank;
	*code = (*code << 2) | ((*color & 0x30) >> 4);
	*color = 0 + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( bladestl )
{
	layer_colorbase[0] = 0;
	layer_colorbase[1] = 1;

	K007342_vh_start(0,tile_callback);
	K007420_vh_start(machine,1,sprite_callback);
}

/***************************************************************************

  Screen Refresh

***************************************************************************/

VIDEO_UPDATE( bladestl )
{
	K007342_tilemap_update();

	K007342_tilemap_draw( bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE ,0);
	K007420_sprites_draw( bitmap, cliprect );
	K007342_tilemap_draw( bitmap, cliprect, 1, 1 | TILEMAP_DRAW_OPAQUE ,0);
	K007342_tilemap_draw( bitmap, cliprect, 0, 0 ,0);
	K007342_tilemap_draw( bitmap, cliprect, 0, 1 ,0);
	return 0;
}
