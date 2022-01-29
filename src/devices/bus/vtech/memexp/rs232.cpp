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
//  mem_map - memory space address map
//-------------------------------------------------

void vtech_rs232_interface_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x4000, 0x47ff).rom().region("software", 0);
	map(0x5000, 0x5000).mirror(0x7ff).r(FUNC(vtech_rs232_interface_device::receive_data_r));
	map(0x5800, 0x5800).mirror(0x7ff).w(FUNC(vtech_rs232_interface_device::transmit_data_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( rs232 )
	ROM_REGION(0x800, "software", 0)
	ROM_DEFAULT_BIOS("16")
	ROM_SYSTEM_BIOS(0, "15", "Version 1.5") // 1985
	ROMX_LOAD("rs232_v15.ic2", 0x000, 0x800, CRC(6545260d) SHA1(4042f6f1e09e383f3c92f628c6187dc5f072fdb2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "16", "Version 1.6") // 1987
	ROMX_LOAD("rs232_v16.ic2", 0x000, 0x800, CRC(d761fc79) SHA1(28e00c7ff33143a948308330859bee54b474e114), ROM_BIOS(1))
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
	vtech_memexp_device::device_add_mconfig(config);

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set([this](int state) { m_rx = state; });
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_rs232_interface_device - constructor
//-------------------------------------------------

vtech_rs232_interface_device::vtech_rs232_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vtech_memexp_device(mconfig, VTECH_RS232_INTERFACE, tag, owner, clock),
	m_rs232(*this, "rs232"),
	m_rx(1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_rs232_interface_device::device_start()
{
	vtech_memexp_device::device_start();

	// register for save states
	save_item(NAME(m_rx));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t vtech_rs232_interface_device::receive_data_r()
{
	return 0x7f | (m_rx << 7);
}

void vtech_rs232_interface_device::transmit_data_w(uint8_t data)
{
	m_rs232->write_txd(!BIT(data, 7));
}
