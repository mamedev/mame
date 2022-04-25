// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe Mouse Port

    8-pin connector

***************************************************************************/

#include "emu.h"
#include "mouseport.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAMCOUPE_MOUSE_PORT, samcoupe_mouse_port_device, "samcoupe_mouse_port", "SAM Coupe Mouse Port")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  samcoupe_mouse_port_device - constructor
//-------------------------------------------------

samcoupe_mouse_port_device::samcoupe_mouse_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAMCOUPE_MOUSE_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_samcoupe_mouse_interface>(mconfig, *this),
	m_mseint_handler(*this),
	m_module(nullptr)
{
}

//-------------------------------------------------
//  samcoupe_mouse_port_device - destructor
//-------------------------------------------------

samcoupe_mouse_port_device::~samcoupe_mouse_port_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void samcoupe_mouse_port_device::device_start()
{
	// get inserted module
	m_module = get_card_device();

	// resolve callbacks
	m_mseint_handler.resolve_safe();
}

//-------------------------------------------------
//  host to module interface
//-------------------------------------------------

uint8_t samcoupe_mouse_port_device::read()
{
	if (m_module)
		return m_module->read();

	return 0xff;
}


//**************************************************************************
//  MODULE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_samcoupe_mouse_interface - constructor
//-------------------------------------------------

device_samcoupe_mouse_interface::device_samcoupe_mouse_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "samcoupemouse")
{
	m_port = dynamic_cast<samcoupe_mouse_port_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_samcoupe_mouse_interface - destructor
//-------------------------------------------------

device_samcoupe_mouse_interface::~device_samcoupe_mouse_interface()
{
}
