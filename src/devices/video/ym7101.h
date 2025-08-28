// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_VIDEO_YM7101_H
#define MAME_VIDEO_YM7101_H

#pragma once

#include "machine/timer.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"

class ym7101_device : public device_t,
					  public device_memory_interface,
					  public device_video_interface,
					  public device_mixer_interface
{
public:
	ym7101_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void if_map(address_map &map) ATTR_COLD;

	auto vint_cb() { return m_vint_callback.bind(); }
	//auto hint_cb() { return m_lv4irqline_callback.bind(); }
	auto sint_cb() { return m_sint_callback.bind(); }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void irq_ack()
	{
		if (machine().side_effects_disabled())
			return;

		//if (m_vint_pending)
		//{
		//  m_vint_pending = 0;
		m_vint_callback(false);
		//}
		//else if (m_hint_pending)
		//{
		//  m_hint_pending = 0;
		//  m_hint_callback(false);
		//}
	}

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	void vram_map(address_map &map) ATTR_COLD;
	void regs_map(address_map &map) ATTR_COLD;

	const address_space_config m_space_vram_config;
	const address_space_config m_space_regs_config;

	devcb_write_line m_vint_callback;
//  devcb_write_line m_hint_callback;
	devcb_write_line m_sint_callback;

	required_device<segapsg_device> m_psg;

	TIMER_CALLBACK_MEMBER(scan_timer_callback);
	emu_timer *m_scan_timer;

	bool m_ie0;
};

DECLARE_DEVICE_TYPE(YM7101, ym7101_device)


#endif // MAME_VIDEO_YM7101_H
