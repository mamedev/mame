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

device_snes_control_port_interface::device_snes_control_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
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
	device_slot_interface(mconfig, *this), m_device(nullptr)
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
	m_device = dynamic_cast<device_snes_control_port_interface *>(get_card_device());
	m_onscreen_cb.bind_relative_to(*owner());
	m_gunlatch_cb.bind_relative_to(*owner());
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

SLOT_INTERFACE_START( snes_control_port_devices )
	SLOT_INTERFACE("joypad", SNES_JOYPAD)
	SLOT_INTERFACE("mouse", SNES_MOUSE)
	SLOT_INTERFACE("multitap", SNES_MULTITAP)
	SLOT_INTERFACE("pachinko", SNES_PACHINKO)
	SLOT_INTERFACE("sscope", SNES_SUPERSCOPE)
	SLOT_INTERFACE("twintap", SNES_TWINTAP)
	SLOT_INTERFACE("barcode_battler", SNES_BARCODE_BATTLER)
	SLOT_INTERFACE("miracle_piano", SNES_MIRACLE)
SLOT_INTERFACE_END
