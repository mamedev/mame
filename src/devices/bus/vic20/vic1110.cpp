// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1110 8K RAM Expansion Cartridge emulation

**********************************************************************/

#include "emu.h"
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

DEFINE_DEVICE_TYPE(VIC1110, vic1110_device, "vic1110", "VIC-1110 8K RAM Expansion")



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

vic1110_device::vic1110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC1110, tag, owner, clock)
	, device_vic20_expansion_card_interface(mconfig, *this)
	, m_ram(*this, "ram", 0x2000, ENDIANNESS_LITTLE)
	, m_sw(*this, "SW")
	, m_window(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1110_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic1110_device::device_reset()
{
	vic20_expansion_window *window = nullptr;

	switch (m_sw->read())
	{
	case BLK1: window = &m_slot->blk1(); break;
	case BLK2: window = &m_slot->blk2(); break;
	case BLK3: window = &m_slot->blk3(); break;
	case BLK5: window = &m_slot->blk5(); break;
	}

	if (m_window && (m_window != window))
		m_window->unmap();

	m_window = window;

	if (m_window)
		m_window->install_ram(0x0000, 0x1fff, m_ram.target());
}
