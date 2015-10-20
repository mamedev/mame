// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ I/O Expansion Slot Devices

***************************************************************************/

#include "carts.h"

SLOT_INTERFACE_START( ioexp_slot_carts )
	SLOT_INTERFACE("joystick", JOYSTICK_INTERFACE)
	SLOT_INTERFACE("printer", PRINTER_INTERFACE)
SLOT_INTERFACE_END
