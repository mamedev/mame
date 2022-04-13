// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    uthernet.cpp

    Apple II Uthernet Card

*********************************************************************/

#include "emu.h"
#include "uthernet.h"

#include "machine/cs8900a.h"

namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_uthernet_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_uthernet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2bus_uthernet_device(mconfig, A2BUS_UTHERNET, tag, owner, clock)
	{
	}

protected:
	a2bus_uthernet_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_netinf(*this, "cs8900a")
	{
	}

	virtual void device_add_mconfig(machine_config &config) override
	{
		CS8900A(config, m_netinf, 20_MHz_XTAL); // see CS8900A data sheet sec 3.13
	}

	virtual void device_start() override { }
	virtual void device_reset() override { }

	// read_c0nx - called for reads from this card's c0nx space
	virtual uint8_t read_c0nx(uint8_t offset) override { return m_netinf->read(offset); }

	// write_c0nx - called for writes to this card's c0nx space
	virtual void write_c0nx(uint8_t offset, uint8_t data) override { m_netinf->write(offset,data); }

private:
	required_device<cs8900a_device> m_netinf;

};

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_UTHERNET, device_a2bus_card_interface, a2bus_uthernet_device, "a2uthernet", "a2RetroSystems Uthernet")
