// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore Plus/4 User Port emulation

**********************************************************************/

#include "emu.h"
#include "user.h"

//-------------------------------------------------
//  SLOT_INTERFACE( plus4_user_port_cards )
//-------------------------------------------------

// slot devices
#include "diag264_lb_user.h"
#include "bus/vic20/vic1011.h"

void plus4_user_port_cards(device_slot_interface &device)
{
	device.option_add("diag264", DIAG264_USER_PORT_LOOPBACK);
	device.option_add("rs232", VIC1011);
}
