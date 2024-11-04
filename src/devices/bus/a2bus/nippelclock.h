// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    nippelclock.h

    Implementation of the Nippel Clock card for Agat.

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2NIPPELCLOCK_H
#define MAME_BUS_A2BUS_A2NIPPELCLOCK_H

#pragma once

#include "a2bus.h"
#include "machine/mc146818.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_nippelclock_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_nippelclock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_nippelclock_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	required_device<mc146818_device> m_rtc;

private:
	void irq_w(int state);
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_NIPPELCLOCK, a2bus_nippelclock_device)

#endif // MAME_BUS_A2BUS_A2NIPPELCLOCK_H
