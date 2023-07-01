// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    mc-68000-Computer System Bus Cards

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "floppy.h"
#include "ram.h"

void mc68000_sysbus_cards(device_slot_interface &device)
{
	device.option_add("floppy", MC68000_FLOPPY);
	device.option_add("ram", MC68000_RAM);
}
