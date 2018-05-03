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


void sr_cards(device_slot_interface &device)
{
	device.option_add("mpcb963", MPCB963);
	device.option_add("mpcba79", MPCBA79);
	device.option_add("mpcb070", MPCB070);
	device.option_add("mpcb071", MPCB071);
	device.option_add("mpcb081", MPCB081);
}
