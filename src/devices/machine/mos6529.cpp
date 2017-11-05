// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6529 Single Port Interface Adapter emulation

**********************************************************************/

#include "emu.h"
#include "mos6529.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MOS6529, mos6529_device, "mos6529", "MOS 6529")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6529_device - constructor
//-------------------------------------------------

mos6529_device::mos6529_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MOS6529, tag, owner, clock),
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
	m_p0_handler(BIT(data, 0));
	m_p1_handler(BIT(data, 1));
	m_p2_handler(BIT(data, 2));
	m_p3_handler(BIT(data, 3));
	m_p4_handler(BIT(data, 4));
	m_p5_handler(BIT(data, 5));
	m_p6_handler(BIT(data, 6));
	m_p7_handler(BIT(data, 7));
}
