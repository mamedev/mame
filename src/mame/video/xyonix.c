#include "emu.h"
#include "includes/xyonix.h"

PALETTE_INIT( xyonix )
{
	int i;


	for (i = 0;i < machine.total_colors();i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 5) & 0x01;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}


static TILE_GET_INFO( get_xyonix_tile_info )
{
	xyonix_state *state = machine.driver_data<xyonix_state>();
	int tileno;
	int attr = state->m_vidram[tile_index+0x1000+1];

	tileno = (state->m_vidram[tile_index+1] << 0) | ((attr & 0x0f) << 8);

	SET_TILE_INFO(0,tileno,attr >> 4,0);
}

WRITE8_MEMBER(xyonix_state::xyonix_vidram_w)
{

	m_vidram[offset] = data;
	m_tilemap->mark_tile_dirty((offset-1)&0x0fff);
}

VIDEO_START(xyonix)
{
	xyonix_state *state = machine.driver_data<xyonix_state>();

	state->m_tilemap = tilemap_create(machine, get_xyonix_tile_info, tilemap_scan_rows, 4, 8, 80, 32);
}

SCREEN_UPDATE_IND16(xyonix)
{
	xyonix_state *state = screen.machine().driver_data<xyonix_state>();

	state->m_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
