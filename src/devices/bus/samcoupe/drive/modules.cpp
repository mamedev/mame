// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe Drive Port modules

***************************************************************************/

#include "emu.h"
#include "modules.h"

#include "atom.h"
#include "floppy.h"

void samcoupe_drive_modules(device_slot_interface &device)
{
	device.option_add("atom", SAM_ATOM_HDD);
	device.option_add("floppy", SAM_FLOPPY);
}
