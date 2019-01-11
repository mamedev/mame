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

	void s11a(machine_config &config);

	void init_s11a();

	DECLARE_WRITE8_MEMBER(bgbank_w);
	DECLARE_WRITE8_MEMBER(dig0_w);

private:
	DECLARE_MACHINE_RESET(s11a);

	void s11a_audio_map(address_map &map);
	void s11a_bg_map(address_map &map);
	void s11a_main_map(address_map &map);
};

#endif // MAME_INCLUDES_S11A_H
