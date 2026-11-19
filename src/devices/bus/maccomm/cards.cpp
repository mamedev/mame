// license:BSD-3-Clause
// copyright-holders:Tim Douglas
/***************************************************************************

    Apple Communication Slot cards

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "sonicsmc.h"


void mac_comm_cards(device_slot_interface &device)
{
	device.option_add("sonicsmc", MAC_COMM_SONICSMC);
}
