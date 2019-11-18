// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom NASBUS Slot Devices

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "avc.h"
#include "floppy.h"


void nasbus_slot_cards(device_slot_interface &device)
{
	device.option_add("avc", NASCOM_AVC);
	device.option_add("floppy", NASCOM_FDC);
}
