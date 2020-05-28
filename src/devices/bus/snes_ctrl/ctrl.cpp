// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom & SNES controller port emulation

**********************************************************************/

#include "emu.h"
#include "ctrl.h"

// slot devices
#include "bcbattle.h"
#include "joypad.h"
#include "miracle.h"
#include "mouse.h"
#include "multitap.h"
#include "pachinko.h"
#include "sscope.h"
#include "twintap.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SNES_CONTROL_PORT, snes_control_port_device, "snes_control_port", "Nintendo SNES / SFC controller port")


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_snes_control_port_interface - constructor
//-------------------------------------------------

device_snes_control_port_interface::device_snes_control_port_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "snesctrl")
{
	m_port = dynamic_cast<snes_control_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_snes_control_port_interface - destructor
//-------------------------------------------------

device_snes_control_port_interface::~device_snes_control_port_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  snes_control_port_device - constructor
//-------------------------------------------------

snes_control_port_device::snes_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SNES_CONTROL_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_snes_control_port_interface>(mconfig, *this),
	m_onscreen_cb(*this),
	m_gunlatch_cb(*this),
	m_device(nullptr)
{
}


//-------------------------------------------------
//  snes_control_port_device - destructor
//-------------------------------------------------

snes_control_port_device::~snes_control_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void snes_control_port_device::device_start()
{
	m_device = get_card_device();
	m_onscreen_cb.resolve();
	m_gunlatch_cb.resolve();
}


uint8_t snes_control_port_device::read_pin4()
{
	uint8_t data = 0;
	if (m_device)
		data |= m_device->read_pin4();
	return data;
}

uint8_t snes_control_port_device::read_pin5()
{
	uint8_t data = 0;
	if (m_device)
		data |= m_device->read_pin5();
	return data;
}

void snes_control_port_device::write_strobe(uint8_t data)
{
	if (m_device)
		m_device->write_strobe(data);
}

void snes_control_port_device::write_pin6(uint8_t data)
{
	if (m_device)
		m_device->write_pin6(data);
}

void snes_control_port_device::port_poll()
{
	if (m_device)
		m_device->port_poll();
}


//-------------------------------------------------
//  SLOT_INTERFACE( snes_control_port_devices )
//-------------------------------------------------

void snes_control_port_devices(device_slot_interface &device)
{
	device.option_add("joypad", SNES_JOYPAD);
	device.option_add("mouse", SNES_MOUSE);
	device.option_add("multitap", SNES_MULTITAP);
	device.option_add("pachinko", SNES_PACHINKO);
	device.option_add("sscope", SNES_SUPERSCOPE);
	device.option_add("twintap", SNES_TWINTAP);
	device.option_add("barcode_battler", SNES_BARCODE_BATTLER);
	device.option_add("miracle_piano", SNES_MIRACLE);
}
