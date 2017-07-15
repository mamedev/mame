// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/****************************************************************************

    Hexbus floppy disk drive
    HX5102

    See hx5102.cpp for documentation

    Michael Zapf
    June 2017

*****************************************************************************/

#ifndef MAME_BUS_HEXBUS_HX5102_H
#define MAME_BUS_HEXBUS_HX5102_H

#pragma once

#include "hexbus.h"

namespace bus { namespace hexbus {

class hx5102_device : public hexbus_chained_device
{
public:
	hx5102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void hexbus_value_changed(uint8_t data) override;
};

}   } // end namespace bus::hexbus

DECLARE_DEVICE_TYPE_NS(HX5102, bus::hexbus, hx5102_device)

#endif // MAME_BUS_HEXBUS_HX5102_H
