// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/*** Video *******************************************************************/
/* see drivers/pgm.c for notes on where improvements can be made */

#include "emu.h"
#include "pgm.h"


/*** Video - Start / Update ****************************************************/

void pgm_state::video_start()
{
}

u32 pgm_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_video->screen_update(screen, bitmap, cliprect);
}

void pgm_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		/* first 0xa00 of main ram = sprites, seems to be buffered, DMA? */
		m_video->get_sprites(m_mainram);

		// vblank start interrupt
		m_maincpu->set_input_line(M68K_IRQ_6, HOLD_LINE);
	}
}
