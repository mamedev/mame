// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Keyboard Interface

***************************************************************************/

#include "emu.h"
#include "keyboard.h"
#include "hle.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(APRICOT_KEYBOARD_INTERFACE, apricot_keyboard_bus_device, "apricot_kbd", "Apricot Keyboard Interface")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  apricot_keyboard_bus_device - constructor
//-------------------------------------------------

apricot_keyboard_bus_device::apricot_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APRICOT_KEYBOARD_INTERFACE, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_kbd(nullptr),
	m_in_handler(*this)
{
}

//-------------------------------------------------
//  apricot_keyboard_bus_device - destructor
//-------------------------------------------------

apricot_keyboard_bus_device::~apricot_keyboard_bus_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apricot_keyboard_bus_device::device_start()
{
	// get connected keyboard
	m_kbd = dynamic_cast<device_apricot_keyboard_interface *>(get_card_device());

	// resolve callbacks
	m_in_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apricot_keyboard_bus_device::device_reset()
{
}

//-------------------------------------------------
//  host to module interface
//-------------------------------------------------

WRITE_LINE_MEMBER( apricot_keyboard_bus_device::out_w )
{
		if (m_kbd)
			m_kbd->out_w(state);
}


//**************************************************************************
//  KEYBOARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_apricot_keyboard_interface - constructor
//-------------------------------------------------

device_apricot_keyboard_interface::device_apricot_keyboard_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_host = dynamic_cast<apricot_keyboard_bus_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_apricot_keyboard_interface - destructor
//-------------------------------------------------

device_apricot_keyboard_interface::~device_apricot_keyboard_interface()
{
}


//**************************************************************************
//  SLOT INTERFACE
//**************************************************************************

void apricot_keyboard_devices(device_slot_interface &device)
{
	device.option_add("hle", APRICOT_KEYBOARD_HLE);
}
