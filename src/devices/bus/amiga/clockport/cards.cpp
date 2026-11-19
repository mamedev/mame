// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Amiga 1200 clock port card options

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "eth64ii.h"
#include "mp3at64.h"
#include "rrnet.h"
#include "silversurfer.h"

void amiga_clockport_cards(device_slot_interface &device)
{
	device.option_add("eth64ii", AMIGA_CLOCKPORT_ETH64II);
	device.option_add("mp3at64", AMIGA_CLOCKPORT_MP3AT64);
	device.option_add("rrnet", AMIGA_CLOCKPORT_RRNET);
	device.option_add("silversurfer", AMIGA_CLOCKPORT_SILVERSURFER);
}
