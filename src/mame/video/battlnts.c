#include "driver.h"
#include "video/konamiic.h"

static int spritebank;

static int layer_colorbase[2];

/***************************************************************************

  Callback for the K007342

***************************************************************************/

static void tile_callback(int layer, int bank, int *code, int *color, int *flags)
{
	*code |= ((*color & 0x0f) << 9) | ((*color & 0x40) << 2);
	*color = layer_colorbase[layer];
}

/***************************************************************************

  Callback for the K007420

***************************************************************************/

static void sprite_callback(int *code,int *color)
{
	*code |= ((*color & 0xc0) << 2) | spritebank;
	*code = (*code << 2) | ((*color & 0x30) >> 4);
	*color = 0;
}

WRITE8_HANDLER( battlnts_spritebank_w )
{
	spritebank = 1024 * (data & 1);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( battlnts )
{
	layer_colorbase[0] = 0;
	layer_colorbase[1] = 0;

	K007342_vh_start(0,tile_callback);
	K007420_vh_start(machine,1,sprite_callback);
}

/***************************************************************************

  Screen Refresh

***************************************************************************/

VIDEO_UPDATE( battlnts ){

	K007342_tilemap_update();

	K007342_tilemap_draw( bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE ,0);
	K007420_sprites_draw( bitmap, cliprect );
	K007342_tilemap_draw( bitmap, cliprect, 0, 1 | TILEMAP_DRAW_OPAQUE ,0);
	return 0;
}
