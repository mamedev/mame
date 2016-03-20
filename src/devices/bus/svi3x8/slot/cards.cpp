// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SVI 318/328 Slot Cards

***************************************************************************/

#include "cards.h"

SLOT_INTERFACE_START( svi_slot_cards )
	SLOT_INTERFACE("sv801", SV801)
	SLOT_INTERFACE("sv802", SV802)
	SLOT_INTERFACE("sv803", SV803)
	SLOT_INTERFACE("sv805", SV805)
	SLOT_INTERFACE("sv806", SV806)
	SLOT_INTERFACE("sv807", SV807)
SLOT_INTERFACE_END

// The single slot expander doesn't support the disk controller, since
// it requires its own power supply to power the disk drives
SLOT_INTERFACE_START( sv602_slot_cards )
	SLOT_INTERFACE("sv802", SV802)
	SLOT_INTERFACE("sv803", SV803)
	SLOT_INTERFACE("sv805", SV805)
	SLOT_INTERFACE("sv806", SV806)
	SLOT_INTERFACE("sv807", SV807)
SLOT_INTERFACE_END
