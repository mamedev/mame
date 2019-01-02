// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Mattel Intellivision controller port emulation

**********************************************************************/

#include "emu.h"
#include "ctrl.h"
// slot devices
#include "handctrl.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(INTV_CONTROL_PORT, intv_control_port_device, "intv_control_port", "Mattel Intellivision control port")


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_intv_control_port_interface - constructor
//-------------------------------------------------

device_intv_control_port_interface::device_intv_control_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_port = dynamic_cast<intv_control_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_intv_control_port_interface - destructor
//-------------------------------------------------

device_intv_control_port_interface::~device_intv_control_port_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  intv_control_port_device - constructor
//-------------------------------------------------

intv_control_port_device::intv_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, INTV_CONTROL_PORT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_device(nullptr)
{
}


//-------------------------------------------------
//  intv_control_port_device - destructor
//-------------------------------------------------

intv_control_port_device::~intv_control_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void intv_control_port_device::device_start()
{
	m_device = dynamic_cast<device_intv_control_port_interface *>(get_card_device());
}


uint8_t intv_control_port_device::read_ctrl()
{
	uint8_t data = 0;
	if (m_device)
		data |= m_device->read_ctrl();
	return data;
}


//-------------------------------------------------
//  SLOT_INTERFACE( intv_control_port_devices )
//-------------------------------------------------

void intv_control_port_devices(device_slot_interface &device)
{
	device.option_add("handctrl", INTV_HANDCTRL);
}
