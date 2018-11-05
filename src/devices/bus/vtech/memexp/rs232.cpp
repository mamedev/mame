// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-200/300 RS-232 Cartridge (K-6317)

***************************************************************************/

#include "emu.h"
#include "rs232.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VTECH_RS232_INTERFACE, vtech_rs232_interface_device, "vtech_rs232", "DSE VZ-200/300 RS-232 Interface")

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( rs232 )
	ROM_REGION(0x800, "software", 0)
	ROM_LOAD("rs232_v15.ic2", 0x000, 0x800, CRC(6545260d) SHA1(4042f6f1e09e383f3c92f628c6187dc5f072fdb2))
ROM_END

const tiny_rom_entry *vtech_rs232_interface_device::device_rom_region() const
{
	return ROM_NAME( rs232 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vtech_rs232_interface_device::device_add_mconfig(machine_config &config)
{
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(vtech_rs232_interface_device::rs232_rx_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_rs232_interface_device - constructor
//-------------------------------------------------

vtech_rs232_interface_device::vtech_rs232_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VTECH_RS232_INTERFACE, tag, owner, clock),
	device_vtech_memexp_interface(mconfig, *this),
	m_rs232(*this, "rs232"),
	m_rx(1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_rs232_interface_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vtech_rs232_interface_device::device_reset()
{
	// program
	program_space().install_rom(0x4000, 0x47ff, 0x800, memregion("software")->base());

	// data
	program_space().install_read_handler(0x5000, 0x57ff, read8_delegate(FUNC(vtech_rs232_interface_device::receive_data_r), this));
	program_space().install_write_handler(0x5800, 0x5fff, write8_delegate(FUNC(vtech_rs232_interface_device::transmit_data_w), this));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER( vtech_rs232_interface_device::rs232_rx_w )
{
	m_rx = state;
}

READ8_MEMBER( vtech_rs232_interface_device::receive_data_r )
{
	return 0x7f | (m_rx << 7);
}

WRITE8_MEMBER( vtech_rs232_interface_device::transmit_data_w )
{
	m_rs232->write_txd(!BIT(data, 7));
}
