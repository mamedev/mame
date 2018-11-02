// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "machine/xavix_mtrk_wheel.h"

DEFINE_DEVICE_TYPE(XAVIX_MTRK_WHEEL, xavix_mtrk_wheel_device, "xavix_mtrk_wheel", "XaviX / Radica Monster Truck Steering Wheel")

xavix_mtrk_wheel_device::xavix_mtrk_wheel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XAVIX_MTRK_WHEEL, tag, owner, clock),
	m_event_out_cb(*this)
{
}

void xavix_mtrk_wheel_device::device_start()
{
	m_event_out_cb.resolve_safe();
}

void xavix_mtrk_wheel_device::device_reset()
{
}

int xavix_mtrk_wheel_device::read_direction()
{
	return 0;
}
