// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

   Cards for the H8 Benton Harbor Bus

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "cpu8080.h"
#include "front_panel.h"
#include "h8bus.h"
#include "h_8_1.h"
#include "h_8_5.h"
#include "wh_8_64.h"

// P1 is reserved for the Front Panel
void h8_p1_cards(device_slot_interface &device)
{
	device.option_add("fp", H8BUS_FRONT_PANEL);
}

// P2 is reserved for the CPU board.
void h8_p2_cards(device_slot_interface &device)
{
	device.option_add("cpu8080", H8BUS_CPU_8080);
}

// P10 is reserved for the HA-8-8 Extended Configuration Option card, which is
// required to run CP/M with the 8080 CPU board.
// TODO - add the HA-8-8
void h8_p10_cards(device_slot_interface &device)
{
}

void h8_cards(device_slot_interface &device)
{
	device.option_add("h_8_1",   H8BUS_H_8_1);
	device.option_add("h_8_5",   H8BUS_H_8_5);
	device.option_add("wh_8_64", H8BUS_WH_8_64);
}
