// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-300 WordPro Cartridge

***************************************************************************/

#include "wordpro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type WORDPRO = &device_creator<wordpro_device>;

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( wordpro )
	ROM_REGION(0x3000, "software", 0)
	ROM_LOAD("wordpro.u3", 0x0000, 0x1000, CRC(c37ea780) SHA1(03f56711e08d88e7b523c0ef9c2a5af83ee7ad05))
	ROM_LOAD("wordpro.u4", 0x1000, 0x1000, CRC(2e3a8c45) SHA1(a9d48d809f39a3478496a6d3ddd728bd0b4efc37))
	ROM_LOAD("wordpro.u5", 0x2000, 0x1000, CRC(2a336802) SHA1(b4de50f943243f18a2bfabef354b76d77178c189))
ROM_END

const rom_entry *wordpro_device::device_rom_region() const
{
	return ROM_NAME( wordpro );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wordpro_device - constructor
//-------------------------------------------------

wordpro_device::wordpro_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WORDPRO, "DSE VZ-300 WordPro", tag, owner, clock, "wordpro", __FILE__),
	device_memexp_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wordpro_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wordpro_device::device_reset()
{
	m_slot->m_program->install_rom(0x6000, 0x67ff, memregion("software")->base());
	m_slot->m_program->install_rom(0xd000, 0xffff, memregion("software")->base());
}
