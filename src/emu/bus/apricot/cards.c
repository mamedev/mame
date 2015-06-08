// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Expansion Slot Devices

***************************************************************************/

#include "cards.h"

SLOT_INTERFACE_START( apricot_expansion_cards )
	SLOT_INTERFACE("256k", APRICOT_256K_RAM)
	SLOT_INTERFACE("128_512k", APRICOT_128_512K_RAM)
SLOT_INTERFACE_END
