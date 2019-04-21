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

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank);

	DECLARE_READ16_MEMBER(video_r);
	DECLARE_WRITE16_MEMBER(video_w);

	auto sprlimit_read_callback() { return m_sprlimit_read_cb.bind(); };
	auto rowscrolloffset_read_callback() { return m_rowscrolloffset_read_cb.bind(); };

	auto write_video_irq_callback() { return m_video_irq_cb.bind(); };

protected:

	enum
	{
		PAGE_ENABLE_MASK        = 0x0008,
		PAGE_WALLPAPER_MASK     = 0x0004,

		SPRITE_ENABLE_MASK      = 0x0001,
		SPRITE_COORD_TL_MASK    = 0x0002,

		PAGE_PRIORITY_FLAG_MASK    = 0x3000,
		PAGE_PRIORITY_FLAG_SHIFT   = 12,
		PAGE_TILE_HEIGHT_MASK   = 0x00c0,
		PAGE_TILE_HEIGHT_SHIFT  = 6,
		PAGE_TILE_WIDTH_MASK    = 0x0030,
		PAGE_TILE_WIDTH_SHIFT   = 4,
		TILE_X_FLIP             = 0x0004,
		TILE_Y_FLIP             = 0x0008
	};


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

	void apply_saturation(const rectangle &cliprect);
	void apply_fade(const rectangle &cliprect);

	template<blend_enable_t Blend, rowscroll_enable_t RowScroll, flipx_t FlipX>
	void draw(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t bitmap_addr, uint16_t tile, int32_t h, int32_t w, uint8_t bpp, uint32_t yflipmask, uint32_t palette_offset);
	void draw_page(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t bitmap_addr, uint16_t *regs);
	void draw_sprite(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t base_addr);
	void draw_sprites(const rectangle &cliprect, uint32_t scanline, int priority);

	uint8_t mix_channel(uint8_t a, uint8_t b);

	uint32_t m_screenbuf[320 * 240];
	uint8_t m_rgb5_to_rgb8[32];
	uint32_t m_rgb555_to_rgb888[0x8000];

	bool m_hide_page0;
	bool m_hide_page1;
	bool m_hide_sprites;
	bool m_debug_sprites;
	bool m_debug_blit;
	bool m_debug_palette;
	uint8_t m_sprite_index_to_debug;

	uint16_t m_video_regs[0x100];

	devcb_read16 m_sprlimit_read_cb;
	devcb_read16 m_rowscrolloffset_read_cb;

	emu_timer *m_screenpos_timer;

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint16_t> m_scrollram;
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
