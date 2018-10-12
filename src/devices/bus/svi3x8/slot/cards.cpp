// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SVI 318/328 Slot Cards

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "sv801.h"
#include "sv802.h"
#include "sv803.h"
#include "sv805.h"
#include "sv806.h"
#include "sv807.h"


void svi_slot_cards(device_slot_interface &device)
{
	device.option_add("sv801", SV801);
	device.option_add("sv802", SV802);
	device.option_add("sv803", SV803);
	device.option_add("sv805", SV805);
	device.option_add("sv806", SV806);
	device.option_add("sv807", SV807);
}

// The single slot expander doesn't support the disk controller, since
// it requires its own power supply to power the disk drives
void sv602_slot_cards(device_slot_interface &device)
{
	device.option_add("sv802", SV802);
	device.option_add("sv803", SV803);
	device.option_add("sv805", SV805);
	device.option_add("sv806", SV806);
	device.option_add("sv807", SV807);
}
