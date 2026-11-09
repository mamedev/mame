// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    IDE64 ShortBus card options

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "digimax.h"
#include "duart.h"
#include "eth64.h"
#include "etfe.h"

void ide64_shortbus_cards(device_slot_interface &device)
{
	device.option_add("digimax", IDE64_SHORTBUS_DIGIMAX);
	device.option_add("duart", IDE64_SHORTBUS_DUART);
	device.option_add("eth64", IDE64_SHORTBUS_ETH64);
	device.option_add("etfe", IDE64_SHORTBUS_ETFE);
}
