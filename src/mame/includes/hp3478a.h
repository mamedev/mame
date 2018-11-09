// license:BSD-3-Clause
// copyright-holders:fenugrec
/***************************************************************************
 HP 3478A Digital Multimeter

 XXX Not sure why I need this file, can this all go in hp3478a.cpp ?
****************************************************************************/

#pragma once

#ifndef MAME_INCLUDES_HP3478A_H
#define MAME_INCLUDES_HP3478A_H

#include "cpu/mcs48/mcs48.h"


class hp3478a_state : public driver_device
{
public:
	hp3478a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void hp3478a(machine_config &config);
	void i8039_io(address_map &map);
	void i8039_map(address_map &map);

protected:
	virtual void machine_start() override;
	//virtual void machine_reset() override;	//not needed?

	DECLARE_WRITE8_MEMBER(port1_w);
	DECLARE_READ8_MEMBER(port1_r);

	required_device<i8039_device> m_maincpu;
};


#endif // MAME_INCLUDES_HP3478A_H
