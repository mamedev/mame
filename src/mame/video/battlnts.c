#include "driver.h"
#include "video/konicdev.h"

static int spritebank;

static int layer_colorbase[2];

/***************************************************************************

  Callback for the K007342

***************************************************************************/

void battlnts_tile_callback(int layer, int bank, int *code, int *color, int *flags)
{
	*code |= ((*color & 0x0f) << 9) | ((*color & 0x40) << 2);
	*color = layer_colorbase[layer];
}

/***************************************************************************

  Callback for the K007420

***************************************************************************/

void battlnts_sprite_callback(int *code,int *color)
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
}

/***************************************************************************

  Screen Refresh

***************************************************************************/

VIDEO_UPDATE( battlnts )
{
	const device_config *k007342 = devtag_get_device(screen->machine, "k007342");
	const device_config *k007420 = devtag_get_device(screen->machine, "k007420");

	k007342_tilemap_update(k007342);

	k007342_tilemap_draw(k007342, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE ,0);
	k007420_sprites_draw(k007420, bitmap, cliprect, screen->machine->gfx[1]);
	k007342_tilemap_draw(k007342, bitmap, cliprect, 0, 1 | TILEMAP_DRAW_OPAQUE ,0);
	return 0;
}
