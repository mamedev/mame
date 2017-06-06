// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Expansion Carts

***************************************************************************/

#include "emu.h"
#include "carts.h"
#include "floppy.h"


SLOT_INTERFACE_START( cg_exp_slot_carts )
	SLOT_INTERFACE("floppy", CGENIE_FDC)
SLOT_INTERFACE_END
