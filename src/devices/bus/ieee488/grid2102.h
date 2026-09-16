// license:BSD-3-Clause
// copyright-holders:usernameak
/**********************************************************************

    GRiD 2102 Portable Floppy emulation

**********************************************************************/

#ifndef MAME_BUS_IEEE488_GRID2102_H
#define MAME_BUS_IEEE488_GRID2102_H

#pragma once

#include "ieee488.h"


DECLARE_DEVICE_TYPE(GPIB_GRID2102, device_ieee488_interface)
DECLARE_DEVICE_TYPE(GPIB_GRID2101_FLOPPY, device_ieee488_interface)
DECLARE_DEVICE_TYPE(GPIB_GRID2101_HDD, device_ieee488_interface)

#endif // MAME_BUS_IEEE488_GRID2102_H
