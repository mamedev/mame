// license:BSD-3-Clause
// copyright-holders:Martin Buchholz
/***************************************************************************

  Poly-Play
  (c) 1985 by VEB Polytechnik Karl-Marx-Stadt

  sound hardware

  driver written by Martin Buchholz (buchholz@mail.uni-greifswald.de)

***************************************************************************/

#include "emu.h"
#include "polyplay.h"

WRITE_LINE_MEMBER(polyplay_state::ctc_zc0_w)
{
	if (state)
	{
		m_flipflop1 = ~m_flipflop1;
		m_speaker1->level_w(m_flipflop1);
	}
}

WRITE_LINE_MEMBER(polyplay_state::ctc_zc1_w)
{
	if (state)
	{
		m_flipflop2 = ~m_flipflop2;
		m_speaker2->level_w(m_flipflop2);
	}
}
