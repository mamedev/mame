// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 8050/8250/SFD-1001 Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_IEEE488_C8050_H
#define MAME_BUS_IEEE488_C8050_H

#pragma once

#include "ieee488.h"



DECLARE_DEVICE_TYPE(GPIB_C8050,   device_ieee488_interface)
DECLARE_DEVICE_TYPE(GPIB_C8250,   device_ieee488_interface)
DECLARE_DEVICE_TYPE(GPIB_C8250LP, device_ieee488_interface)
DECLARE_DEVICE_TYPE(GPIB_SFD1001, device_ieee488_interface)


#endif // MAME_BUS_IEEE488_C8050_H
