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
	m_p_handler{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6529_device::device_start()
{
	// resolve callbacks
	for (int i = 0; i < 8; i++)
		m_p_handler[i].resolve_safe();
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
	for (int bit = 0; bit < 8; bit++)
		m_p_handler[bit](BIT(data, bit));
}
