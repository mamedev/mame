// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/********************************************************************************
 *
 * mame/includes/mzr8105
 *
 ********************************************************************************/

#ifndef MZR8105_H
#define MZR8105_H
#pragma once

#include "bus/vme/vme.h"

class mzr8105_state : public driver_device
{
public:
mzr8105_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device (mconfig, type, tag)
	,m_maincpu (*this, "maincpu")
	{
	}

	virtual void machine_start() override;
private:
	required_device<cpu_device> m_maincpu;
};


#endif // MZR8105_H
