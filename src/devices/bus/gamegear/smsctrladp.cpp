// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Game Gear "SMS Controller Adaptor" emulation
    Also known as "Master Link" cable.

**********************************************************************/

#include "emu.h"
#include "smsctrladp.h"

#include "bus/sms_ctrl/controllers.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SMS_CTRL_ADAPTOR, sms_ctrl_adaptor_device, "sms_ctrl_adaptor", "SMS Controller Adaptor")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_ctrl_adaptor_device - constructor
//-------------------------------------------------

sms_ctrl_adaptor_device::sms_ctrl_adaptor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_CTRL_ADAPTOR, tag, owner, clock),
	device_gg_ext_port_interface(mconfig, *this),
	m_subctrl_port(*this, "ctrl"),
	m_th_state(0x40)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_ctrl_adaptor_device::device_start()
{
	save_item(NAME(m_th_state));
}


//-------------------------------------------------
//  sms_peripheral_r - sms_ctrl_adaptor read
//-------------------------------------------------

uint8_t sms_ctrl_adaptor_device::peripheral_r()
{
	uint8_t const in = m_subctrl_port->in_r();
	return BIT(in, 0, 4) | 0x10 | (BIT(in, 4) << 5) | m_th_state | (BIT(in, 5) << 7);
}


//-------------------------------------------------
//  sms_peripheral_w - sms_ctrl_adaptor write
//-------------------------------------------------

void sms_ctrl_adaptor_device::peripheral_w(uint8_t data)
{
	// FIXME: need driver state to be passed through
	uint8_t const out = (bitswap<2>(data, 6, 7) << 5) | 0x1f;
	uint8_t const mask = bitswap<2>(~data, 6, 7) << 5; // assume only driven low until this is fixed
	m_subctrl_port->out_w(out, mask);
}


WRITE_LINE_MEMBER( sms_ctrl_adaptor_device::th_pin_w )
{
	m_th_state = state ? 0x40 : 0x00;
	m_port->th_pin_w(state);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sms_ctrl_adaptor_device::device_add_mconfig(machine_config &config)
{
	// Game Gear screen is an LCD - it won't work with lightguns anyway
	SMS_CONTROL_PORT(config, m_subctrl_port, sms_control_port_devices, SMS_CTRL_OPTION_JOYPAD);
	m_subctrl_port->th_handler().set(FUNC(sms_ctrl_adaptor_device::th_pin_w));
}

