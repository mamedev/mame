// license:BSD-3-Clause
// copyright-holders:Sven Schnelle

#include "emu.h"
#include "topcat.h"

// define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TOPCAT, topcat_device, "topcat", "HP Topcat ASIC")

topcat_device::topcat_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
{
}

topcat_device::topcat_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: topcat_device(mconfig, TOPCAT, tag, owner, clock)
{
}

void topcat_device::device_start()
{
}

void topcat_device::device_reset()
{
}

READ8_MEMBER(topcat_device::address_r)
{
	return 0xff;
}

WRITE8_MEMBER(topcat_device::address_w)
{
}

READ8_MEMBER(topcat_device::register_r)
{
	return space.unmap();
}

WRITE8_MEMBER(topcat_device::register_w)
{
}
