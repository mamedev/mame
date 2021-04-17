// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
 * s11a.h
 *
 *  Created on: 1/01/2013
 */

#ifndef MAME_INCLUDES_S11A_H
#define MAME_INCLUDES_S11A_H

#include "includes/s11.h"

class s11a_state : public s11_state
{
public:
	s11a_state(const machine_config &mconfig, device_type type, const char *tag)
		: s11_state(mconfig, type, tag)
	{ }

	void s11a_base(machine_config &config);
	void s11a(machine_config &config);
	void s11a_obg(machine_config &config);

	void init_s11a();

	void dig0_w(uint8_t data);
};

#endif // MAME_INCLUDES_S11A_H
