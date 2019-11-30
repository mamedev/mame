// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Expansion Slot Devices

***************************************************************************/

#include "emu.h"
#include "cards.h"
#include "ram.h"
#include "winchester.h"

void apricot_expansion_cards(device_slot_interface &device)
{
	device.option_add("128k", APRICOT_128K_RAM);
	device.option_add("256k", APRICOT_256K_RAM);
	device.option_add("512k", APRICOT_512K_RAM);
	device.option_add("winchester", APRICOT_WINCHESTER);
}
