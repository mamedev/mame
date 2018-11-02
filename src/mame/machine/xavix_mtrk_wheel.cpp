// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "machine/xavix_mtrk_wheel.h"

DEFINE_DEVICE_TYPE(XAVIX_MTRK_WHEEL, xavix_mtrk_wheel_device, "xavix_mtrk_wheel", "XaviX / Radica Monster Truck Steering Wheel")

xavix_mtrk_wheel_device::xavix_mtrk_wheel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XAVIX_MTRK_WHEEL, tag, owner, clock),
	m_event_out_cb(*this),
	m_in(*this, "WHEEL")
{
}

static INPUT_PORTS_START( wheel )
	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)
INPUT_PORTS_END


ioport_constructor xavix_mtrk_wheel_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(wheel);
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
