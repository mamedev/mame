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

device_intv_control_port_interface::device_intv_control_port_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "intvctrl")
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
	device_single_card_slot_interface<device_intv_control_port_interface>(mconfig, *this),
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
	m_device = get_card_device();
}


//-------------------------------------------------
//  SLOT_INTERFACE( intv_control_port_devices )
//-------------------------------------------------

void intv_control_port_devices(device_slot_interface &device)
{
	device.option_add("handctrl", INTV_HANDCTRL);
}
