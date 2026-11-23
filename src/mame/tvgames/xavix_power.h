// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_POWER_H
#define MAME_TVGAMES_XAVIX_POWER_H

#pragma once

class xavix_power_hold_device : public device_t
{
public:
	xavix_power_hold_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename... T> void set_power_off_callback(T &&... args) { m_power_off_cb.set(std::forward<T>(args)...); }
	void write(offs_t offset, u8 data, u8 mem_mask);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(power_hold_expired);

	device_delegate<void ()> m_power_off_cb;
	emu_timer *m_timer;
	u8 m_last_level = 0xff;
};

DECLARE_DEVICE_TYPE(XAVIX_POWER_HOLD, xavix_power_hold_device)

#endif // MAME_TVGAMES_XAVIX_POWER_H
