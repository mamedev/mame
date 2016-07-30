// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Keyboard Interface

***************************************************************************/

#include "keyboard.h"
#include "hle.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type APRICOT_KEYBOARD_INTERFACE = &device_creator<apricot_keyboard_bus_device>;


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  apricot_keyboard_bus_device - constructor
//-------------------------------------------------

apricot_keyboard_bus_device::apricot_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, APRICOT_KEYBOARD_INTERFACE, "Apricot Keyboard Interface", tag, owner, clock, "apricot_kbd", __FILE__),
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

SLOT_INTERFACE_START( apricot_keyboard_devices )
	SLOT_INTERFACE("hle", APRICOT_KEYBOARD_HLE)
SLOT_INTERFACE_END
