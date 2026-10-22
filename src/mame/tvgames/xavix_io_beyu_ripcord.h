// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_BEYU_RIPCORD_H
#define MAME_TVGAMES_XAVIX_IO_BEYU_RIPCORD_H

#pragma once

#include "screen.h"

#include <utility>

class xavix_beyblade_ripcord_device : public device_t
{
public:
	xavix_beyblade_ripcord_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename... T> void set_event_callback(T &&... args) { m_event_out_cb.set(std::forward<T>(args)...); }
	void vblank_w(int state);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(event_timer);

	device_delegate<void ()> m_event_out_cb;
	required_device<screen_device> m_screen;
	required_ioport m_position;
	emu_timer *m_event_timer;
	u8 m_last_position;
	u8 m_events_remaining;
	bool m_position_valid;
};

DECLARE_DEVICE_TYPE(XAVIX_BEYBLADE_RIPCORD, xavix_beyblade_ripcord_device)

#endif // MAME_TVGAMES_XAVIX_IO_BEYU_RIPCORD_H
