// license: BSD-3-Clause
// copyright-holders: Vas Crabb
/***************************************************************************

    mitsumi.h

    Mitsumi keyboards with MOS MCU and 15-row matrix:
    * A500 keyboard (watchdog, crystal, reset line)
    * A600 keyboard (no keypad, no watchdog, crystal, reset line)
    * A2000/A3000/A4000/CDTV keyboard (watchdog, ceramic resonator)

***************************************************************************/
#ifndef MAME_BUS_AMIGA_KEYBOARD_MITSUMI_H
#define MAME_BUS_AMIGA_KEYBOARD_MITSUMI_H

#include "keyboard.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(A500_KBD_US, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A500_KBD_DE, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A500_KBD_FR, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A500_KBD_IT, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A500_KBD_SE, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A500_KBD_ES, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A500_KBD_DK, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A500_KBD_CH, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A500_KBD_NO, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A500_KBD_GB, device_amiga_keyboard_interface)

DECLARE_DEVICE_TYPE(A1000_KBD_US, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A1000_KBD_DE, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A1000_KBD_FR, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A1000_KBD_IT, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A1000_KBD_SE, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A1000_KBD_DK, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A1000_KBD_GB, device_amiga_keyboard_interface)

DECLARE_DEVICE_TYPE(A600_KBD_US, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A600_KBD_DE, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A600_KBD_FR, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A600_KBD_IT, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A600_KBD_SE, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A600_KBD_ES, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A600_KBD_DK, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A600_KBD_CH, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A600_KBD_NO, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A600_KBD_GB, device_amiga_keyboard_interface)

DECLARE_DEVICE_TYPE(A2000_KBD_US, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A2000_KBD_DE, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A2000_KBD_FR, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A2000_KBD_IT, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A2000_KBD_SE, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A2000_KBD_ES, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A2000_KBD_DK, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A2000_KBD_CH, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A2000_KBD_NO, device_amiga_keyboard_interface)
DECLARE_DEVICE_TYPE(A2000_KBD_GB, device_amiga_keyboard_interface)

#endif // MAME_BUS_AMIGA_KEYBOARD_MITSUMI_H
