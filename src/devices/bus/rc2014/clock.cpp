// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Clock Module

****************************************************************************/

#include "emu.h"
#include "clock.h"

class single_clock_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	single_clock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
};

single_clock_device::single_clock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_SINGLE_CLOCK, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
{
}

void single_clock_device::device_start()
{
	m_bus->set_bus_clock(XTAL(7'372'800));
}

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_SINGLE_CLOCK, device_rc2014_card_interface, single_clock_device, "rc2014_clock", "RC2014 Single clock generator")
