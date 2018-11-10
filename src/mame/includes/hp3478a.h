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

/* port pin/bit defs */
#define P20	(1 << 0)
#define P21	(1 << 1)
#define P22	(1 << 2)
#define P23	(1 << 3)
#define P24	(1 << 4)
#define P25	(1 << 5)
#define P26	(1 << 6)
#define P27	(1 << 7)

class hp3478a_state : public driver_device
{
public:
	hp3478a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bank0(*this, "bank0")
	{
	}

	void hp3478a(machine_config &config);
	void i8039_io(address_map &map);
	void i8039_map(address_map &map);

protected:
	virtual void machine_start() override;
	//virtual void machine_reset() override;	//not needed?

	DECLARE_WRITE8_MEMBER(p1write);
	DECLARE_READ8_MEMBER(p1read);
	DECLARE_WRITE8_MEMBER(p2write);
	DECLARE_READ8_MEMBER(busread);
	DECLARE_WRITE8_MEMBER(buswrite);

	required_device<i8039_device> m_maincpu;
	required_memory_bank m_bank0;

};


#endif // MAME_INCLUDES_HP3478A_H
