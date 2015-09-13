// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom NASBUS Slot Devices

***************************************************************************/

#include "cards.h"

SLOT_INTERFACE_START( nasbus_slot_cards )
	SLOT_INTERFACE("avc", NASCOM_AVC)
	SLOT_INTERFACE("floppy", NASCOM_FDC)
SLOT_INTERFACE_END
