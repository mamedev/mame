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

	void s11c(machine_config &config);

	void init_s11c();

protected:
	virtual void machine_reset() override;

private:

	void s11c_main_map(address_map &map);
	void s11c_audio_map(address_map &map);
	void s11c_bg_map(address_map &map);
};


#endif // MAME_INCLUDES_S11C_H
