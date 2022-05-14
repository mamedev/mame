// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    RC2014 ROM Module

**********************************************************************/

#ifndef MAME_BUS_RC2014_ROM_H
#define MAME_BUS_RC2014_ROM_H

#pragma once

#include "bus/rc2014/rc2014.h"

DECLARE_DEVICE_TYPE(RC2014_SWITCHABLE_ROM, device_rc2014_card_interface)
DECLARE_DEVICE_TYPE(RC2014_PAGABLE_ROM, device_rc2014_ext_card_interface)

#endif // MAME_BUS_RC2014_ROM_H
