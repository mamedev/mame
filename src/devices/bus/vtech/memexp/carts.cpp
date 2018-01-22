// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Memory Expansion Slot Devices

***************************************************************************/

#include "emu.h"
#include "carts.h"

#include "floppy.h"
#include "memory.h"
#include "rs232.h"
#include "rtty.h"
#include "wordpro.h"


SLOT_INTERFACE_START( vtech_memexp_carts )
	SLOT_INTERFACE("floppy",       VTECH_FLOPPY_CONTROLLER)
	SLOT_INTERFACE("laser110_16k", VTECH_LASER110_16K)
	SLOT_INTERFACE("laser210_16k", VTECH_LASER210_16K)
	SLOT_INTERFACE("laser310_16k", VTECH_LASER310_16K)
	SLOT_INTERFACE("laser_64k",    VTECH_LASER_64K)
	SLOT_INTERFACE("rs232",        VTECH_RS232_INTERFACE)
	SLOT_INTERFACE("rtty",         VTECH_RTTY_INTERFACE)
	SLOT_INTERFACE("wordpro",      VTECH_WORDPRO)
SLOT_INTERFACE_END
