// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Expansion Carts

***************************************************************************/

#include "emu.h"
#include "carts.h"
#include "floppy.h"


void cg_exp_slot_carts(device_slot_interface &device)
{
	device.option_add("floppy", CGENIE_FDC);
}
