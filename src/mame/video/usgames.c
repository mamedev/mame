#include "emu.h"
#include "includes/usgames.h"


PALETTE_INIT(usgames)
{
	int j;

	for (j = 0; j < 0x200; j++)
	{
		int data;
		int r, g, b, i;

		if (j & 0x01)
			data = (j >> 5) & 0x0f;
		else
			data = (j >> 1) & 0x0f;

		r = (data & 1) >> 0;
		g = (data & 2) >> 1;
		b = (data & 4) >> 2;
		i = (data & 8) >> 3;

		r = 0xff * r;
		g = 0x7f * g * (i + 1);
		b = 0x7f * b * (i + 1);

		palette_set_color(machine,j,MAKE_RGB(r, g, b));
	}
}



static TILE_GET_INFO( get_usgames_tile_info )
{
	usgames_state *state = machine.driver_data<usgames_state>();
	int tileno, colour;

	tileno = state->m_videoram[tile_index*2];
	colour = state->m_videoram[tile_index*2+1];

	SET_TILE_INFO(0,tileno,colour,0);
}

VIDEO_START(usgames)
{
	usgames_state *state = machine.driver_data<usgames_state>();
	state->m_tilemap = tilemap_create(machine, get_usgames_tile_info,tilemap_scan_rows, 8, 8,64,32);
	gfx_element_set_source(machine.gfx[0], state->m_charram);
}


WRITE8_HANDLER( usgames_videoram_w )
{
	usgames_state *state = space->machine().driver_data<usgames_state>();
	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_tilemap,offset/2);
}

WRITE8_HANDLER( usgames_charram_w )
{
	usgames_state *state = space->machine().driver_data<usgames_state>();
	state->m_charram[offset] = data;
	gfx_element_mark_dirty(space->machine().gfx[0], offset/8);
}


SCREEN_UPDATE(usgames)
{
	usgames_state *state = screen->machine().driver_data<usgames_state>();
	tilemap_draw(bitmap,cliprect,state->m_tilemap,0,0);
	return 0;
}
