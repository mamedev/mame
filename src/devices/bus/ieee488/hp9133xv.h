// license:BSD-3-Clause
// copyright-holders: Pascal Gervaix
/*********************************************************************

    hp9133xv.h

    HP 9133V/XV combined disc drive, low level emulation.

    Each model is a unit: a Winchester board and, except on the 9134XV, a
    floppy board, behind one HP-IB connector; each board is a separate
    HP-IB device with its own 68B09, firmware and bus address. One type per
    model or option presets the J3 straps of the Winchester board:
      - GPIB_HP9133V       4.8M Winchester + floppy
      - GPIB_HP9133V_004   4.6M Winchester as 4 volumes + floppy
      - GPIB_HP9133XV      14.5M Winchester + floppy
      - GPIB_HP9134XV      14.5M Winchester, no floppy board
      - GPIB_HP9133XV_010  10M Winchester (option 010) + floppy

*********************************************************************/

#ifndef MAME_BUS_IEEE488_HP9133XV_H
#define MAME_BUS_IEEE488_HP9133XV_H

#pragma once

#include "ieee488.h"

DECLARE_DEVICE_TYPE(GPIB_HP9133XV, device_t)
DECLARE_DEVICE_TYPE(GPIB_HP9133V, device_t)
DECLARE_DEVICE_TYPE(GPIB_HP9133V_004, device_t)
DECLARE_DEVICE_TYPE(GPIB_HP9134XV, device_t)
DECLARE_DEVICE_TYPE(GPIB_HP9133XV_010, device_t)

#endif // MAME_BUS_IEEE488_HP9133XV_H
