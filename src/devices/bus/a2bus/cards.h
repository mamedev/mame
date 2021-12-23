// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

	Apple II bus slot cards

	All of these cards are electrically compatible, but for compatibility
	reasons we divide them by machine type.

***************************************************************************/

#ifndef MAME_BUS_A2BUS_CARDS_H
#define MAME_BUS_A2BUS_CARDS_H

#include "bus/a2bus/a2bus.h"

void apple2_slot0_cards(device_slot_interface &device) ATTR_COLD;
void apple2_cards(device_slot_interface &device) ATTR_COLD;
void apple2e_cards(device_slot_interface &device) ATTR_COLD;
void apple2gs_cards(device_slot_interface &device) ATTR_COLD;
void apple3_cards(device_slot_interface &device) ATTR_COLD;

#endif // MAME_BUS_A2BUS_CARDS_H
