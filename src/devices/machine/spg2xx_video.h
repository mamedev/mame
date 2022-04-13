// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation (Video)

**********************************************************************/

#ifndef MAME_MACHINE_SPG2XX_VIDEO_H
#define MAME_MACHINE_SPG2XX_VIDEO_H

#pragma once

#include "spg_renderer.h"
#include "cpu/unsp/unsp.h"
#include "screen.h"

class spg2xx_video_device : public device_t
{
public:
	spg2xx_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	auto guny_in() { return m_guny_in.bind(); }
	auto gunx_in() { return m_gunx_in.bind(); }


	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank);

	uint16_t video_r(offs_t offset);
	void video_w(offs_t offset, uint16_t data);

	auto sprlimit_read_callback() { return m_sprlimit_read_cb.bind(); }

	auto write_video_irq_callback() { return m_video_irq_cb.bind(); }

protected:
	virtual void device_add_mconfig(machine_config &config) override;

	devcb_read16 m_guny_in;
	devcb_read16 m_gunx_in;

	inline void check_video_irq();

	static const device_timer_id TIMER_SCREENPOS = 2;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	void do_sprite_dma(uint32_t len);

	uint16_t m_video_regs[0x100];

	devcb_read16 m_sprlimit_read_cb;

	emu_timer *m_screenpos_timer;

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint16_t> m_scrollram;
	required_shared_ptr<uint16_t> m_hcompram;
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_spriteram;

	devcb_write_line m_video_irq_cb;

	required_device<spg_renderer_device> m_renderer;
};

class spg24x_video_device : public spg2xx_video_device
{
public:
	template <typename T, typename U>
	spg24x_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&screen_tag)
		: spg24x_video_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	spg24x_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(SPG24X_VIDEO, spg24x_video_device)

#endif // MAME_MACHINE_SPG2XX_VIDEO_H
