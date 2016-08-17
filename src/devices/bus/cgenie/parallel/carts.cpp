// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Parallel Carts

***************************************************************************/

#include "carts.h"

SLOT_INTERFACE_START( parallel_slot_carts )
	SLOT_INTERFACE("joystick", CGENIE_JOYSTICK)
	SLOT_INTERFACE("printer", CGENIE_PRINTER)
SLOT_INTERFACE_END
