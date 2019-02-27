// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    AtomSID emulation

**********************************************************************/


#include "emu.h"
#include "sid.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ATOM_SID, atom_sid_device, "atom_sid", "AtomSID")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_sid_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	MOS6581(config, m_sid, 4_MHz_XTAL / 4);
	m_sid->add_route(ALL_OUTPUTS, "speaker", 1.0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  atom_sid_device - constructor
//-------------------------------------------------

atom_sid_device::atom_sid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ATOM_SID, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_sid(*this, "sid6581")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atom_sid_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xbdc0, 0xbddf, read8sm_delegate(FUNC(mos6581_device::read), m_sid.target()), write8sm_delegate(FUNC(mos6581_device::write), m_sid.target()));
}
