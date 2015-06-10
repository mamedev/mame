// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot RAM Expansions

***************************************************************************/

#include "ram.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type APRICOT_256K_RAM = &device_creator<apricot_256k_ram_device>;
const device_type APRICOT_128K_RAM = &device_creator<apricot_128k_ram_device>;
const device_type APRICOT_512K_RAM = &device_creator<apricot_512k_ram_device>;


//**************************************************************************
//  APRICOT 256K RAM DEVICE
//**************************************************************************

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( apricot_256k )
	PORT_START("sw")
	PORT_DIPNAME(0x01, 0x00, "Base Address")
	PORT_DIPSETTING(0x00, "40000H")
	PORT_DIPSETTING(0x01, "80000H")
INPUT_PORTS_END

ioport_constructor apricot_256k_ram_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( apricot_256k );
}

//-------------------------------------------------
//  apricot_256k_ram_device - constructor
//-------------------------------------------------

apricot_256k_ram_device::apricot_256k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, APRICOT_256K_RAM, "Apricot 256K RAM Expansion Board", tag, owner, clock, "apricot_256k_ram", __FILE__),
	device_apricot_expansion_card_interface(mconfig, *this),
	m_sw(*this, "sw")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apricot_256k_ram_device::device_start()
{
	m_ram.resize(0x40000 / 2);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apricot_256k_ram_device::device_reset()
{
	if (m_sw->read() == 0)
		m_bus->install_ram(0x40000, 0x7ffff, &m_ram[0]);
	else
		m_bus->install_ram(0x80000, 0xbffff, &m_ram[0]);
}


//**************************************************************************
//  APRICOT 128K RAM DEVICE
//**************************************************************************

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( apricot_128k )
	PORT_START("strap")
	PORT_DIPNAME(0x03, 0x01, "Base Address")
	PORT_DIPSETTING(0x00, "512K")
	PORT_DIPSETTING(0x01, "256K - 384K")
	PORT_DIPSETTING(0x02, "384K - 512K")
INPUT_PORTS_END

ioport_constructor apricot_128k_ram_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( apricot_128k );
}

//-------------------------------------------------
//  apricot_128_512k_ram_device - constructor
//-------------------------------------------------

apricot_128k_ram_device::apricot_128k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, APRICOT_128K_RAM, "Apricot 128/512K RAM Expansion Board (128K)", tag, owner, clock, "apricot_128k_ram", __FILE__),
	device_apricot_expansion_card_interface(mconfig, *this),
	m_strap(*this, "strap")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apricot_128k_ram_device::device_start()
{
	m_ram.resize(0x20000 / 2);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apricot_128k_ram_device::device_reset()
{
	if (m_strap->read() == 1)
		m_bus->install_ram(0x40000, 0x5ffff, &m_ram[0]);
	else if (m_strap->read() == 2)
		m_bus->install_ram(0x60000, 0x7ffff, &m_ram[0]);
}


//**************************************************************************
//  APRICOT 512K RAM DEVICE
//**************************************************************************

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( apricot_512k )
	PORT_START("strap")
	PORT_DIPNAME(0x03, 0x00, "Base Address")
	PORT_DIPSETTING(0x00, "512K")
	PORT_DIPSETTING(0x01, "256K - 384K")
	PORT_DIPSETTING(0x02, "384K - 512K")
INPUT_PORTS_END

ioport_constructor apricot_512k_ram_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( apricot_512k );
}

//-------------------------------------------------
//  apricot_128_512k_ram_device - constructor
//-------------------------------------------------

apricot_512k_ram_device::apricot_512k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, APRICOT_512K_RAM, "Apricot 128/512K RAM Expansion Board (512K)", tag, owner, clock, "apricot_512k_ram", __FILE__),
	device_apricot_expansion_card_interface(mconfig, *this),
	m_strap(*this, "strap")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apricot_512k_ram_device::device_start()
{
	m_ram.resize(0x80000 / 2);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apricot_512k_ram_device::device_reset()
{
	if (m_strap->read() == 0)
		m_bus->install_ram(0x40000, 0xbffff, &m_ram[0]);
}
