// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    External RC power-hold circuit used by selected XaviX boards. Firmware
    pulses IO1.7 to retain power; stopping the pulses allows the hold to decay.
    Patents US7093164 and WO2004093053A1 identify this connection.

    TODO: Measure the physical decay time; two seconds is provisional.

***************************************************************************/

#include "emu.h"
#include "xavix_power.h"

DEFINE_DEVICE_TYPE(XAVIX_POWER_HOLD, xavix_power_hold_device, "xavix_power_hold", "XaviX External Power Hold")

xavix_power_hold_device::xavix_power_hold_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_POWER_HOLD, tag, owner, clock)
	, m_power_off_cb(*this)
	, m_timer(nullptr)
{
}

void xavix_power_hold_device::device_start()
{
	m_timer = timer_alloc(FUNC(xavix_power_hold_device::power_hold_expired), this);
	save_item(NAME(m_last_level));
}

void xavix_power_hold_device::device_reset()
{
	m_timer->adjust(attotime::never);
	m_last_level = 0xff;
}

void xavix_power_hold_device::write(offs_t offset, u8 data, u8 mem_mask)
{
	if (BIT(mem_mask, 7))
	{
		u8 const level = BIT(data, 7);
		if (m_last_level == 0xff)
			m_last_level = level;
		else if (level != m_last_level)
		{
			m_last_level = level;
			m_timer->adjust(attotime::from_seconds(2));
		}
	}
}

TIMER_CALLBACK_MEMBER(xavix_power_hold_device::power_hold_expired)
{
	m_power_off_cb(1);
}
