// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Kontron PSI keyboard interface

***************************************************************************/

#include "emu.h"
#include "psi_kbd.h"
#include "ergoline.h"
#include "hle.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSI_KEYBOARD_INTERFACE, psi_keyboard_bus_device, "psi_kbd", "PSI Keyboard Interface")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  psi_keyboard_bus_device - constructor
//-------------------------------------------------

psi_keyboard_bus_device::psi_keyboard_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PSI_KEYBOARD_INTERFACE, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_kbd(nullptr),
	m_rx_handler(*this),
	m_key_strobe_handler(*this),
	m_key_data(0xff)
{
}

//-------------------------------------------------
//  psi_keyboard_bus_device - destructor
//-------------------------------------------------

psi_keyboard_bus_device::~psi_keyboard_bus_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psi_keyboard_bus_device::device_start()
{
	// get connected keyboard
	m_kbd = dynamic_cast<device_psi_keyboard_interface *>(get_card_device());

	// resolve callbacks
	m_rx_handler.resolve_safe();
	m_key_strobe_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psi_keyboard_bus_device::device_reset()
{
	m_key_data = 0xff;
}

//-------------------------------------------------
//  host to module interface
//-------------------------------------------------

WRITE_LINE_MEMBER( psi_keyboard_bus_device::tx_w )
{
	if (m_kbd)
		m_kbd->tx_w(state);
}


//**************************************************************************
//  KEYBOARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_psi_keyboard_interface - constructor
//-------------------------------------------------

device_psi_keyboard_interface::device_psi_keyboard_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_host = dynamic_cast<psi_keyboard_bus_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_psi_keyboard_interface - destructor
//-------------------------------------------------

device_psi_keyboard_interface::~device_psi_keyboard_interface()
{
}


//**************************************************************************
//  SLOT INTERFACE
//**************************************************************************

void psi_keyboard_devices(device_slot_interface &device)
{
	device.option_add("ergoline", ERGOLINE_KEYBOARD);
	device.option_add("hle", PSI_HLE_KEYBOARD);
}
