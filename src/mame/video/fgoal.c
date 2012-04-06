/***************************************************************************

    Taito Field Goal video emulation

***************************************************************************/

#include "emu.h"
#include "includes/fgoal.h"


WRITE8_MEMBER(fgoal_state::fgoal_color_w)
{
	m_current_color = data & 3;
}


WRITE8_MEMBER(fgoal_state::fgoal_ypos_w)
{
	m_ypos = data;
}


WRITE8_MEMBER(fgoal_state::fgoal_xpos_w)
{
	m_xpos = data;
}


VIDEO_START( fgoal )
{
	fgoal_state *state = machine.driver_data<fgoal_state>();
	machine.primary_screen->register_screen_bitmap(state->m_fgbitmap);
	machine.primary_screen->register_screen_bitmap(state->m_bgbitmap);

	state->save_item(NAME(state->m_fgbitmap));
	state->save_item(NAME(state->m_bgbitmap));
}


SCREEN_UPDATE_IND16( fgoal )
{
	fgoal_state *state = screen.machine().driver_data<fgoal_state>();
	const UINT8* VRAM = state->m_video_ram;

	int x;
	int y;
	int n;

	/* draw color overlay foreground and background */

	if (state->m_fgoal_player == 1 && (input_port_read(screen.machine(), "IN1") & 0x40))
	{
		drawgfxzoom_opaque(state->m_fgbitmap, cliprect, screen.machine().gfx[0],
			0, (state->m_fgoal_player << 2) | state->m_current_color,
			1, 1,
			0, 16,
			0x40000,
			0x40000);

		drawgfxzoom_opaque(state->m_bgbitmap, cliprect, screen.machine().gfx[1],
			0, 0,
			1, 1,
			0, 16,
			0x40000,
			0x40000);
	}
	else
	{
		drawgfxzoom_opaque(state->m_fgbitmap, cliprect, screen.machine().gfx[0],
			0, (state->m_fgoal_player << 2) | state->m_current_color,
			0, 0,
			0, 0,
			0x40000,
			0x40000);

		drawgfxzoom_opaque(state->m_bgbitmap, cliprect, screen.machine().gfx[1],
			0, 0,
			0, 0,
			0, 0,
			0x40000,
			0x40000);
	}

	/* the ball has a fixed color */

	for (y = state->m_ypos; y < state->m_ypos + 8; y++)
	{
		for (x = state->m_xpos; x < state->m_xpos + 8; x++)
		{
			if (y < 256 && x < 256)
			{
				state->m_fgbitmap.pix16(y, x) = 128 + 16;
			}
		}
	}

	/* draw bitmap layer */

	for (y = 0; y < 256; y++)
	{
		UINT16* p = &bitmap.pix16(y);

		const UINT16* FG = &state->m_fgbitmap.pix16(y);
		const UINT16* BG = &state->m_bgbitmap.pix16(y);

		for (x = 0; x < 256; x += 8)
		{
			UINT8 v = *VRAM++;

			for (n = 0; n < 8; n++)
			{
				if (v & (1 << n))
				{
					p[x + n] = FG[x + n];
				}
				else
				{
					p[x + n] = BG[x + n];
				}
			}
		}
	}
	return 0;
}
