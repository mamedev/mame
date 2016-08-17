// license:BSD-3-Clause
// copyright-holders:Andrew Gardner,Aaron Giles
/***************************************************************************

    okiadpcm.h

    OKI ADCPM emulation.

***************************************************************************/

#include "emu.h"
#include "okiadpcm.h"


//**************************************************************************
//  ADPCM STATE HELPER
//**************************************************************************

// ADPCM state and tables
bool oki_adpcm_state::s_tables_computed = false;
const INT8 oki_adpcm_state::s_index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };
int oki_adpcm_state::s_diff_lookup[49*16];

//-------------------------------------------------
//  reset - reset the ADPCM state
//-------------------------------------------------

void oki_adpcm_state::reset()
{
	// reset the signal/step
	m_signal = -2;
	m_step = 0;
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

INT16 oki_adpcm_state::clock(UINT8 nibble)
{
	// update the signal
	m_signal += s_diff_lookup[m_step * 16 + (nibble & 15)];

	// clamp to the maximum
	if (m_signal > 2047)
		m_signal = 2047;
	else if (m_signal < -2048)
		m_signal = -2048;

	// adjust the step size and clamp
	m_step += s_index_shift[nibble & 7];
	if (m_step > 48)
		m_step = 48;
	else if (m_step < 0)
		m_step = 0;

	// return the signal
	return m_signal;
}


//-------------------------------------------------
//  compute_tables - precompute tables for faster
//  sound generation
//-------------------------------------------------

void oki_adpcm_state::compute_tables()
{
	// skip if we already did it
	if (s_tables_computed)
		return;
	s_tables_computed = true;

	// nibble to bit map
	static const INT8 nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	// loop over all possible steps
	for (int step = 0; step <= 48; step++)
	{
		// compute the step value
		int stepval = floor(16.0 * pow(11.0 / 10.0, (double)step));

		// loop over all nibbles and compute the difference
		for (int nib = 0; nib < 16; nib++)
		{
			s_diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
					stepval/2 * nbl2bit[nib][2] +
					stepval/4 * nbl2bit[nib][3] +
					stepval/8);
		}
	}
}


//**************************************************************************
//  ADPCM2 STATE HELPER
//**************************************************************************

// ADPCM state and tables
bool oki_adpcm2_state::s_tables_computed = false;
const INT8 oki_adpcm2_state::s_index_shift[8] = { -2, -2, -2, -2, 2, 6, 9, 11 };
int oki_adpcm2_state::s_diff_lookup[49*16];

//-------------------------------------------------
//  reset - reset the ADPCM state
//-------------------------------------------------

void oki_adpcm2_state::reset()
{
	// reset the signal/step
	m_signal = -2;
	m_step = 0;
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

INT16 oki_adpcm2_state::clock(UINT8 nibble)
{
	// update the signal
	m_signal += s_diff_lookup[m_step * 16 + (nibble & 15)];

	// clamp to the maximum
	if (m_signal > 2047)
		m_signal = 2047;
	else if (m_signal < -2048)
		m_signal = -2048;

	// adjust the step size and clamp
	m_step += s_index_shift[nibble & 7];
	if (m_step > 48)
		m_step = 48;
	else if (m_step < 0)
		m_step = 0;

	// return the signal
	return m_signal;
}


//-------------------------------------------------
//  compute_tables - precompute tables for faster
//  sound generation
//-------------------------------------------------

void oki_adpcm2_state::compute_tables()
{
	// skip if we already did it
	if (s_tables_computed)
		return;
	s_tables_computed = true;

	// nibble to bit map
	static const INT8 nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	// loop over all possible steps
	float floatstep = 64;
	for (int step = 0; step <= 48; step++)
	{
		// compute the step value
		int stepval = floor(floatstep * 1.08f);
		floatstep = floatstep * 1.08f;

		// loop over all nibbles and compute the difference
		for (int nib = 0; nib < 16; nib++)
		{
			s_diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
					stepval/2 * nbl2bit[nib][2] +
					stepval/4 * nbl2bit[nib][3] +
					stepval/8);
		}
	}
}
