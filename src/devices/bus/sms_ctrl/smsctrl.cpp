// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System controller port emulation

**********************************************************************/

#include "emu.h"
#include "screen.h"
#include "smsctrl.h"

// slot devices
#include "joypad.h"
#include "lphaser.h"
#include "paddle.h"
#include "sports.h"
#include "sportsjp.h"
#include "rfu.h"
#include "multitap.h"
#include "graphic.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SMS_CONTROL_PORT, sms_control_port_device, "sms_control_port", "Sega SMS controller port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_sms_control_port_interface - constructor
//-------------------------------------------------

device_sms_control_port_interface::device_sms_control_port_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "smsctrl")
{
	m_port = dynamic_cast<sms_control_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_sms_control_port_interface - destructor
//-------------------------------------------------

device_sms_control_port_interface::~device_sms_control_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_control_port_device - constructor
//-------------------------------------------------

sms_control_port_device::sms_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_CONTROL_PORT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_device(nullptr),
	m_th_pin_handler(*this)
{
}


//-------------------------------------------------
//  sms_control_port_device - destructor
//-------------------------------------------------

sms_control_port_device::~sms_control_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_control_port_device::device_start()
{
	m_device = dynamic_cast<device_sms_control_port_interface *>(get_card_device());

	m_th_pin_handler.resolve_safe();
}


uint8_t sms_control_port_device::port_r()
{
	uint8_t data = 0xff;
	if (m_device)
		data = m_device->peripheral_r();
	return data;
}

void sms_control_port_device::port_w( uint8_t data )
{
	if (m_device)
		m_device->peripheral_w(data);
}


void sms_control_port_device::th_pin_w(int state)
{
	m_th_pin_handler(state);
}


//-------------------------------------------------
//  SLOT_INTERFACE( sms_control_port_devices )
//-------------------------------------------------

void sms_control_port_devices(device_slot_interface &device)
{
	device.option_add("joypad", SMS_JOYPAD);
	device.option_add("lphaser", SMS_LIGHT_PHASER);
	device.option_add("paddle", SMS_PADDLE);
	device.option_add("sportspad", SMS_SPORTS_PAD);
	device.option_add("sportspadjp", SMS_SPORTS_PAD_JP);
	device.option_add("rapidfire", SMS_RAPID_FIRE);
	device.option_add("multitap", SMS_MULTITAP);
	device.option_add("graphic", SMS_GRAPHIC);
}
