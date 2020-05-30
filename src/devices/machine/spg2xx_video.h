// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation (Video)

**********************************************************************/

#ifndef MAME_MACHINE_SPG2XX_VIDEO_H
#define MAME_MACHINE_SPG2XX_VIDEO_H

#pragma once

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

	DECLARE_READ16_MEMBER(video_r);
	DECLARE_WRITE16_MEMBER(video_w);

	auto sprlimit_read_callback() { return m_sprlimit_read_cb.bind(); };

	auto write_video_irq_callback() { return m_video_irq_cb.bind(); };

protected:

	devcb_read16 m_guny_in;
	devcb_read16 m_gunx_in;

	inline void check_video_irq();

	static const device_timer_id TIMER_SCREENPOS = 2;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void do_sprite_dma(uint32_t len);

	enum blend_enable_t : bool
	{
		BlendOff = false,
		BlendOn = true
	};

	enum rowscroll_enable_t : bool
	{
		RowScrollOff = false,
		RowScrollOn = true
	};

	enum flipx_t : bool
	{
		FlipXOff = false,
		FlipXOn = true
	};

	void apply_saturation_and_fade(bitmap_rgb32& bitmap, const rectangle& cliprect, int scanline);

	void draw_page(const rectangle &cliprect, uint32_t* dst, uint32_t scanline, int priority, uint32_t tilegfxdata_addr, uint16_t *regs);
	void draw_sprites(const rectangle &cliprect, uint32_t* dst, uint32_t scanline, int priority);

	inline void draw_sprite(const rectangle &cliprect, uint32_t* dst, uint32_t scanline, int priority, uint32_t base_addr);
	inline void draw_linemap(const rectangle& cliprect, uint32_t* dst, uint32_t scanline, int priority, uint32_t tilegfxdata_addr, uint16_t* regs);
	inline bool get_tile_info(uint32_t tilemap_rambase, uint32_t palettemap_rambase, uint32_t x0, uint32_t y0, uint32_t tile_count_x, uint32_t ctrl, uint32_t attr, uint16_t& tile, bool& blend, bool& flip_x, bool& flip_y, uint32_t& palette_offset);

	template<spg2xx_video_device::blend_enable_t Blend, spg2xx_video_device::flipx_t FlipX>
	inline void draw_tilestrip(const rectangle& cliprect, uint32_t* dst, uint32_t tile_h, uint32_t tile_w, uint32_t tilegfxdata_addr, uint16_t tile, uint32_t tile_scanline, int drawx, bool flip_y, uint32_t palette_offset, const uint32_t nc_bpp, const uint32_t bits_per_row, const uint32_t words_per_tile);

	uint8_t mix_channel(uint8_t a, uint8_t b);

	uint8_t m_rgb5_to_rgb8[32];
	uint32_t m_rgb555_to_rgb888[0x8000];

	int m_ycmp_table[480];
	void update_vcmp_table();

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
