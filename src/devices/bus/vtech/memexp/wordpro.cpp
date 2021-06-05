// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-300 WordPro Cartridge

***************************************************************************/

#include "emu.h"
#include "wordpro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VTECH_WORDPRO, vtech_wordpro_device, "vtech_wordpro", "DSE VZ-300 WordPro")

//-------------------------------------------------
//  mem_map - memory space address map
//-------------------------------------------------

void vtech_wordpro_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x6000, 0x67ff).rom().region("software", 0);
	map(0xd000, 0xffff).rom().region("software", 0);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( wordpro )
	ROM_REGION(0x3000, "software", 0)
	ROM_LOAD("wordpro.u3", 0x0000, 0x1000, CRC(c37ea780) SHA1(03f56711e08d88e7b523c0ef9c2a5af83ee7ad05))
	ROM_LOAD("wordpro.u4", 0x1000, 0x1000, CRC(2e3a8c45) SHA1(a9d48d809f39a3478496a6d3ddd728bd0b4efc37))
	ROM_LOAD("wordpro.u5", 0x2000, 0x1000, CRC(2a336802) SHA1(b4de50f943243f18a2bfabef354b76d77178c189))
ROM_END

const tiny_rom_entry *vtech_wordpro_device::device_rom_region() const
{
	return ROM_NAME( wordpro );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_wordpro_device - constructor
//-------------------------------------------------

vtech_wordpro_device::vtech_wordpro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vtech_memexp_device(mconfig, VTECH_WORDPRO, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_wordpro_device::device_start()
{
	vtech_memexp_device::device_start();
}
