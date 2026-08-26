// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Evesham Micros Dolphin-DOS 2.0

**********************************************************************/

#include "emu.h"
#include "dolphindos.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "ucd5"
#define M6522_0_TAG     "uab1"
#define M6522_1_TAG     "ucd4"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1541_DOLPHIN_DOS, c1541_dolphin_dos_device, "c1541dd", "Evesham Micros Dolphin-DOS")


//-------------------------------------------------
//  ROM( c1541dd )
//-------------------------------------------------

ROM_START( c1541dd )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "dd20.bin", 0x0000, 0x8000, CRC(94c7fe19) SHA1(e4d5b9ad6b719dd988276214aa4536d3525d313c) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541_dolphin_dos_device::device_rom_region() const
{
	return ROM_NAME( c1541dd );
}


//-------------------------------------------------
//  ADDRESS_MAP( c1541dd_mem )
//-------------------------------------------------

void c1541_dolphin_dos_device::c1541dd_mem(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x6000).ram();
	map(0x1800, 0x180f).mirror(0x63f0).m(M6522_0_TAG, FUNC(via6522_device::map));
	map(0x1c00, 0x1c0f).mirror(0x63f0).m(M6522_1_TAG, FUNC(via6522_device::map));
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xffff).rom().region(M6502_TAG, 0x2000);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1541_dolphin_dos_device::device_add_mconfig(machine_config &config)
{
	c1541_device_base::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &c1541_dolphin_dos_device::c1541dd_mem);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1541_dolphin_dos_device - constructor
//-------------------------------------------------

c1541_dolphin_dos_device::c1541_dolphin_dos_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, C1541_DOLPHIN_DOS, tag, owner, clock) {  }
