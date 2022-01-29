// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-200/300 RTTY Cartridge (K-6318)

***************************************************************************/

#include "emu.h"
#include "rtty.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VTECH_RTTY_INTERFACE, vtech_rtty_interface_device, "vtech_rtty", "DSE VZ-200/300 RTTY Interface")

//-------------------------------------------------
//  mem_map - memory space address map
//-------------------------------------------------

void vtech_rtty_interface_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x4000, 0x4fff).rom().region("software", 0);
	map(0x5000, 0x5000).mirror(0x7ff).r(FUNC(vtech_rtty_interface_device::receive_data_r));
	map(0x5800, 0x5800).mirror(0x7ff).w(FUNC(vtech_rtty_interface_device::transmit_data_w));
	map(0x6000, 0x6000).mirror(0x7ff).w(FUNC(vtech_rtty_interface_device::relay_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( rtty )
	ROM_REGION(0x1000, "software", 0)
	ROM_LOAD("vzrtty.ic3", 0x0000, 0x1000, CRC(ccf4289b) SHA1(de737ef0e0b582b3102da473836af1fa159a2e78))
ROM_END

const tiny_rom_entry *vtech_rtty_interface_device::device_rom_region() const
{
	return ROM_NAME( rtty );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_rtty_interface_device - constructor
//-------------------------------------------------

vtech_rtty_interface_device::vtech_rtty_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vtech_memexp_device(mconfig, VTECH_RTTY_INTERFACE, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_rtty_interface_device::device_start()
{
	vtech_memexp_device::device_start();
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t vtech_rtty_interface_device::receive_data_r()
{
	return 0xff;
}

void vtech_rtty_interface_device::transmit_data_w(uint8_t data)
{
	logerror("transmit_w: %d\n", BIT(data, 7));
}

void vtech_rtty_interface_device::relay_w(uint8_t data)
{
	logerror("relay_w: %d\n", BIT(data, 7));
}
