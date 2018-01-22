// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ I/O Expansion Slot Devices

***************************************************************************/

#include "emu.h"
#include "carts.h"

#include "joystick.h"
#include "printer.h"


SLOT_INTERFACE_START( vtech_ioexp_slot_carts )
	SLOT_INTERFACE("joystick", VTECH_JOYSTICK_INTERFACE)
	SLOT_INTERFACE("printer", VTECH_PRINTER_INTERFACE)
SLOT_INTERFACE_END
