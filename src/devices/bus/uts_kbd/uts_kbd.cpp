// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Sperry Univac UTS series keyboard port

    The UTS 20 System Description presents four types of keyboards that
    may be connected to the terminal as auxiliary input devices:
    - Typewriter Keyboard
    - Expanded Typewriter Keyboard
    - Katakana/English Keyboard
    - UTS 400-Format Keyboard

    A Magnetic Stripe Reader which can read ABA or IATA data is another
    device which can be connected to the keyboard input. This apparently
    includes a pass-through keyboard connector.

    Keyboard input is transmitted serially at 9600 baud, using 8 data bits,
    1 stop bit and odd parity. A two-byte sequence is sent for each key,
    with the (non-ASCII) keycode being contained in the second byte. The
    only other active line appears to be a ready signal (assumed to be
    active high), which the terminal drives to synchronize transmissions.
    This might even be a single line driven bidirectionally. There appear
    to be at most four wires attached to the DE-9 connector.

***************************************************************************/

#include "emu.h"
#include "bus/uts_kbd/uts_kbd.h"

#include "bus/uts_kbd/extw.h"
#include "bus/uts_kbd/400kbd.h"

//**************************************************************************
//  UTS KEYBOARD PORT DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(UTS_KEYBOARD, uts_keyboard_port_device, "uts_kbd", "UTS Keyboard Port")

uts_keyboard_port_device::uts_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, UTS_KEYBOARD, tag, owner, clock)
	, device_single_card_slot_interface<device_uts_keyboard_interface>(mconfig, *this)
	, m_rxd_callback(*this)
	, m_kbd(nullptr)
{
}

void uts_keyboard_port_device::device_config_complete()
{
	m_kbd = get_card_device();
}

void uts_keyboard_port_device::device_resolve_objects()
{
	m_rxd_callback.resolve_safe();
}

void uts_keyboard_port_device::device_start()
{
}

//**************************************************************************
//  UTS KEYBOARD INTERFACE
//**************************************************************************

device_uts_keyboard_interface::device_uts_keyboard_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "utskbd")
	, m_port(device, DEVICE_SELF_OWNER)
{
}

device_uts_keyboard_interface::~device_uts_keyboard_interface()
{
}

//**************************************************************************
//  KEYBOARD OPTIONS
//**************************************************************************

void uts10_keyboards(device_slot_interface &slot)
{
	slot.option_add("extw", UTS_EXTW_KEYBOARD);
}

void uts20_keyboards(device_slot_interface &slot)
{
	uts10_keyboards(slot);
	slot.option_add("uts400", UTS_400_KEYBOARD);
}
