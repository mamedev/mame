// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
 * s11c.h
 *
 *  Created on: 1/01/2013
 */

#ifndef MAME_INCLUDES_S11C_H
#define MAME_INCLUDES_S11C_H

#include "includes/s11b.h"

class s11c_state : public s11b_state
{
public:
	s11c_state(const machine_config &mconfig, device_type type, const char *tag)
		: s11b_state(mconfig, type, tag)
	{ }

	DECLARE_MACHINE_RESET(s11c);
	DECLARE_DRIVER_INIT(s11c);

	void s11c(machine_config &config);
	void s11c_main_map(address_map &map);
protected:

private:


};


#endif // MAME_INCLUDES_S11C_H
