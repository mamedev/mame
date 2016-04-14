// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Controller Port emulation

**********************************************************************/

#include "ctrl.h"
// slot devices
#include "joystick.h"
#include "mahjong.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NEOGEO_CONTROL_PORT = &device_creator<neogeo_control_port_device>;


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_neogeo_control_port_interface - constructor
//-------------------------------------------------

device_neogeo_control_port_interface::device_neogeo_control_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_port = dynamic_cast<neogeo_control_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_neogeo_control_port_interface - destructor
//-------------------------------------------------

device_neogeo_control_port_interface::~device_neogeo_control_port_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  neogeo_control_port_device - constructor
//-------------------------------------------------

neogeo_control_port_device::neogeo_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, NEOGEO_CONTROL_PORT, "SNK Neo Geo control port", tag, owner, clock, "neogeo_control_port", __FILE__),
						device_slot_interface(mconfig, *this), m_device(nullptr)
{
}


//-------------------------------------------------
//  ~neogeo_control_port_device - destructor
//-------------------------------------------------

neogeo_control_port_device::~neogeo_control_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neogeo_control_port_device::device_start()
{
	m_device = dynamic_cast<device_neogeo_control_port_interface *>(get_card_device());
}


UINT8 neogeo_control_port_device::read_ctrl()
{
	UINT8 data = 0;
	if (m_device)
		data |= m_device->read_ctrl();
	return data;
}

UINT8 neogeo_control_port_device::read_start_sel()
{
	UINT8 data = 0;
	if (m_device)
		data |= m_device->read_start_sel();
	return data;
}


void neogeo_control_port_device::write_ctrlsel(UINT8 data)
{
	if (m_device)
		m_device->write_ctrlsel(data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( neogeo_control_port_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( neogeo_controls )
	SLOT_INTERFACE("joy",     NEOGEO_JOYSTICK)
	SLOT_INTERFACE("mahjong", NEOGEO_MJCTRL)
SLOT_INTERFACE_END

