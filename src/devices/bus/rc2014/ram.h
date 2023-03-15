// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    RC2014 RAM Module

**********************************************************************/

#ifndef MAME_BUS_RC2014_RAM_H
#define MAME_BUS_RC2014_RAM_H

#pragma once

#include "bus/rc2014/rc2014.h"

DECLARE_DEVICE_TYPE(RC2014_RAM_32K, device_rc2014_card_interface)
DECLARE_DEVICE_TYPE(RC2014_RAM_64K, device_rc2014_ext_card_interface)
DECLARE_DEVICE_TYPE(RC2014_RAM_64K_40P, device_rc2014_card_interface)

#endif // MAME_BUS_RC2014_RAM_H
