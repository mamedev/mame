// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Expansion Slot Devices

***************************************************************************/

#include "cards.h"

SLOT_INTERFACE_START( apricot_expansion_cards )
	SLOT_INTERFACE("128k", APRICOT_128K_RAM)
	SLOT_INTERFACE("256k", APRICOT_256K_RAM)
	SLOT_INTERFACE("512k", APRICOT_512K_RAM)
SLOT_INTERFACE_END
