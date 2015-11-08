// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Memory Expansion Slot Devices

***************************************************************************/

#include "carts.h"

SLOT_INTERFACE_START( memexp_slot_carts )
	SLOT_INTERFACE("floppy", FLOPPY_CONTROLLER)
	SLOT_INTERFACE("laser110_16k", LASER110_16K)
	SLOT_INTERFACE("laser210_16k", LASER210_16K)
	SLOT_INTERFACE("laser310_16k", LASER310_16K)
	SLOT_INTERFACE("laser_64k", LASER_64K)
	SLOT_INTERFACE("rs232", RS232_INTERFACE)
	SLOT_INTERFACE("wordpro", WORDPRO)
SLOT_INTERFACE_END
