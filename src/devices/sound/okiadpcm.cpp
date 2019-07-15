// license:BSD-3-Clause
// copyright-holders:Andrew Gardner,Aaron Giles
/***************************************************************************

    okiadpcm.h

    OKI ADPCM emulation.

    See the following patents/applications:
    (Note: if not registered, the application below was not actually granted, and is effectively abandoned)
    Application JP,1980-109800 (Unexamined Publication JP,S57-035434,A) (Examined Publication JP,S61-024850,B) (Registration number JP,1356613,B) https://patents.google.com/patent/JPS5735434A/en
    Application JP,1981-185490 (Unexamined Publication JP,S58-088926,A) (Not examined or registered) https://patents.google.com/patent/JPS5888926A/en
    Application JP,1982-213971 (Unexamined Publication JP,S59-104699,A) (Not examined or registered) https://patents.google.com/patent/JPS59104699A/en <- this one goes into a bit more detail/better arranged diagrams, and shows a Q table with entries 0-63 rather than 0-48 of the real msm5205

    Application JP,1987-184421 (Unexamined Publication JP,S64-028700,A) (Not examined) (Registration number JP,2581696,B) https://patents.google.com/patent/JPS6428700A/en <- quad band coding system for adpcm?
    Application JP,1994-039523 (Unexamined Publication JP,H07-248798,A) (Not examined) (Registration number JP,3398457,B) https://patents.google.com/patent/JP3398457B2/en <- this may cover the 'adpcm2' method
    Application JP,1995-104333 (Unexamined Publication JP,H08-307371,A) (Not examined or registered) https://patents.google.com/patent/JPH08307371A/en <- something unrelated to adpcm, wireless transmission error detection related?
    Application JP,1995-162009 (Unexamined Publication JP,H09-018425,A) (Not examined or registered) https://patents.google.com/patent/JPH0918425A/en <- looks like ADPCM2 maybe?
        Application JP,1988-176215 (Unexamined Publication JP,H02-026426,A) (Not examined or registered) https://patents.google.com/patent/JPH0226426A/en <- Fujitsu variant on (G.726/727?) SB-ADPCM, cited by above
***************************************************************************/

#include "emu.h"
#include "okiadpcm.h"


//**************************************************************************
//  ADPCM STATE HELPER
//**************************************************************************

// ADPCM state and tables
bool oki_adpcm_state::s_tables_computed = false;
const int8_t oki_adpcm_state::s_index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };
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

int16_t oki_adpcm_state::clock(uint8_t nibble)
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
	static const int8_t nbl2bit[16][4] =
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
const int8_t oki_adpcm2_state::s_index_shift[8] = { -2, -2, -2, -2, 2, 6, 9, 11 };
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

int16_t oki_adpcm2_state::clock(uint8_t nibble)
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
	static const int8_t nbl2bit[16][4] =
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
