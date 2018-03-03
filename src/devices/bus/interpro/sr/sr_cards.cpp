// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"
#include "sr_cards.h"

// video
#include "gt.h"

// storage

// sound

// network

// communication ports

// other


SLOT_INTERFACE_START(sr_cards)
	SLOT_INTERFACE("mpcb963", MPCB963)
	SLOT_INTERFACE("mpcba79", MPCBA79)
	SLOT_INTERFACE("mpcb070", MPCB070)
	SLOT_INTERFACE("mpcb071", MPCB071)
	SLOT_INTERFACE("mpcb081", MPCB081)
SLOT_INTERFACE_END
