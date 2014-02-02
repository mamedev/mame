// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore Plus/4 User Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "user.h"

//-------------------------------------------------
//  SLOT_INTERFACE( plus4_user_port_cards )
//-------------------------------------------------

// slot devices
#include "diag264_lb_user.h"
#include "bus/vic20/vic1011.h"

SLOT_INTERFACE_START( plus4_user_port_cards )
	SLOT_INTERFACE("diag264", DIAG264_USER_PORT_LOOPBACK)
	SLOT_INTERFACE("rs232", VIC1011)
SLOT_INTERFACE_END
