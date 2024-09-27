// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MIT_CADR_DISPLAY_H
#define MAME_MIT_CADR_DISPLAY_H

#pragma once


#include "emupal.h"


DECLARE_DEVICE_TYPE(CADR_DISPLAY, cadr_display_device)


class cadr_display_device : public device_t
{
public:
	cadr_display_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	auto irq_callback() { return m_irq_cb.bind(); }

	u32 status_r();
	void status_w(u32 data);
	u32 video_ram_read(offs_t offset);
	void video_ram_write(offs_t offset, u32 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	static constexpr u16 SCREEN_WIDTH = 768;
	static constexpr u16 SCREEN_HEIGHT = 939; // Docs mention a 768x900 display, but this displays the entire screen
	static constexpr u16 VIDEO_RAM_SIZE = 32 * 1024;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(tv_60hz_callback);

	required_device<palette_device> m_palette;
	devcb_write_line m_irq_cb;
	emu_timer *m_60hz_timer;
	u32 m_status;
	std::unique_ptr<u32[]> m_video_ram;
	bitmap_ind16 m_bitmap;
};

#endif // MAME_MIT_CADR_DISPLAY_H
