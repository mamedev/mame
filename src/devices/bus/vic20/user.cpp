// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore VIC-20 User Port emulation

**********************************************************************/

#include "user.h"

//-------------------------------------------------
//  SLOT_INTERFACE( vic20_user_port_cards )
//-------------------------------------------------

// slot devices
#include "4cga.h"
#include "vic1011.h"

SLOT_INTERFACE_START( vic20_user_port_cards )
	SLOT_INTERFACE("4cga", C64_4CGA)
	SLOT_INTERFACE("rs232", VIC1011)
SLOT_INTERFACE_END
