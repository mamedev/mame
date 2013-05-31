/**********************************************************************

    Luxor ABC 800/802/806/1600 keyboard port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "abckb.h"

// supported devices
#include "machine/abc800kb.h"
#include "machine/abc77.h"
#include "machine/abc99.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type ABC_KEYBOARD_PORT = &device_creator<abc_keyboard_port_device>;



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abc_keyboard_interface - constructor
//-------------------------------------------------

abc_keyboard_interface::abc_keyboard_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_slot = dynamic_cast<abc_keyboard_port_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_keyboard_port_device - constructor
//-------------------------------------------------

abc_keyboard_port_device::abc_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ABC_KEYBOARD_PORT, "Luxor ABC keyboard port", tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_write_trxc(*this),
	m_write_keydown(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_keyboard_port_device::device_start()
{
	m_card = dynamic_cast<abc_keyboard_interface *>(get_card_device());

	// resolve callbacks
	m_write_trxc.resolve_safe();
	m_write_keydown.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc_keyboard_port_device::device_reset()
{
	if (m_card != NULL)
		get_card_device()->reset();
}


//-------------------------------------------------
//  rxd_r -
//-------------------------------------------------

READ_LINE_MEMBER( abc_keyboard_port_device::rxd_r )
{
	int state = 1;

	if (m_card != NULL)
		state = m_card->rxd_r();

	return state;
}


//-------------------------------------------------
//  txd_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( abc_keyboard_port_device::txd_w )
{
	if (m_card != NULL)
		m_card->txd_w(state);
}


//-------------------------------------------------
//  trxc_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( abc_keyboard_port_device::trxc_w )
{
	m_write_trxc(state);
}


//-------------------------------------------------
//  keydown_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( abc_keyboard_port_device::keydown_w )
{
	m_write_keydown(state);
}



//**************************************************************************
//  SLOT INTERFACE
//**************************************************************************

SLOT_INTERFACE_START( abc_keyboard_devices )
	SLOT_INTERFACE("abc800", ABC800_KEYBOARD)
	SLOT_INTERFACE("abc55", ABC55)
	SLOT_INTERFACE("abc77", ABC77)
	SLOT_INTERFACE("abc99", ABC99)
SLOT_INTERFACE_END
