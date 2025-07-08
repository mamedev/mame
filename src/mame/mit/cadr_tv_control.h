// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MIT_CADR_TV_CONTROL_H
#define MAME_MIT_CADR_TV_CONTROL_H

#pragma once


#include "screen.h"


DECLARE_DEVICE_TYPE(CADR_TV_CONTROL, cadr_tv_control_device)


class cadr_tv_control_device : public device_t
{
public:
	cadr_tv_control_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	auto irq_callback() { return m_irq_cb.bind(); }

	u32 video_ram_read(offs_t offset);
	void video_ram_write(offs_t offset, u32 data);
	u32 tv_control_r(offs_t offset);
	void tv_control_w(offs_t offset, u32 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(tv_60hz_callback);

	required_device<screen_device> m_screen;
	devcb_write_line m_irq_cb;
	emu_timer *m_60hz_timer;
	u32 m_status;
	u32 m_sync_csr;
	u32 m_sync_address;
	std::unique_ptr<u32[]> m_video_ram;
	std::unique_ptr<u8[]> m_sync_ram;
};

#endif // MAME_MIT_CADR_TV_CONTROL_H
