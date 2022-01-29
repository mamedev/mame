// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Zenith Z-29 keyboard port

    This supports the custom serial interface for the H-29/Z-29 and other
    early Heath/Zenith terminals with detachable keyboards. (Keyboards for
    later Zenith terminals such as the Z-49 use a standard asynchronous
    serial protocol.)

    All printable characters outside of the numeric keypad use standard
    ASCII codes, as do Back Space, Line Feed, Return, Esc and Del. The
    following codes are assigned to other keys:

        80              Up
        81              Down
        82              Left
        83              Right
        84              Home
        85              Erase
        86              Help
        87              No Scroll
        88              Set Up
        89              Break
        8A              Caps Lock
        8B              Tab
        8C              Space Bar
        8F              (Power On)
        90              0 (Keypad)
        91              1 (Keypad)
        92              2 (Keypad)
        93              3 (Keypad)
        94              4 (Keypad)
        95              5 (Keypad)
        96              6 (Keypad)
        97              7 (Keypad)
        98              8 (Keypad)
        99              9 (Keypad)
        9A              . (Keypad)
        9B              Enter (Keypad)
        9C              - (Keypad)
        9D              , (Keypad)
        9F 80           F1
        9F 81           F2
        9F 82           F3
        9F 83           F4
        9F 84           F5
        9F 85           F6
        9F 86           F7
        9F 87           F8
        9F 88           F9

    Bit 6 is set in these codes when pressed with Shift, and bit 5 when
    pressed with Ctrl.

***************************************************************************/

#include "emu.h"
#include "keyboard.h"

#include "he191_3425.h"
#include "md_kbd.h"

//**************************************************************************
//  Z-29 KEYBOARD PORT DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(Z29_KEYBOARD, z29_keyboard_port_device, "z29_kbd", "Z-29 Keyboard Port")

z29_keyboard_port_device::z29_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, Z29_KEYBOARD, tag, owner, clock)
	, device_single_card_slot_interface<device_z29_keyboard_interface>(mconfig, *this)
	, m_keyin_callback(*this)
	, m_reset_callback(*this)
	, m_kbd(nullptr)
{
}

void z29_keyboard_port_device::device_config_complete()
{
	m_kbd = get_card_device();
}

void z29_keyboard_port_device::device_resolve_objects()
{
	m_keyin_callback.resolve_safe();
	m_reset_callback.resolve_safe();
}

void z29_keyboard_port_device::device_start()
{
}

//**************************************************************************
//  Z-29 KEYBOARD INTERFACE
//**************************************************************************

device_z29_keyboard_interface::device_z29_keyboard_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "z29kbd")
	, m_port(device, DEVICE_SELF_OWNER)
{
}

device_z29_keyboard_interface::~device_z29_keyboard_interface()
{
}

//**************************************************************************
//  KEYBOARD OPTIONS
//**************************************************************************

void z29_keyboards(device_slot_interface &slot)
{
	slot.option_add("heath", HE191_3425);
	slot.option_add("md", MD_KEYBOARD);
}
