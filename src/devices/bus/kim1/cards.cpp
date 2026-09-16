// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    KIM-1 bus slot cards

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "k1016_16k.h"
#include "k1008_vismem.h"


void kim1_cards(device_slot_interface &device)
{
	device.option_add("mtuk1008", KIM1BUS_K1008); // MTU K-1008 8K "Visible Memory" mono 320x200 framebuffer
	device.option_add("mtuk1016", KIM1BUS_K1016); // MTU K-1016 16K RAM card
}
