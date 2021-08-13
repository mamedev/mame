// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    ACT Apricot XEN Video Expansion Slot Devices

***************************************************************************/

#include "emu.h"
#include "cards.h"
#include "mono.h"

void apricot_video_cards(device_slot_interface &device)
{
	device.option_add("mono", APRICOT_MONO_DISPLAY);
}
