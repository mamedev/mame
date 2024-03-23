// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    romcard.h

    Implemention of the Apple II ROM card/firmware card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_ROMCARD_H
#define MAME_BUS_A2BUS_ROMCARD_H

#pragma once

#include "a2bus.h"

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_ROMCARDUSER, device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_ROMCARDFP, device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_ROMCARDINT, device_a2bus_card_interface)

#endif // MAME_BUS_A2BUS_ROMCARD_H
