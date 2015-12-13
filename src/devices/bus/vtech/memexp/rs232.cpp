// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-200/300 RS-232 Cartridge

***************************************************************************/

#include "rs232.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type RS232_INTERFACE = &device_creator<rs232_interface_device>;

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( rs232 )
	ROM_REGION(0x800, "software", 0)
	ROM_LOAD("rs232_v15.ic2", 0x000, 0x800, CRC(6545260d) SHA1(4042f6f1e09e383f3c92f628c6187dc5f072fdb2))
ROM_END

const rom_entry *rs232_interface_device::device_rom_region() const
{
	return ROM_NAME( rs232 );
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( rs232 )
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(rs232_interface_device, rs232_rx_w))
MACHINE_CONFIG_END

machine_config_constructor rs232_interface_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( rs232 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wordpro_device - constructor
//-------------------------------------------------

rs232_interface_device::rs232_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, RS232_INTERFACE, "DSE VZ-200/300 RS-232 Interface", tag, owner, clock, "vz_rs232", __FILE__),
	device_memexp_interface(mconfig, *this),
	m_rs232(*this, "rs232"),
	m_rx(1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rs232_interface_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rs232_interface_device::device_reset()
{
	// program
	m_slot->m_program->install_rom(0x4000, 0x47ff, 0, 0x800, memregion("software")->base());

	// data
	m_slot->m_program->install_read_handler(0x5000, 0x57ff, read8_delegate(FUNC(rs232_interface_device::receive_data_r), this));
	m_slot->m_program->install_write_handler(0x5800, 0x5fff, write8_delegate(FUNC(rs232_interface_device::transmit_data_w), this));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER( rs232_interface_device::rs232_rx_w )
{
	m_rx = state;
}

READ8_MEMBER( rs232_interface_device::receive_data_r )
{
	return 0x7f | (m_rx << 7);
}

WRITE8_MEMBER( rs232_interface_device::transmit_data_w )
{
	m_rs232->write_txd(!BIT(data, 7));
}
