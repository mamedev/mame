// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Colecovision Expansion Slot cards

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "lundy_speech.h"
#include "sgm.h"


void coleco_expansion_cards(device_slot_interface &device)
{
	device.option_add("lundy_speech", COLECO_LUNDY_SPEECH);
	device.option_add("sgm", COLECO_SGM);
}
