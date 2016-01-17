// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6529 Single Port Interface Adapter emulation

**********************************************************************/

#include "mos6529.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type MOS6529 = &device_creator<mos6529_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6529_device - constructor
//-------------------------------------------------

mos6529_device::mos6529_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MOS6529, "MOS6529", tag, owner, clock, "mos6529", __FILE__),
	m_input(0),
	m_p0_handler(*this),
	m_p1_handler(*this),
	m_p2_handler(*this),
	m_p3_handler(*this),
	m_p4_handler(*this),
	m_p5_handler(*this),
	m_p6_handler(*this),
	m_p7_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6529_device::device_start()
{
	// resolve callbacks
	m_p0_handler.resolve_safe();
	m_p1_handler.resolve_safe();
	m_p2_handler.resolve_safe();
	m_p3_handler.resolve_safe();
	m_p4_handler.resolve_safe();
	m_p5_handler.resolve_safe();
	m_p6_handler.resolve_safe();
	m_p7_handler.resolve_safe();
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( mos6529_device::read )
{
	return m_input;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( mos6529_device::write )
{
	m_p0_handler((data>>0)&1);
	m_p1_handler((data>>1)&1);
	m_p2_handler((data>>2)&1);
	m_p3_handler((data>>3)&1);
	m_p4_handler((data>>4)&1);
	m_p5_handler((data>>5)&1);
	m_p6_handler((data>>6)&1);
	m_p7_handler((data>>7)&1);
}
