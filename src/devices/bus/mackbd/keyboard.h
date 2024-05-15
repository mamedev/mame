// license: BSD-3-Clause
// copyright-holders: Vas Crabb
/***************************************************************************

    Apple M0110/M0120 keyboard/keypad

***************************************************************************/
#ifndef MAME_BUS_MACKBD_KEYBOARD_H
#define MAME_BUS_MACKBD_KEYBOARD_H

#pragma once

#include "mackbd.h"

DECLARE_DEVICE_TYPE(MACKBD_M0110,  device_mac_keyboard_interface)
DECLARE_DEVICE_TYPE(MACKBD_M0110B, device_mac_keyboard_interface)
DECLARE_DEVICE_TYPE(MACKBD_M0110F, device_mac_keyboard_interface)
DECLARE_DEVICE_TYPE(MACKBD_M0110T, device_mac_keyboard_interface)
DECLARE_DEVICE_TYPE(MACKBD_M0110J, device_mac_keyboard_interface)

DECLARE_DEVICE_TYPE(MACKBD_M0120,  device_mac_keyboard_interface)
DECLARE_DEVICE_TYPE(MACKBD_M0120P, device_mac_keyboard_interface)

#endif // MAME_BUS_MACKBD_KEYBOARD_H
