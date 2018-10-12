// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore 64 User Port emulation

**********************************************************************/

#include "emu.h"
#include "user.h"

//-------------------------------------------------
//  SLOT_INTERFACE( c64_user_port_cards )
//-------------------------------------------------

// slot devices
#include "bus/vic20/4cga.h"
#include "4dxh.h"
#include "4ksa.h"
#include "4tba.h"
#include "bn1541.h"
#include "geocable.h"
#include "bus/vic20/vic1011.h"

void c64_user_port_cards(device_slot_interface &device)
{
	device.option_add("4cga", C64_4CGA);
	device.option_add("4dxh", C64_4DXH);
	device.option_add("4ksa", C64_4KSA);
	device.option_add("4tba", C64_4TBA);
	device.option_add("bn1541", C64_BN1541);
	device.option_add("geocable", C64_GEOCABLE);
	device.option_add("rs232", VIC1011);
}
