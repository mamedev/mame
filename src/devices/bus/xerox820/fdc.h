// license:BSD-3-Clause
// copyright-holders:Dave Rand
/**********************************************************************

    Xerox 820-II FD1797 floppy controller daughterboard

**********************************************************************/

#ifndef MAME_BUS_XEROX820_FDC_H
#define MAME_BUS_XEROX820_FDC_H

#pragma once

#include "dbslot.h"


// The FD1797 floppy-controller daughterboard and its 5.25"/RX024-box variants
// are private to fdc.cpp; they are exposed only through the dbslot card
// interface (paired with DEFINE_DEVICE_TYPE_PRIVATE in the source file).
DECLARE_DEVICE_TYPE(XEROX820_FDC,      device_xerox820_dbslot_card_interface) // FD1797, 8" drives
DECLARE_DEVICE_TYPE(XEROX820_FDC5,     device_xerox820_dbslot_card_interface) // FD1797, 5.25" drives
DECLARE_DEVICE_TYPE(XEROX820_FDC_BOX5, device_xerox820_dbslot_card_interface) // FD1797 + 16/8 RX024 5.25" box

#endif // MAME_BUS_XEROX820_FDC_H
