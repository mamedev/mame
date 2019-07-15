// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"
#include "sr_cards.h"

// video
#include "gt.h"
#include "edge.h"

// storage

// sound

// network

// communication ports

// other


void cbus_cards(device_slot_interface &device)
{
	device.option_add("mpcb963", MPCB963);
	device.option_add("mpcba79", MPCBA79);
	device.option_add("msmt070", MSMT070);
	device.option_add("msmt071", MSMT071);
	device.option_add("msmt081", MSMT081);
}

void srx_cards(device_slot_interface &device)
{
	device.option_add("mpcb030", MPCB030);
	device.option_add("mpcb828", MPCB828);
	device.option_add("mpcb849", MPCB849);
	device.option_add("mpcb896", MPCB896);
	device.option_add("mpcba63", MPCBA63);
	device.option_add("mpcbb68", MPCBB68);
	device.option_add("mpcbb92", MPCBB92);
	device.option_add("msmt094", MSMT094);
}
