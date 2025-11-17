// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Generic Keytronic serial keyboard connector

    Although Key Tronic Corporation produced many keyboards with varied and
    incompatible OEM interfaces, their most common interface uses
    full-duplex TTL-level (possibly inverted) serial communications at
    300 baud, with 8 data bits and no parity. These keyboards normally
    include buzzers to generate keyclicks when enabled and beeps when
    requested, and often feature programmable LEDs as well.

    Some of these keyboards send ASCII codes, while others send matrix scan
    codes to be decoded by the host. Systems that support multiple
    non-ASCII keyboard types can distinguish these by the ID bytes they
    send back in response to the 0x10 command.

***************************************************************************/

#include "emu.h"
#include "bus/keytronic/keytronic.h"


//**************************************************************************
//  KEYBOARD CONNECTOR DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(KEYTRONIC_CONNECTOR, keytronic_connector_device, "keytronic_connector", "Keytronic serial keyboard connector")

keytronic_connector_device::keytronic_connector_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, KEYTRONIC_CONNECTOR, tag, owner, clock)
	, device_single_card_slot_interface<device_keytronic_interface>(mconfig, *this)
	, m_ser_out_callback(*this)
	, m_kbd(nullptr)
{
}

void keytronic_connector_device::device_config_complete()
{
	m_kbd = get_card_device();
}

void keytronic_connector_device::device_start()
{
}

//**************************************************************************
//  KEYTRONIC KEYBOARD INTERFACE
//**************************************************************************

device_keytronic_interface::device_keytronic_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "keytronic")
	, m_port(device, DEVICE_SELF_OWNER)
{
}

device_keytronic_interface::~device_keytronic_interface()
{
}

//**************************************************************************
//  KEYBOARD OPTIONS
//**************************************************************************

#include "informer_kbd.h"
#include "kay_kbd.h"
#include "keytronic_l2207.h"

void ascii_terminal_keyboards(device_slot_interface &device)
{
	device.option_add("l2207", KEYTRONIC_L2207);
}

void informer_207_100_keyboards(device_slot_interface &device)
{
	device.option_add("in207100", INFORMER_207_100_KBD); // I207DEC
	device.option_add("in213", INFORMER_213_KBD); // I207IBM
	device.option_add("l2207", KEYTRONIC_L2207); // DEC 203 (doesn't work reliably on 207/100 except with clear NVRAM)
}

void informer_213_keyboards(device_slot_interface &device)
{
	device.option_add("in213", INFORMER_213_KBD);
}

void kaypro_keyboards(device_slot_interface &device)
{
	device.option_add("kayproii", KAYPROII_KEYBOARD);
	device.option_add("kaypro10", KAYPRO_10_KEYBOARD);
}
