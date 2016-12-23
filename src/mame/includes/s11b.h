// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
 * s11b.h
 *
 *  Created on: 1/01/2013
 */

#ifndef S11B_H_
#define S11B_H_

#include "includes/s11a.h"

class s11b_state : public s11a_state
{
public:
	s11b_state(const machine_config &mconfig, device_type type, const char *tag)
		: s11a_state(mconfig, type, tag),
		m_bg_hc55516(*this, "hc55516_bg")

	{ }

	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(pia2c_pa_w);
	DECLARE_WRITE8_MEMBER(pia2c_pb_w);
	DECLARE_WRITE8_MEMBER(pia34_pa_w);
	DECLARE_WRITE_LINE_MEMBER(pia40_ca2_w);

	DECLARE_WRITE8_MEMBER(bg_speech_clock_w);
	DECLARE_WRITE8_MEMBER(bg_speech_digit_w);

	DECLARE_MACHINE_RESET(s11b);
	DECLARE_DRIVER_INIT(s11b);
	DECLARE_DRIVER_INIT(s11b_invert);

protected:
	optional_device<hc55516_device> m_bg_hc55516;

	void set_invert(bool inv) { m_invert = inv; }

private:
	bool m_invert;  // later System 11B games start expecting inverted data to the display LED segments.


};

#endif /* S11B_H_ */
