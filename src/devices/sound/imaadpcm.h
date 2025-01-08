// license:BSD-3-Clause
// copyright-holders:Andrew Gardner,Aaron Giles
/***************************************************************************

    imaadpcm.h

    IMA ADCPM emulation.

***************************************************************************/

#ifndef MAME_SOUND_IMAADPCM_H
#define MAME_SOUND_IMAADPCM_H

#pragma once


// ======================> ima_adpcm_state

// Internal ADPCM state
class ima_adpcm_state
{
public:
	ima_adpcm_state() { compute_tables(); reset(); }

	void reset();
	int16_t clock(uint8_t nibble);
	int16_t output() { return m_signal; }
	void save();
	void restore();

	int32_t   m_signal;
	int32_t   m_step;
	int32_t   m_loop_signal;
	int32_t   m_loop_step;
	bool      m_saved;

private:
	static const int8_t s_index_shift[8];
	static int s_diff_lookup[89*16];

	static void compute_tables();
	static bool s_tables_computed;
};

#endif // MAME_SOUND_IMAADPCM_H
