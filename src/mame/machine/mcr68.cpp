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

MACHINE_START_MEMBER(mcr68_state,mcr68)
{
}


MACHINE_RESET_MEMBER(mcr68_state,mcr68)
{
	/* for the most part all MCR/68k games are the same */
	m_v493_callback = timer_expired_delegate(FUNC(mcr68_state::mcr68_493_callback),this);
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
		machine().scheduler().timer_set(attotime::from_hz(30) - m_timing_factor, m_v493_callback);
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
	machine().scheduler().timer_set(m_screen->scan_period(), timer_expired_delegate(FUNC(mcr68_state::mcr68_493_off_callback),this));

	if (VERBOSE)
		logerror("--- (INT1) ---\n");
}
