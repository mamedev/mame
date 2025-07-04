// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Atom Tube Interface

**********************************************************************/

#include "emu.h"
#include "tube.h"

#include "bus/bbc/tube/tube.h"


namespace {

class atom_tube_device : public device_t, public device_acorn_bus_interface
{
public:
	atom_tube_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ATOM_TUBE, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_tube(*this, "tube")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<bbc_tube_slot_device> m_tube;

	void bus_irq_w(int state)
	{
		m_bus->irq_w(state);
	}
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_tube_device::device_add_mconfig(machine_config &config)
{
	BBC_TUBE_SLOT(config, m_tube, electron_tube_devices, nullptr);
	m_tube->irq_handler().set(FUNC(atom_tube_device::bus_irq_w));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atom_tube_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xbee0, 0xbeef, emu::rw_delegate(*m_tube, FUNC(bbc_tube_slot_device::host_r)), emu::rw_delegate(*m_tube, FUNC(bbc_tube_slot_device::host_w)));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ATOM_TUBE, device_acorn_bus_interface, atom_tube_device, "atom_tube", "Atom Tube Interface")
