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

	void dig1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia2c_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia2c_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia34_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia40_ca2_w(int state);

	void bg_speech_clock_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg_speech_digit_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void machine_reset_s11b();
	void init_s11b();
	void init_s11b_invert();

protected:
	optional_device<hc55516_device> m_bg_hc55516;

	void set_invert(bool inv) { m_invert = inv; }

private:
	bool m_invert;  // later System 11B games start expecting inverted data to the display LED segments.


};

#endif /* S11B_H_ */
