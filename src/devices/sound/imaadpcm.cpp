// license:BSD-3-Clause
// copyright-holders:Andrew Gardner,Aaron Giles
/***************************************************************************

    imaadpcm.cpp

    IMA ADPCM emulation.

***************************************************************************/

#include "emu.h"
#include "imaadpcm.h"


//**************************************************************************
//  ADPCM STATE HELPER
//**************************************************************************

// ADPCM state and tables
bool ima_adpcm_state::s_tables_computed = false;
const int8_t ima_adpcm_state::s_index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };
int ima_adpcm_state::s_diff_lookup[89*16];

//-------------------------------------------------
//  reset - reset the ADPCM state
//-------------------------------------------------

void ima_adpcm_state::reset()
{
	// reset the signal/step
	m_signal = m_loop_signal = 0;
	m_step = m_loop_step = 0;
	m_saved = false;
}


//-------------------------------------------------
//  clock - decode single nibble and update
//  ADPCM output
//-------------------------------------------------

int16_t ima_adpcm_state::clock(uint8_t nibble)
{
	// update the signal
	m_signal += s_diff_lookup[m_step * 16 + (nibble & 15)];

	// clamp to the maximumf
	if (m_signal > 32767)
		m_signal = 32767;
	else if (m_signal < -32768)
		m_signal = -32768;

	// adjust the step size and clamp
	m_step += s_index_shift[nibble & 7];
	if (m_step > 88)
		m_step = 88;
	else if (m_step < 0)
		m_step = 0;

	// return the signal
	return m_signal;
}


//-------------------------------------------------
//  save - save current ADPCM state to buffer
//-------------------------------------------------

void ima_adpcm_state::save()
{
	if (!m_saved)
	{
		m_loop_signal = m_signal;
		m_loop_step = m_step;
		m_saved = true;
	}
}


//-------------------------------------------------
//  restore - restore previous ADPCM state
//  from buffer
//-------------------------------------------------

void ima_adpcm_state::restore()
{
	m_signal = m_loop_signal;
	m_step = m_loop_step;
}


//-------------------------------------------------
//  compute_tables - precompute tables for faster
//  sound generation
//-------------------------------------------------

void ima_adpcm_state::compute_tables()
{
	// skip if we already did it
	if (s_tables_computed)
		return;
	s_tables_computed = true;

	// nibble to bit map
	static const int8_t nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	// loop over all possible steps
	for (int step = -8; step <= 80; step++)
	{
		// compute the step value
		int stepval = std::min(floor(16.0 * pow(11.0 / 10.0, (double)step)), 32767.);

		// manual correction of some early values
		if (step == -5 || step == -4)
			stepval++;

		// loop over all nibbles and compute the difference
		for (int nib = 0; nib < 16; nib++)
		{
			s_diff_lookup[(step + 8)*16 + nib] = nbl2bit[nib][0] *
					(stepval * nbl2bit[nib][1] +
					stepval/2 * nbl2bit[nib][2] +
					stepval/4 * nbl2bit[nib][3] +
					stepval/8);
		}
	}
}
