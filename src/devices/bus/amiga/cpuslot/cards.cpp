// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    86-pin expansion slot (A500, A1000)
    Coprocessor slot (A2000, B2000)

    Card options

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "a570.h"
#include "a590.h"
#include "action_replay.h"
#include "megamix500.h"


void a1000_cpuslot_cards(device_slot_interface &device)
{
}

void a500_cpuslot_cards(device_slot_interface &device)
{
	device.option_add("a570", AMIGA_CPUSLOT_A570);
	device.option_add("a590", AMIGA_CPUSLOT_A590);
	device.option_add("ar", AMIGA_CPUSLOT_ACTION_REPLAY_MK1);
	device.option_add("ar2", AMIGA_CPUSLOT_ACTION_REPLAY_MK2);
	device.option_add("ar3", AMIGA_CPUSLOT_ACTION_REPLAY_MK3);
	device.option_add("megamix500", AMIGA_CPUSLOT_MEGAMIX500);
}

void a2000_cpuslot_cards(device_slot_interface &device)
{
}
