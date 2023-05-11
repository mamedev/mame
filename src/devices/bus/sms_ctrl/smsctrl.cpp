// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Sega 7-bit I/O port emulation

**********************************************************************/

#include "emu.h"
#include "smsctrl.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SMS_CONTROL_PORT, sms_control_port_device, "sms_control_port", "Sega 9-pin I/O port")



//**************************************************************************
//  CONTROLLER INTERFACE
//**************************************************************************

device_sms_control_interface::device_sms_control_interface(machine_config const &mconfig, device_t &device) :
	device_interface(device, "smsctrl"),
	m_port(dynamic_cast<sms_control_port_device *>(device.owner()))
{
}


device_sms_control_interface::~device_sms_control_interface()
{
}


u8 device_sms_control_interface::in_r()
{
	return 0xff;
}


void device_sms_control_interface::out_w(u8 data, u8 mem_mask)
{
}



//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

sms_control_port_device::sms_control_port_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SMS_CONTROL_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_sms_control_interface>(mconfig, *this),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_th_handler(*this),
	m_controller(nullptr)
{
}


sms_control_port_device::~sms_control_port_device()
{
}


void sms_control_port_device::device_resolve_objects()
{
	m_th_handler.resolve_safe();
}


void sms_control_port_device::device_start()
{
	m_controller = get_card_device();
}
