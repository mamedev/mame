// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SVI 318/328 Expander Bus Modules

***************************************************************************/

#include "emu.h"
#include "modules.h"

#include "sv601.h"
#include "sv602.h"
#include "sv603.h"

void svi_expander_modules(device_slot_interface &device)
{
	device.option_add("sv601", SV601);
	device.option_add("sv602", SV602);
	device.option_add("sv603", SV603);
}
