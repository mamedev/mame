// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
 * s11b.h
 *
 *  Created on: 1/01/2013
 */

#ifndef MAME_INCLUDES_S11B_H
#define MAME_INCLUDES_S11B_H

#include "includes/s11a.h"

class s11b_state : public s11a_state
{
public:
	s11b_state(const machine_config &mconfig, device_type type, const char *tag)
		: s11a_state(mconfig, type, tag)
		, m_bg_hc55516(*this, "hc55516_bg")
	{ }

	void s11b(machine_config &config);

	void init_s11b();
	void init_s11b_invert();

	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(pia2c_pa_w);
	DECLARE_WRITE8_MEMBER(pia2c_pb_w);
	DECLARE_WRITE8_MEMBER(pia34_pa_w);

	DECLARE_WRITE8_MEMBER(bg_speech_clock_w);
	DECLARE_WRITE8_MEMBER(bg_speech_digit_w);

protected:
	void set_invert(bool inv) { m_invert = inv; }

private:
	DECLARE_MACHINE_RESET(s11b);

	void s11b_audio_map(address_map &map);
	void s11b_bg_map(address_map &map);
	void s11b_main_map(address_map &map);

	optional_device<hc55516_device> m_bg_hc55516;


	bool m_invert;  // later System 11B games start expecting inverted data to the display LED segments.
};

#endif // MAME_INCLUDES_S11B_H
