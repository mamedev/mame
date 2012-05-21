/***************************************************************************

  Poly-Play
  (c) 1985 by VEB Polytechnik Karl-Marx-Stadt

  video hardware

  driver written by Martin Buchholz (buchholz@mail.uni-greifswald.de)

***************************************************************************/

#include "emu.h"
#include "includes/polyplay.h"


PALETTE_INIT( polyplay )
{
	palette_set_color(machine,0,MAKE_RGB(0x00,0x00,0x00));
	palette_set_color(machine,1,MAKE_RGB(0xff,0xff,0xff));

	palette_set_color(machine,2,MAKE_RGB(0x00,0x00,0x00));
	palette_set_color(machine,3,MAKE_RGB(0xff,0x00,0x00));
	palette_set_color(machine,4,MAKE_RGB(0x00,0xff,0x00));
	palette_set_color(machine,5,MAKE_RGB(0xff,0xff,0x00));
	palette_set_color(machine,6,MAKE_RGB(0x00,0x00,0xff));
	palette_set_color(machine,7,MAKE_RGB(0xff,0x00,0xff));
	palette_set_color(machine,8,MAKE_RGB(0x00,0xff,0xff));
	palette_set_color(machine,9,MAKE_RGB(0xff,0xff,0xff));
}


WRITE8_MEMBER(polyplay_state::polyplay_characterram_w)
{
	if (m_characterram[offset] != data)
	{
		gfx_element_mark_dirty(machine().gfx[1], (offset >> 3) & 0x7f);

		m_characterram[offset] = data;
	}
}

VIDEO_START( polyplay )
{
	polyplay_state *state = machine.driver_data<polyplay_state>();
	gfx_element_set_source(machine.gfx[1], state->m_characterram);
}


SCREEN_UPDATE_IND16( polyplay )
{
	polyplay_state *state = screen.machine().driver_data<polyplay_state>();
	UINT8 *videoram = state->m_videoram;
	offs_t offs;


	for (offs = 0; offs < 0x800; offs++)
	{
		int sx = (offs & 0x3f) << 3;
		int sy = offs >> 6 << 3;
		UINT8 code = videoram[offs];

		drawgfx_opaque(bitmap,cliprect, screen.machine().gfx[(code >> 7) & 0x01],
				code, 0, 0, 0, sx, sy);
	}

	return 0;
}
