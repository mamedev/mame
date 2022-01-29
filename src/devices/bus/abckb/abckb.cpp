// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 800/802/806/1600 keyboard port emulation

**********************************************************************/

#include "emu.h"
#include "abckb.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ABC_KEYBOARD_PORT, abc_keyboard_port_device, "abc_keyboard_port", "Luxor ABC keyboard port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abc_keyboard_interface - constructor
//-------------------------------------------------

abc_keyboard_interface::abc_keyboard_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "abckb")
{
	m_slot = dynamic_cast<abc_keyboard_port_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_keyboard_port_device - constructor
//-------------------------------------------------

abc_keyboard_port_device::abc_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ABC_KEYBOARD_PORT, tag, owner, clock),
	device_single_card_slot_interface<abc_keyboard_interface>(mconfig, *this),
	m_out_rx_handler(*this),
	m_out_trxc_handler(*this),
	m_out_keydown_handler(*this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_keyboard_port_device::device_start()
{
	m_card = get_card_device();

	// resolve callbacks
	m_out_rx_handler.resolve_safe();
	m_out_trxc_handler.resolve_safe();
	m_out_keydown_handler.resolve_safe();
}


//-------------------------------------------------
//  write_rx -
//-------------------------------------------------

WRITE_LINE_MEMBER( abc_keyboard_port_device::write_rx )
{
	m_out_rx_handler(state);
}


//-------------------------------------------------
//  txd_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( abc_keyboard_port_device::txd_w )
{
	if (m_card)
		m_card->txd_w(state);
}


//-------------------------------------------------
//  trxc_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( abc_keyboard_port_device::trxc_w )
{
	m_out_trxc_handler(state);
}


//-------------------------------------------------
//  keydown_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( abc_keyboard_port_device::keydown_w )
{
	m_out_keydown_handler(state);
}



//**************************************************************************
//  SLOT INTERFACE
//**************************************************************************

#include "abc800kb.h"
#include "abc77.h"
#include "abc99.h"

void abc_keyboard_devices(device_slot_interface &device)
{
	device.option_add("abc800", ABC800_KEYBOARD);
	device.option_add("abc55", ABC55);
	device.option_add("abc77", ABC77);
	device.option_add("abc99", ABC99);
}
