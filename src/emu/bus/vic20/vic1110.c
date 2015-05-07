// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1110 8K RAM Expansion Cartridge emulation

**********************************************************************/

#include "vic1110.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	BLK1 = 0x07,
	BLK2 = 0x0b,
	BLK3 = 0x0d,
	BLK5 = 0x0e
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIC1110 = &device_creator<vic1110_device>;



//-------------------------------------------------
//  INPUT_PORTS( vic1110 )
//-------------------------------------------------

INPUT_PORTS_START( vic1110 )
	PORT_START("SW")
	PORT_DIPNAME( 0x0f, BLK1, "Memory Location" ) PORT_DIPLOCATION("SW:1,2,3,4")
	PORT_DIPSETTING(    BLK1, "$2000-$3FFF" )
	PORT_DIPSETTING(    BLK2, "$4000-$5FFF" )
	PORT_DIPSETTING(    BLK3, "$6000-$7FFF" )
	PORT_DIPSETTING(    BLK5, "$A000-B3FFF" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vic1110_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vic1110 );
}




//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1110_device - constructor
//-------------------------------------------------

vic1110_device::vic1110_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VIC1110, "VIC1110", tag, owner, clock, "vic1110", __FILE__),
		device_vic20_expansion_card_interface(mconfig, *this),
		m_ram(*this, "ram"),
		m_sw(*this, "SW")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1110_device::device_start()
{
	// allocate memory
	m_ram.allocate(0x2000);
}


//-------------------------------------------------
//  vic20_cd_r - cartridge data read
//-------------------------------------------------

UINT8 vic1110_device::vic20_cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	UINT8 sw = m_sw->read();

	if ((!blk1 && (sw == BLK1)) || (!blk2 && (sw == BLK2)) || (!blk3 && (sw == BLK3)) || (!blk5 && (sw == BLK5)))
	{
		data = m_ram[offset & 0x1fff];
	}

	return data;
}


//-------------------------------------------------
//  vic20_cd_w - cartridge data write
//-------------------------------------------------

void vic1110_device::vic20_cd_w(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	UINT8 sw = m_sw->read();

	if ((!blk1 && (sw == BLK1)) || (!blk2 && (sw == BLK2)) || (!blk3 && (sw == BLK3)) || (!blk5 && (sw == BLK5)))
	{
		m_ram[offset & 0x1fff] = data;
	}
}
