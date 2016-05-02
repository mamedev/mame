// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn Controller Port emulation
 
**********************************************************************/

#include "ctrl.h"
// slot devices
#include "analog.h"
#include "joy.h"
#include "joy_md.h"
#include "keybd.h"
#include "mouse.h"
#include "multitap.h"
#include "pointer.h"
#include "racing.h"
#include "segatap.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type SATURN_CONTROL_PORT = &device_creator<saturn_control_port_device>;


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_saturn_control_port_interface - constructor
//-------------------------------------------------

device_saturn_control_port_interface::device_saturn_control_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_port = dynamic_cast<saturn_control_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_saturn_control_port_interface - destructor
//-------------------------------------------------

device_saturn_control_port_interface::~device_saturn_control_port_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saturn_control_port_device - constructor
//-------------------------------------------------

saturn_control_port_device::saturn_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, SATURN_CONTROL_PORT, "Sega Saturn control port", tag, owner, clock, "saturn_control_port", __FILE__),
						device_slot_interface(mconfig, *this), m_device(nullptr)
{
}


//-------------------------------------------------
//  ~saturn_control_port_device - destructor
//-------------------------------------------------

saturn_control_port_device::~saturn_control_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saturn_control_port_device::device_start()
{
	m_device = dynamic_cast<device_saturn_control_port_interface *>(get_card_device());
}


UINT8 saturn_control_port_device::read_status()
{
	UINT8 data = 0;
	if (m_device)
		data |= m_device->read_status();
	return data;
}

// Notice that the variable idx is only used by the multitap / segatap adapters
// Otherwise, any value is ignored and the unique controller ID is returned
UINT8 saturn_control_port_device::read_id(int idx)
{
	UINT8 data = 0;
	if (m_device)
		data |= m_device->read_id(idx);
	return data;
}

UINT8 saturn_control_port_device::read_ctrl(UINT8 offset)
{
	UINT8 data = 0;
	if (m_device)
		data |= m_device->read_ctrl(offset);
	return data;
}

UINT16 saturn_control_port_device::read_direct()
{
	UINT16 data = 0;
	if (m_device)
		data |= m_device->read_direct();
	return data;
}


//-------------------------------------------------
//  SLOT_INTERFACE( saturn_controls )
//-------------------------------------------------

SLOT_INTERFACE_START( saturn_controls )
	SLOT_INTERFACE("joypad",    SATURN_JOY)
	SLOT_INTERFACE("racing",    SATURN_WHEEL)
	SLOT_INTERFACE("analog",    SATURN_ANALOG)
//	SLOT_INTERFACE("lightgun",  SATURN_LIGHTGUN)
	SLOT_INTERFACE("trackball", SATURN_TRACK)
	SLOT_INTERFACE("keyboard",  SATURN_KEYBD)
	SLOT_INTERFACE("joy_md3",   SATURN_JOYMD3B)
	SLOT_INTERFACE("joy_md6",   SATURN_JOYMD6B)
	SLOT_INTERFACE("mouse",     SATURN_MOUSE)
	SLOT_INTERFACE("multitap",  SATURN_MULTITAP)
	SLOT_INTERFACE("segatap",   SATURN_SEGATAP)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( saturn_joys )
	SLOT_INTERFACE("joypad",    SATURN_JOY)
SLOT_INTERFACE_END

