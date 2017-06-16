// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Hexbus floppy disk drive
    HX5102

    See hx5102.cpp for documentation

    Michael Zapf
    June 2017

*****************************************************************************/

#ifndef MAME_BUS_TI99_HEXBUS_HX5102_H
#define MAME_BUS_TI99_HEXBUS_HX5102_H

#pragma once

#include "hexbus.h"

namespace bus { namespace ti99 { namespace hexbus {

class hx5102_device : public device_t, public device_ti_hexbus_interface
{
public:
	hx5102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void device_start() override;
};

}   }   }  // end namespace bus::ti99::hexbus

DECLARE_DEVICE_TYPE_NS(TI_HX5102, bus::ti99::hexbus, hx5102_device)

#endif // MAME_BUS_TI99_HEXBUS_HX5102_H
