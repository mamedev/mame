// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    zcross.h

    Zero-crossing comparator for line-level audio

*********************************************************************/

#ifndef MAME_SOUND_ZCROSS_H
#define MAME_SOUND_ZCROSS_H

#pragma once


// zero-crossing comparator with hysteresis for turning a line output back into a logic level,
// locating each crossing on the band-limited signal rather than between the raw samples
class zero_crossing_comparator
{
public:
	static constexpr int TAPS = 16;

	zero_crossing_comparator(s16 hysteresis);

	bool state() const { return m_state; }

	// returns true when the output changes, with the zero crossing's position in 0..1 after the
	// sample TAPS / 2 samples back; callers delaying every edge by that much keep them in step
	bool update(s16 sample, double &position);

	void register_save(device_t &device, int index = 0) ATTR_COLD;

private:
	s16 const m_hysteresis;
	bool m_state;
	s16 m_history[TAPS];
};

#endif // MAME_SOUND_ZCROSS_H
