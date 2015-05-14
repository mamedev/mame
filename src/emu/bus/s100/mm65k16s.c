// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs MM65K16S memory board emulation

**********************************************************************/

#include "mm65k16s.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************




//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type S100_MM65K16S = &device_creator<s100_mm65k16s_device>;


//-------------------------------------------------
//  ROM( mm65k16s )
//-------------------------------------------------

ROM_START( mm65k16s )
	ROM_REGION( 0x10000, "proms", 0 )
	ROM_LOAD( "82s100.6c", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x10, "plds", 0 )
	ROM_LOAD( "pal14l4.6d", 0x0000, 0x10, NO_DUMP )
	ROM_LOAD( "pal16l2.16d", 0x0000, 0x10, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *s100_mm65k16s_device::device_rom_region() const
{
	return ROM_NAME( mm65k16s );
}


//-------------------------------------------------
//  INPUT_PORTS( mm65k16s )
//-------------------------------------------------

static INPUT_PORTS_START( mm65k16s )
	PORT_START("J6754")
	PORT_DIPNAME( 0x01, 0x00, "Bank A Lower 32K" )
	PORT_DIPSETTING(    0x01, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )
	PORT_DIPNAME( 0x02, 0x00, "Bank A Upper 32K" )
	PORT_DIPSETTING(    0x02, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )
	PORT_DIPNAME( 0x04, 0x00, "Bank B Lower 32K" )
	PORT_DIPSETTING(    0x04, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )
	PORT_DIPNAME( 0x08, 0x00, "Bank B Upper 32K" )
	PORT_DIPSETTING(    0x08, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )

	PORT_START("J2")
	PORT_DIPNAME( 0x01, 0x00, "Bank A Recognizes Phantom" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("J1")
	PORT_DIPNAME( 0x01, 0x00, "Bank B Recognizes Phantom" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("5D")
	PORT_DIPNAME( 0x03, 0x00, "First 16K Memory Addressing" ) PORT_DIPLOCATION("5D:1,2")
	PORT_DIPSETTING(    0x00, "Block 0 (0000H-3FFFH)" )
	PORT_DIPSETTING(    0x02, "Block 1 (4000H-7FFFH)" )
	PORT_DIPSETTING(    0x01, "Block 2 (8000H-BFFFH)" )
	PORT_DIPSETTING(    0x03, "Block 3 (C000H-FFFFH" )
	PORT_DIPNAME( 0x0c, 0x08, "Second 16K Memory Addressing" ) PORT_DIPLOCATION("5D:3,4")
	PORT_DIPSETTING(    0x00, "Block 0 (0000H-3FFFH)" )
	PORT_DIPSETTING(    0x08, "Block 1 (4000H-7FFFH)" )
	PORT_DIPSETTING(    0x04, "Block 2 (8000H-BFFFH)" )
	PORT_DIPSETTING(    0x0c, "Block 3 (C000H-FFFFH" )
	PORT_DIPNAME( 0x30, 0x10, "Third 16K Memory Addressing" ) PORT_DIPLOCATION("5D:5,6")
	PORT_DIPSETTING(    0x00, "Block 0 (0000H-3FFFH)" )
	PORT_DIPSETTING(    0x20, "Block 1 (4000H-7FFFH)" )
	PORT_DIPSETTING(    0x10, "Block 2 (8000H-BFFFH)" )
	PORT_DIPSETTING(    0x30, "Block 3 (C000H-FFFFH" )
	PORT_DIPNAME( 0xc0, 0xc0, "Fourth 16K Memory Addressing" ) PORT_DIPLOCATION("5D:7,8")
	PORT_DIPSETTING(    0x00, "Block 0 (0000H-3FFFH)" )
	PORT_DIPSETTING(    0x80, "Block 1 (4000H-7FFFH)" )
	PORT_DIPSETTING(    0x40, "Block 2 (8000H-BFFFH)" )
	PORT_DIPSETTING(    0xc0, "Block 3 (C000H-FFFFH" )

	PORT_START("PAGE07")
	PORT_DIPNAME( 0x0f, 0x00, "2K Segment Disable" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, "Page 0" )
	PORT_DIPSETTING(    0x01, "Page 1" )
	PORT_DIPSETTING(    0x02, "Page 2" )
	PORT_DIPSETTING(    0x03, "Page 3" )
	PORT_DIPSETTING(    0x04, "Page 4" )
	PORT_DIPSETTING(    0x05, "Page 5" )
	PORT_DIPSETTING(    0x06, "Page 6" )
	PORT_DIPSETTING(    0x07, "Page 7" )

	PORT_START("J3")
	PORT_DIPNAME( 0x01, 0x01, "Addressing Mode" )
	PORT_DIPSETTING(    0x01, "Extended Addressing" )
	PORT_DIPSETTING(    0x00, "Bank Select" )

	PORT_START("1C")
	PORT_DIPNAME( 0xff, 0x00, "Extended Addressing" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x00, "000000H" )
	// ...
	PORT_DIPSETTING(    0xff, "FF0000H" )
	PORT_DIPNAME( 0xff, 0x40, "Bank Select Port" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "00H" )
	// ...
	PORT_DIPSETTING(    0x40, "40H" )
	// ...
	PORT_DIPSETTING(    0xff, "FFH" )

	PORT_START("A0A7")
	PORT_DIPNAME( 0x01, 0x01, "Bank Select Data Bit A0" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPNAME( 0x02, 0x00, "Bank Select Data Bit A1" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPNAME( 0x04, 0x00, "Bank Select Data Bit A2" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPNAME( 0x08, 0x00, "Bank Select Data Bit A3" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPNAME( 0x10, 0x00, "Bank Select Data Bit A4" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPNAME( 0x20, 0x00, "Bank Select Data Bit A5" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPNAME( 0x40, 0x00, "Bank Select Data Bit A6" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPNAME( 0x80, 0x00, "Bank Select Data Bit A7" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x80, "1" )

	PORT_START("0B7B")
	PORT_DIPNAME( 0x01, 0x00, "Bank Select Data Bit 0B" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPNAME( 0x02, 0x00, "Bank Select Data Bit 1B" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPNAME( 0x04, 0x00, "Bank Select Data Bit 2B" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPNAME( 0x08, 0x00, "Bank Select Data Bit 3B" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPNAME( 0x10, 0x00, "Bank Select Data Bit 4B" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPNAME( 0x20, 0x00, "Bank Select Data Bit 5B" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPNAME( 0x40, 0x00, "Bank Select Data Bit 6B" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPNAME( 0x80, 0x00, "Bank Select Data Bit 7B" ) PORT_CONDITION("J3", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x80, "1" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor s100_mm65k16s_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mm65k16s );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s100_mm65k16s_device - constructor
//-------------------------------------------------

s100_mm65k16s_device::s100_mm65k16s_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, S100_MM65K16S, "MM65K16S", tag, owner, clock, "mm65k16s", __FILE__),
	device_s100_card_interface(mconfig, *this),
	m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s100_mm65k16s_device::device_start()
{
	m_ram.allocate(0x10000);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s100_mm65k16s_device::device_reset()
{
}


//-------------------------------------------------
//  s100_smemr_r - memory read
//-------------------------------------------------

UINT8 s100_mm65k16s_device::s100_smemr_r(address_space &space, offs_t offset)
{
	UINT8 data = 0;

	if (offset < 0xf800)
	{
		data = m_ram[offset];
	}

	return data;
}


//-------------------------------------------------
//  s100_mwrt_w - memory write
//-------------------------------------------------

void s100_mm65k16s_device::s100_mwrt_w(address_space &space, offs_t offset, UINT8 data)
{
	if (offset < 0xf800)
	{
		m_ram[offset] = data;
	}
}


//-------------------------------------------------
//  s100_phantom_w - phantom
//-------------------------------------------------

void s100_mm65k16s_device::s100_phantom_w(int state)
{
}
