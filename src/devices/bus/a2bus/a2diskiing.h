// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2diskiing.h

    Apple II Disk II Controller Card, new floppy

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2DISKIING_H
#define MAME_BUS_A2BUS_A2DISKIING_H

#pragma once

#include "a2bus.h"


DECLARE_DEVICE_TYPE(A2BUS_DISKIING, device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_DISKIING13, device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_APPLESURANCE, device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_AGAT7_FDC, device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_AGAT9_FDC, device_a2bus_card_interface)

#endif  // MAME_BUS_A2BUS_A2DISKIING_H
