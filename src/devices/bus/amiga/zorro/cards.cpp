// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Amiga Zorro Cards

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "a2052.h"
#include "a2058.h"
#include "a2065.h"
#include "a2232.h"
#include "a590.h"
#include "action_replay.h"
#include "buddha.h"
#include "picasso2.h"
#include "ripple.h"
#include "toccata.h"


void a1000_expansion_cards(device_slot_interface &device)
{
}

void a500_expansion_cards(device_slot_interface &device)
{
	device.option_add("ar1", ZORRO_ACTION_REPLAY_MK1);
	device.option_add("ar2", ZORRO_ACTION_REPLAY_MK2);
	device.option_add("ar3", ZORRO_ACTION_REPLAY_MK3);
	device.option_add("a590", ZORRO_A590);
}

void a2000_expansion_cards(device_slot_interface &device)
{
	device.option_add("ar1", ZORRO_ACTION_REPLAY_MK1);
	device.option_add("ar2", ZORRO_ACTION_REPLAY_MK2);
	device.option_add("ar3", ZORRO_ACTION_REPLAY_MK3);
}

void zorro2_cards(device_slot_interface &device)
{
	device.option_add("a2052", ZORRO_A2052);
	device.option_add("a2058", ZORRO_A2058);
	device.option_add("a2065", ZORRO_A2065);
	device.option_add("a2091", ZORRO_A2091);
	device.option_add("a2232", ZORRO_A2232);
	device.option_add("buddha", ZORRO_BUDDHA);
	device.option_add("picasso2p", ZORRO_PICASSO2P);
	device.option_add("ripple", ZORRO_RIPPLE);
	device.option_add("toccata", ZORRO_TOCCATA);
}

void zorro3_cards(device_slot_interface &device)
{
	device.option_add("a2052", ZORRO_A2052);
	device.option_add("a2058", ZORRO_A2058);
	device.option_add("a2065", ZORRO_A2065);
	device.option_add("a2091", ZORRO_A2091);
	device.option_add("a2232", ZORRO_A2232);
	device.option_add("buddha", ZORRO_BUDDHA);
	device.option_add("picasso2p", ZORRO_PICASSO2P);
	device.option_add("ripple", ZORRO_RIPPLE);
	device.option_add("toccata", ZORRO_TOCCATA);
}
