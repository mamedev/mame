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

#include "emu.h"
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

DEFINE_DEVICE_TYPE(NEOGEO_CONTROL_PORT,        neogeo_control_port_device,   "neogeo_control_port", "SNK Neo Geo controller port")
DEFINE_DEVICE_TYPE(NEOGEO_CTRL_EDGE_CONNECTOR, neogeo_ctrl_edge_port_device, "neogeo_ctrl_edge",    "SNK Neo Geo Edge Connector (Controller)")


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_neogeo_control_port_interface - constructor
//-------------------------------------------------

device_neogeo_control_port_interface::device_neogeo_control_port_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "neogeoctrl")
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

device_neogeo_ctrl_edge_interface::device_neogeo_ctrl_edge_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "neogeoedge")
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

neogeo_control_port_device::neogeo_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEOGEO_CONTROL_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_neogeo_control_port_interface>(mconfig, *this),
	m_device(nullptr)
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
	m_device = get_card_device();
}


uint8_t neogeo_control_port_device::read_ctrl()
{
	return m_device ? m_device->read_ctrl() : 0xff;
}

uint8_t neogeo_control_port_device::read_start_sel()
{
	return m_device ? m_device->read_start_sel() : 0xff;
}


void neogeo_control_port_device::write_ctrlsel(uint8_t data)
{
	if (m_device)
		m_device->write_ctrlsel(data);
}


//-------------------------------------------------
//  neogeo_ctrl_edge_port_device - constructor
//-------------------------------------------------

neogeo_ctrl_edge_port_device::neogeo_ctrl_edge_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEOGEO_CTRL_EDGE_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface<device_neogeo_ctrl_edge_interface>(mconfig, *this),
	m_device(nullptr)
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
	m_device = get_card_device();
}


uint8_t neogeo_ctrl_edge_port_device::in0_r()
{
	return m_device ? m_device->in0_r() : 0xff;
}

uint8_t neogeo_ctrl_edge_port_device::in1_r()
{
	return m_device ? m_device->in1_r() : 0xff;
}

uint8_t neogeo_ctrl_edge_port_device::read_start_sel()
{
	return m_device ? m_device->read_start_sel() : 0xff;
}

void neogeo_ctrl_edge_port_device::write_ctrlsel(uint8_t data)
{
	if (m_device)
		m_device->write_ctrlsel(data);
}



//-------------------------------------------------
//  SLOT_INTERFACE( neogeo_control_port_devices )
//-------------------------------------------------

void neogeo_controls(device_slot_interface &device)
{
	device.option_add("joy",     NEOGEO_JOY);
	device.option_add("mahjong", NEOGEO_MJCTRL);
}

void neogeo_arc_edge(device_slot_interface &device)
{
	device.option_add("joy",     NEOGEO_JOY_AC);
}

void neogeo_arc_edge_fixed(device_slot_interface &device)
{
	device.option_add("joy",     NEOGEO_JOY_AC);
	device.option_add("dial",    NEOGEO_DIAL);
	device.option_add("irrmaze", NEOGEO_IRRMAZE);
	device.option_add("kiz4p",   NEOGEO_KIZ4P);
}

void neogeo_arc_pin15(device_slot_interface &device)
{
	device.option_add("mahjong", NEOGEO_MJCTRL_AC);
	device.option_add("joy",     NEOGEO_JOY);
}
