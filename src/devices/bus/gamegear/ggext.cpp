// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Game Gear EXT port emulation
    Also known as Gear-to-Gear (or VS, in Japan) cable connector

**********************************************************************/

#include "emu.h"
#include "ggext.h"
// slot devices
#include "smsctrladp.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(GG_EXT_PORT, gg_ext_port_device, "gg_ext_port", "Game Gear EXT Port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_gg_ext_port_interface - constructor
//-------------------------------------------------

device_gg_ext_port_interface::device_gg_ext_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_port = dynamic_cast<gg_ext_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_gg_ext_port_interface - destructor
//-------------------------------------------------

device_gg_ext_port_interface::~device_gg_ext_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  gg_ext_port_device - constructor
//-------------------------------------------------

gg_ext_port_device::gg_ext_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GG_EXT_PORT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_device(nullptr),
	m_th_pin_handler(*this)
{
}


//-------------------------------------------------
//  gg_ext_port_device - destructor
//-------------------------------------------------

gg_ext_port_device::~gg_ext_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gg_ext_port_device::device_start()
{
	m_device = dynamic_cast<device_gg_ext_port_interface *>(get_card_device());

	m_th_pin_handler.resolve_safe();
}


uint8_t gg_ext_port_device::port_r()
{
	uint8_t data = 0xff;
	if (m_device)
		data = m_device->peripheral_r();
	return data;
}

void gg_ext_port_device::port_w( uint8_t data )
{
	if (m_device)
		m_device->peripheral_w(data);
}


void gg_ext_port_device::th_pin_w(int state)
{
	m_th_pin_handler(state);
}


//-------------------------------------------------
//  SLOT_INTERFACE( gg_ext_port_devices )
//-------------------------------------------------

void gg_ext_port_devices(device_slot_interface &device)
{
	device.option_add("smsctrladp", SMS_CTRL_ADAPTOR);
}
