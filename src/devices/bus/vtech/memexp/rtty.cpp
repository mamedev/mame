// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-200/300 RTTY Cartridge (K-6318)

***************************************************************************/

#include "rtty.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type RTTY_INTERFACE = &device_creator<rtty_interface_device>;

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( rtty )
	ROM_REGION(0x1000, "software", 0)
	ROM_LOAD("vzrtty.ic3", 0x0000, 0x1000, CRC(ccf4289b) SHA1(de737ef0e0b582b3102da473836af1fa159a2e78))
ROM_END

const tiny_rom_entry *rtty_interface_device::device_rom_region() const
{
	return ROM_NAME( rtty );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  rtty_interface_device - constructor
//-------------------------------------------------

rtty_interface_device::rtty_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, RTTY_INTERFACE, "DSE VZ-200/300 RTTY Interface", tag, owner, clock, "vz_rtty", __FILE__),
	device_memexp_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rtty_interface_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rtty_interface_device::device_reset()
{
	// program
	m_slot->m_program->install_rom(0x4000, 0x4fff, 0x1000, memregion("software")->base());

	// data
	m_slot->m_program->install_read_handler(0x5000, 0x57ff, read8_delegate(FUNC(rtty_interface_device::receive_data_r), this));
	m_slot->m_program->install_write_handler(0x5800, 0x5fff, write8_delegate(FUNC(rtty_interface_device::transmit_data_w), this));
	m_slot->m_program->install_write_handler(0x6000, 0x67ff, write8_delegate(FUNC(rtty_interface_device::relay_w), this));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER( rtty_interface_device::receive_data_r )
{
	return 0xff;
}

WRITE8_MEMBER( rtty_interface_device::transmit_data_w )
{
	logerror("transmit_w: %d\n", BIT(data, 7));
}

WRITE8_MEMBER( rtty_interface_device::relay_w )
{
	logerror("relay_w: %d\n", BIT(data, 7));
}
