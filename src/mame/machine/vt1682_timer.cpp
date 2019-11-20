// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "machine/vt1682_timer.h"

#define LOG_TIMER     (1U << 1)

#define LOG_ALL           ( LOG_TIMER )

#define VERBOSE             (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(VT_VT1682_TIMER, vrt_vt1682_timer_device, "vt1682timer", "VRT VT1682 Timer")

vrt_vt1682_timer_device::vrt_vt1682_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VT_VT1682_TIMER, tag, owner, clock)
{
}

void vrt_vt1682_timer_device::device_start()
{
}

void vrt_vt1682_timer_device::device_reset()
{
}

