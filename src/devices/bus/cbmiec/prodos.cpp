// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VTS Data Professional-DOS

**********************************************************************/

#include "emu.h"
#include "prodos.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "ucd5"
#define M6522_0_TAG     "uab1"
#define M6522_1_TAG     "ucd4"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1541_PROFESSIONAL_DOS_V1, c1541_professional_dos_v1_device, "c1541pd", "VTS Data Professional-DOS")


//-------------------------------------------------
//  ROM( c1541pd )
//-------------------------------------------------

ROM_START( c1541pd )
	ROM_REGION( 0x6000, M6502_TAG, 0 )
	ROM_LOAD( "325302-01.uab4", 0x0000, 0x2000, CRC(29ae9752) SHA1(8e0547430135ba462525c224e76356bd3d430f11) )
	ROM_LOAD( "professionaldos-v1-floppy-expansion-eprom-27128.bin", 0x2000, 0x4000, CRC(c9abf072) SHA1(2b26adc1f4192b6ca1514754f73c929087b24426) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541_professional_dos_v1_device::device_rom_region() const
{
	return ROM_NAME( c1541pd );
}


//-------------------------------------------------
//  ADDRESS_MAP( c1541pd_mem )
//-------------------------------------------------

void c1541_professional_dos_v1_device::c1541pd_mem(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x6000).ram();
	map(0x1800, 0x180f).mirror(0x63f0).m(M6522_0_TAG, FUNC(via6522_device::map));
	map(0x1c00, 0x1c0f).mirror(0x63f0).m(M6522_1_TAG, FUNC(via6522_device::map));
	map(0x8000, 0x9fff).rom().region(M6502_TAG, 0x4000);
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xffff).rom().region(M6502_TAG, 0x0000);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1541_professional_dos_v1_device::device_add_mconfig(machine_config &config)
{
	c1541_device_base::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &c1541_professional_dos_v1_device::c1541pd_mem);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1541_professional_dos_v1_device - constructor
//-------------------------------------------------

c1541_professional_dos_v1_device::c1541_professional_dos_v1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, C1541_PROFESSIONAL_DOS_V1, tag, owner, clock) {  }
