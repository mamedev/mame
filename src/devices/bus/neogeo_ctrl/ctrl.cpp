// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Controller Port emulation

    This actually covers two separate piece of hardware of Neo Geo system:
    - The 15-pin controller ports that are used for controllers in the
      AES home system and for mahjong controllers in the MVS arcade PCB
    - The controller part of the main edge connector that is used for
      joystick inputs in the MVS arcade PCB

    Technically, the latter is not a configurable slot, because it's not
    a component that arcade operators could simply change with a different
    controller, but this implementation allows for simpler code.

**********************************************************************/

#include "ctrl.h"
// slot devices
#include "joystick.h"
#include "mahjong.h"
#include "dial.h"
#include "irrmaze.h"
#include "kizuna4p.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NEOGEO_CONTROL_PORT = &device_creator<neogeo_control_port_device>;
const device_type NEOGEO_CTRL_EDGE_CONNECTOR = &device_creator<neogeo_ctrl_edge_port_device>;


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

//-------------------------------------------------
//  device_neogeo_ctrl_edge_interface - constructor
//-------------------------------------------------

device_neogeo_ctrl_edge_interface::device_neogeo_ctrl_edge_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_port = dynamic_cast<neogeo_ctrl_edge_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_neogeo_ctrl_edge_interface - destructor
//-------------------------------------------------

device_neogeo_ctrl_edge_interface::~device_neogeo_ctrl_edge_interface()
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
	UINT8 data = 0xff;
	if (m_device)
		data &= m_device->read_ctrl();
	return data;
}

UINT8 neogeo_control_port_device::read_start_sel()
{
	UINT8 data = 0xff;
	if (m_device)
		data &= m_device->read_start_sel();
	return data;
}


void neogeo_control_port_device::write_ctrlsel(UINT8 data)
{
	if (m_device)
		m_device->write_ctrlsel(data);
}


//-------------------------------------------------
//  neogeo_ctrl_edge_port_device - constructor
//-------------------------------------------------

neogeo_ctrl_edge_port_device::neogeo_ctrl_edge_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
							device_t(mconfig, NEOGEO_CTRL_EDGE_CONNECTOR, "SNK Neo Geo Edge Connector (Controller)", tag, owner, clock, "neogeo_ctrl_edge", __FILE__),
							device_slot_interface(mconfig, *this), m_device(nullptr)
{
}


//-------------------------------------------------
//  ~neogeo_ctrl_edge_port_device - destructor
//-------------------------------------------------

neogeo_ctrl_edge_port_device::~neogeo_ctrl_edge_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neogeo_ctrl_edge_port_device::device_start()
{
	m_device = dynamic_cast<device_neogeo_ctrl_edge_interface *>(get_card_device());
}


READ8_MEMBER(neogeo_ctrl_edge_port_device::in0_r)
{
	UINT8 data = 0xff;
	if (m_device)
		data &= m_device->in0_r(space, offset, mem_mask);
	return data;
}

READ8_MEMBER(neogeo_ctrl_edge_port_device::in1_r)
{
	UINT8 data = 0xff;
	if (m_device)
		data &= m_device->in1_r(space, offset, mem_mask);
	return data;
}

UINT8 neogeo_ctrl_edge_port_device::read_start_sel()
{
	UINT8 data = 0xff;
	if (m_device)
		data &= m_device->read_start_sel();
	return data;
}

void neogeo_ctrl_edge_port_device::write_ctrlsel(UINT8 data)
{
	if (m_device)
		m_device->write_ctrlsel(data);
}



//-------------------------------------------------
//  SLOT_INTERFACE( neogeo_control_port_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( neogeo_controls )
	SLOT_INTERFACE("joy",     NEOGEO_JOY)
	SLOT_INTERFACE("mahjong", NEOGEO_MJCTRL)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( neogeo_arc_edge )
	SLOT_INTERFACE("joy",     NEOGEO_JOY_AC)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( neogeo_arc_edge_fixed )
	SLOT_INTERFACE("joy",     NEOGEO_JOY_AC)
	SLOT_INTERFACE("dial",    NEOGEO_DIAL)
	SLOT_INTERFACE("irrmaze", NEOGEO_IRRMAZE)
	SLOT_INTERFACE("kiz4p",   NEOGEO_KIZ4P)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( neogeo_arc_pin15 )
	SLOT_INTERFACE("mahjong", NEOGEO_MJCTRL)
SLOT_INTERFACE_END
