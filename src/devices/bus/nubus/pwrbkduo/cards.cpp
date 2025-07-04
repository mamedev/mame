// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Macintosh PowerBook Duo dock slot

    Official docks made by Apple:
    - Floppy Dock - contains a SWIM2 chip and an ADB port.  No ROM.
    - Duo Dock I - contains video, SCC, SCSI, an ADB port, and a declaration ROM.
    - Duo Dock II - contains video, SCC, SCSI, Sonic Ethernet, an ADB port, and a declaration ROM.
    - Mini Dock - same functionality as Duo Dock I, but smaller and doesn't inject the Duo.

    Third-party docks supposedly exist but I haven't found any specific references.

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "duodock.h"
#include "floppydock.h"
#include "ethernetudock.h"

void pwrbkduo_cards(device_slot_interface &device)
{
	device.option_add("duodock", DUODOCK_DUODOCK);          // Apple DuoDock
	device.option_add("etherudock", DUODOCK_ETHERUDOCK);    // NewerTech Ethernet MicroDock
	device.option_add("floppydock", DUODOCK_FLOPPYDOCK);    // Apple Floppy Dock
}
