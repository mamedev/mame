// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2videoterm.h

    Implementation of the Videx Videoterm 80-column card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2VIDEOTERM_H
#define MAME_BUS_A2BUS_A2VIDEOTERM_H

#pragma once

#include "a2bus.h"

// device type declaration
DECLARE_DEVICE_TYPE(A2BUS_VIDEOTERM,      device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_IBSAP16,        device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_IBSAP16ALT,     device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_VTC1,           device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_AEVIEWMASTER80, device_a2bus_card_interface)

#endif // MAME_BUS_A2BUS_A2VIDEOTERM_H
