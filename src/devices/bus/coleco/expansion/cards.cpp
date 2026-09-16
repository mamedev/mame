// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Colecovision Expansion Slot cards

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "sgm.h"


void coleco_expansion_cards(device_slot_interface &device)
{
	device.option_add("sgm", COLECO_SGM);
}
