/***************************************************************************

  video.c

***************************************************************************/

#include "driver.h"
#include "video/konicdev.h"

int qdrmfgp_get_palette(void);



void qdrmfgp_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags)
{
	*color = ((*color>>2) & 0x0f) | qdrmfgp_get_palette();
}

void qdrmfgp2_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags)
{
	*color = (*color>>1) & 0x7f;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( qdrmfgp )
{
	const device_config *k056832 = devtag_get_device(machine, "k056832");

	k056832_set_layer_association(k056832, 0);

	k056832_set_layer_offs(k056832, 0, 2, 0);
	k056832_set_layer_offs(k056832, 1, 4, 0);
	k056832_set_layer_offs(k056832, 2, 6, 0);
	k056832_set_layer_offs(k056832, 3, 8, 0);
}

VIDEO_START( qdrmfgp2 )
{
	const device_config *k056832 = devtag_get_device(machine, "k056832");

	k056832_set_layer_association(k056832, 0);

	k056832_set_layer_offs(k056832, 0, 3, 1);
	k056832_set_layer_offs(k056832, 1, 5, 1);
	k056832_set_layer_offs(k056832, 2, 7, 1);
	k056832_set_layer_offs(k056832, 3, 9, 1);
}

/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( qdrmfgp )
{
	const device_config *k056832 = devtag_get_device(screen->machine, "k056832");
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	k056832_tilemap_draw(k056832, bitmap, cliprect, 3, 0, 1);
	k056832_tilemap_draw(k056832, bitmap, cliprect, 2, 0, 2);
	k056832_tilemap_draw(k056832, bitmap, cliprect, 1, 0, 4);
	k056832_tilemap_draw(k056832, bitmap, cliprect, 0, 0, 8);
	return 0;
}
