// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Cassette Interface

**********************************************************************/


#ifndef MAME_BUS_ACORN_SYSTEM_CASS_H
#define MAME_BUS_ACORN_SYSTEM_CASS_H

#pragma once

#include "bus/acorn/bus.h"
#include "machine/timer.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "speaker.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class acorn_cass_device :
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	acorn_cass_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void cass_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(cass_c);
	TIMER_DEVICE_CALLBACK_MEMBER(cass_p);

	required_device<cassette_image_device> m_cass;

	uint8_t m_cass_data[4];
	bool m_cass_state;
	bool m_cassold;
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_CASS, acorn_cass_device)


#endif // MAME_BUS_ACORN_SYSTEM_CASS_H
