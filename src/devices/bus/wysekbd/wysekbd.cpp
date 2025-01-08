// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Wyse Keyboard Port interface

    The connector used for most later Wyse keyboards carries four signals:

        1  DATA
        2  VDD (+5V)
        3  CMD (or CLOCK)
        4  GND

    These keyboards generally use simple gate arrays rather than MCUs. The
    MCU in the terminal is responsible for scanning the entire key matrix
    one switch at a time, which it does by sending a series of pulses on
    CMD and shifting bits in from the DATA return line. The protocol is
    further described in U.S. Patent 4,706,068.

    Each keyboard has a different matrix layout, so the same key may appear
    at different indices on different keyboards. The one consistent feature
    is the reservation of one row for an ID code (though this is absent on
    the earliest keyboards). Some keyboards also have LEDs whose state can
    be set by sending a few additional pulses following the ID row.

***************************************************************************/

#include "emu.h"
#include "wysekbd.h"

#include "wysegakb.h"

//**************************************************************************
//  WYSE KEYBOARD PORT DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(WYSE_KEYBOARD, wyse_keyboard_port_device, "wysekbd", "Wyse Keyboard Port")

wyse_keyboard_port_device::wyse_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, WYSE_KEYBOARD, tag, owner, clock)
	, device_single_card_slot_interface<wyse_keyboard_interface>(mconfig, *this)
	, m_kbd(nullptr)
{
}

void wyse_keyboard_port_device::device_config_complete()
{
	m_kbd = get_card_device();
}

void wyse_keyboard_port_device::device_start()
{
}

//**************************************************************************
//  WYSE KEYBOARD INTERFACE
//**************************************************************************

wyse_keyboard_interface::wyse_keyboard_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "wysekbd")
{
}

wyse_keyboard_interface::~wyse_keyboard_interface()
{
}

//**************************************************************************
//  KEYBOARD OPTIONS
//**************************************************************************

void wy85_keyboards(device_slot_interface &slot)
{
	slot.option_add("wy85", WY85_KEYBOARD);
}

void wy30_keyboards(device_slot_interface &slot)
{
	slot.option_add("wy30", WY30_KEYBOARD);
}

void wy60_keyboards(device_slot_interface &slot)
{
	slot.option_add("ascii", WY60_ASCII_KEYBOARD);
	slot.option_add("at", WYSE_AT_KEYBOARD);
	slot.option_add("316x", WYSE_316X_KEYBOARD);
	slot.option_add("pce", WYSE_PCE_KEYBOARD);
	slot.option_add("pceint", WYSE_PCEINT_KEYBOARD);
}
