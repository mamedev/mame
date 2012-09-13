/***************************************************************************

  video.c

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/qdrmfgp.h"



void qdrmfgp_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags)
{
	qdrmfgp_state *state = machine.driver_data<qdrmfgp_state>();
	*color = ((*color>>2) & 0x0f) | state->m_pal;
}

void qdrmfgp2_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags)
{
	*color = (*color>>1) & 0x7f;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(qdrmfgp_state,qdrmfgp)
{
	device_t *k056832 = machine().device("k056832");

	k056832_set_layer_association(k056832, 0);

	k056832_set_layer_offs(k056832, 0, 2, 0);
	k056832_set_layer_offs(k056832, 1, 4, 0);
	k056832_set_layer_offs(k056832, 2, 6, 0);
	k056832_set_layer_offs(k056832, 3, 8, 0);
}

VIDEO_START_MEMBER(qdrmfgp_state,qdrmfgp2)
{
	device_t *k056832 = machine().device("k056832");

	k056832_set_layer_association(k056832, 0);

	k056832_set_layer_offs(k056832, 0, 3, 1);
	k056832_set_layer_offs(k056832, 1, 5, 1);
	k056832_set_layer_offs(k056832, 2, 7, 1);
	k056832_set_layer_offs(k056832, 3, 9, 1);
}

/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE_IND16( qdrmfgp )
{
	device_t *k056832 = screen.machine().device("k056832");
	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	k056832_tilemap_draw(k056832, bitmap, cliprect, 3, 0, 1);
	k056832_tilemap_draw(k056832, bitmap, cliprect, 2, 0, 2);
	k056832_tilemap_draw(k056832, bitmap, cliprect, 1, 0, 4);
	k056832_tilemap_draw(k056832, bitmap, cliprect, 0, 0, 8);
	return 0;
}
