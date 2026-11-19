// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB) slot options

****************************************************************************/

#include "emu.h"
#include "cards.h"

#include "gp19.h"
#include "igc.h"
#include "imaginator.h"
#include "super19.h"
#include "superset.h"
#include "tlbbase.h"
#include "ultra.h"
#include "watz.h"


void h19_tlb_options(device_slot_interface &device)
{
	device.option_add("heath",        HEATH_TLB);
	device.option_add("gp19",         HEATH_GP19);
	device.option_add("imaginator",   HEATH_IMAGINATOR);
	device.option_add("super19",      HEATH_SUPER19);
	device.option_add("superset",     HEATH_SUPERSET);
	device.option_add("ultrarom",     HEATH_ULTRA);
	device.option_add("watzman",      HEATH_WATZ);
}

void h89_tlb_options(device_slot_interface &device)
{
	h19_tlb_options(device);

	device.option_add("igc",          HEATH_IGC);
	device.option_add("igc_super19",  HEATH_IGC_SUPER19);
	device.option_add("igc_ultrarom", HEATH_IGC_ULTRA);
	device.option_add("igc_watzman",  HEATH_IGC_WATZ);
}
