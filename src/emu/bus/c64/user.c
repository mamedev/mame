// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore 64 User Port emulation

**********************************************************************/

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

SLOT_INTERFACE_START( c64_user_port_cards )
	SLOT_INTERFACE("4cga", C64_4CGA)
	SLOT_INTERFACE("4dxh", C64_4DXH)
	SLOT_INTERFACE("4ksa", C64_4KSA)
	SLOT_INTERFACE("4tba", C64_4TBA)
	SLOT_INTERFACE("bn1541", C64_BN1541)
	SLOT_INTERFACE("geocable", C64_GEOCABLE)
	SLOT_INTERFACE("rs232", VIC1011)
SLOT_INTERFACE_END
