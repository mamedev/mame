// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore VIC-20 User Port emulation

**********************************************************************/

#include "emu.h"
#include "user.h"

//-------------------------------------------------
//  SLOT_INTERFACE( vic20_user_port_cards )
//-------------------------------------------------

// slot devices
#include "4cga.h"
#include "vic1011.h"

void vic20_user_port_cards(device_slot_interface &device)
{
	device.option_add("4cga", C64_4CGA);
	device.option_add("rs232", VIC1011);
}
