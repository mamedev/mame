// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Amiga Zorro-II Cards

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "a2052.h"
#include "a2058.h"
#include "a2065.h"
#include "a2091.h"
#include "a2232.h"
#include "buddha.h"
#include "merlin.h"
#include "oktagon2008.h"
#include "picasso2.h"
#include "rainbow2.h"
#include "ripple.h"
#include "toccata.h"


void zorro2_cards(device_slot_interface &device)
{
	device.option_add("a2052", AMIGA_A2052);
	device.option_add("a2058", AMIGA_A2058);
	device.option_add("a2065", AMIGA_A2065);
	device.option_add("a2091", AMIGA_A2091);
	device.option_add("a2232", AMIGA_A2232);
	device.option_add("buddha", AMIGA_BUDDHA);
	device.option_add("merlin", AMIGA_MERLIN);
	device.option_add("oktagon2008", AMIGA_OKTAGON2008);
	device.option_add("picasso2p", AMIGA_PICASSO2P);
	device.option_add("rainbow2", AMIGA_RAINBOW2);
	device.option_add("framemaster", AMIGA_FRAMEMASTER);
	device.option_add("ripple", AMIGA_RIPPLE);
	device.option_add("toccata", AMIGA_TOCCATA);
}
