// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    AtomSID emulation

**********************************************************************/

#include "emu.h"
#include "sid.h"

#include "sound/mos6581.h"
#include "speaker.h"


namespace {

class atom_sid_device : public device_t, public device_acorn_bus_interface
{
public:
	atom_sid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ATOM_SID, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_sid(*this, "sid6581")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<mos6581_device> m_sid;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_sid_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	MOS6581(config, m_sid, 4_MHz_XTAL / 4);
	m_sid->add_route(ALL_OUTPUTS, "speaker", 1.0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atom_sid_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xbdc0, 0xbddf, emu::rw_delegate(*m_sid, FUNC(mos6581_device::read)), emu::rw_delegate(*m_sid, FUNC(mos6581_device::write)));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ATOM_SID, device_acorn_bus_interface, atom_sid_device, "atom_sid", "AtomSID")
