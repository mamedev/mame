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
	{ }

	void s11b_base(machine_config &config);
	void s11b(machine_config &config);
	void s11b_jokerz(machine_config &config);

	void init_s11b();
	void init_s11b_invert();

protected:
	virtual void machine_reset() override;
	void set_invert(bool inv) { m_invert = inv; }

	void dig1_w(uint8_t data);
	void pia2c_pa_w(uint8_t data);
	void pia2c_pb_w(uint8_t data);
	void pia34_pa_w(uint8_t data);

private:
	bool m_invert;  // later System 11B games start expecting inverted data to the display LED segments.
};

#endif // MAME_INCLUDES_S11B_H
