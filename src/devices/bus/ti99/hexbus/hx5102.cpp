// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Hexbus floppy disk drive
    HX5102

    Work in progress

    Michael Zapf
    June 2017

*****************************************************************************/

#include "emu.h"
#include "hx5102.h"

#define TRACE_HEXBUS 0

DEFINE_DEVICE_TYPE_NS(TI_HX5102, bus::ti99::hexbus, hx5102_device, "ti_hx5102", "TI Hexbus Floppy")

namespace bus { namespace ti99 { namespace hexbus {

hx5102_device::hx5102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	hexbus_chained_device(mconfig, TI_HX5102, tag, owner, clock)
{
}

void hx5102_device::hexbus_value_changed(uint8_t data)
{
	if (TRACE_HEXBUS) logerror("Hexbus value changed to %02x\n", data);
}

}   }   }  // end namespace bus::ti99::hexbus

