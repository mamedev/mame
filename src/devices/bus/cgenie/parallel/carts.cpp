// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Parallel Carts

***************************************************************************/

#include "emu.h"
#include "carts.h"
#include "joystick.h"
#include "printer.h"


SLOT_INTERFACE_START( cg_parallel_slot_carts )
	SLOT_INTERFACE("joystick", CGENIE_JOYSTICK)
	SLOT_INTERFACE("printer", CGENIE_PRINTER)
SLOT_INTERFACE_END
