// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_BB_BAT_H
#define MAME_TVGAMES_XAVIX_IO_BB_BAT_H

#pragma once

#include "screen.h"

#include <utility>

class xavix_baseball_bat_device : public device_t
{
public:
	xavix_baseball_bat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename... T> void set_event_callback(T &&... args) { m_event_out_cb.set(std::forward<T>(args)...); }
	void vblank_w(int state);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u8 EVENT_COUNTS[4] = { 0, 2, 4, 8 };

	TIMER_CALLBACK_MEMBER(event_timer);

	device_delegate<void ()> m_event_out_cb;
	required_device<screen_device> m_screen;
	required_ioport m_strength;
	emu_timer *m_event_timer;
	u8 m_events_remaining;
	bool m_swing_armed;
};

DECLARE_DEVICE_TYPE(XAVIX_BASEBALL_BAT, xavix_baseball_bat_device)

#endif // MAME_TVGAMES_XAVIX_IO_BB_BAT_H
