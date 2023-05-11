// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawai MB63H158 Touch Sensor

    This is a 64-pin Fujitsu CMOS gate array programmed as a keyboard
    scanner. The matrix it scans has 5 or 6 address lines and 4 active
    high return lines. Typically, additional HCMOS logic ICs are used to
    decode the former and multiplex the latter. It also manages a pair of
    150 ns static RAMs as a 512×15-bit private buffer.

    SDIP and QFP versions appear to have different numberings for the same
    functional set of pins, including two mode pins that are variously
    strapped. The CPU interface includes a byte-wide data bus, 8 address
    lines and two CS inputs that are normally tied together. The device
    has no CPU-writable registers and does not generate interrupts.

***************************************************************************/

#include "emu.h"
#include "mb63h158.h"

// device type definition
DEFINE_DEVICE_TYPE(MB63H158, mb63h158_device, "mb63h158", "Kawai MB63H158 Touch Sensor")

mb63h158_device::mb63h158_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MB63H158, tag, owner, clock)
{
}

void mb63h158_device::device_start()
{
}

void mb63h158_device::device_reset()
{
}

u8 mb63h158_device::read(offs_t offset)
{
	// TODO
	return 0;
}
