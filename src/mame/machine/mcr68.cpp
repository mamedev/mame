// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Bryan McPhail
/***************************************************************************

    Midway MCR-68k system

***************************************************************************/

#include "emu.h"
#include "audio/midway.h"
#include "includes/mcr68.h"

#define VERBOSE 0



/*************************************
 *
 *  Generic MCR/68k machine initialization
 *
 *************************************/

void mcr68_state::machine_start()
{   /* for the most part all MCR/68k games are the same */
	m_493_callback.init(*this, FUNC(mcr68_state::mcr68_493_callback));
	m_493_off_callback.init(*this, FUNC(mcr68_state::mcr68_493_off_callback));
}



/*************************************
 *
 *  Scanline callback
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER( mcr68_state::scanline_cb )
{
	// VSYNC
	if (param == 0)
	{
		if (VERBOSE)
			logerror("--- VBLANK ---\n");

		m_ptm->set_c1(0);
		m_ptm->set_c1(1);

		/* also set a timer to generate the 493 signal at a specific time before the next VBLANK */
		/* the timing of this is crucial for Blasted and Tri-Sports, which check the timing of */
		/* VBLANK and 493 using counter 2 */
		m_493_callback.call_after(attotime::from_hz(30) - m_timing_factor);
	}

	// HSYNC
	m_ptm->set_c3(0);
	m_ptm->set_c3(1);
}



/*************************************
 *
 *  MCR/68k interrupt central
 *
 *************************************/

TIMER_CALLBACK_MEMBER(mcr68_state::mcr68_493_off_callback)
{
	m_maincpu->set_input_line(1, CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(mcr68_state::mcr68_493_callback)
{
	m_maincpu->set_input_line(1, ASSERT_LINE);
	m_493_off_callback.call_after(m_screen->scan_period());

	if (VERBOSE)
		logerror("--- (INT1) ---\n");
}
