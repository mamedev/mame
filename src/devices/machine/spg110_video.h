// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_SPG110_VIDEO_H
#define MAME_MACHINE_SPG110_VIDEO_H

#pragma once

#include "cpu/unsp/unsp.h"
#include "emupal.h"
#include "screen.h"
//#include "machine/timer.h"


class spg110_video_device : public device_t, public device_memory_interface

{
public:
	spg110_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	spg110_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T, typename U>
	spg110_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&screen_tag)
		: spg110_video_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	void set_video_irq_spidman(bool is_spiderman) { m_is_spiderman = is_spiderman; }

	void map_video(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank(int state);

	void spg110_201c_w(uint16_t data);
	void spg110_2020_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void spg110_2028_w(uint16_t data);
	void spg110_2029_w(uint16_t data);

	uint16_t spg110_2028_r();
	uint16_t spg110_2029_r();

	void spg110_2031_w(uint16_t data);
	void spg110_2032_w(uint16_t data);
	void spg110_2033_w(uint16_t data);
	void spg110_2034_w(uint16_t data);
	void spg110_2035_w(uint16_t data);
	void spg110_2036_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void spg110_2037_w(uint16_t data);
	void spg110_2039_w(uint16_t data);

	void spg110_203c_w(uint16_t data);
	void spg110_203d_w(uint16_t data);

	void spg110_2042_w(uint16_t data);

	void spg110_2045_w(uint16_t data);

	void spg110_205x_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);


	uint16_t spg110_2037_r();
	uint16_t spg110_2042_r();

	void dma_dst_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dma_unk_2061_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dma_len_trigger_w(uint16_t data);
	void spg110_2063_w(uint16_t data);
	void dma_dst_step_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dma_src_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dma_unk_2067_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dma_src_step_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t dma_len_status_r();
	uint16_t spg110_2063_r();

	uint16_t dma_manual_r();
	void dma_manual_w(uint16_t data);

	uint16_t tmap0_regs_r(offs_t offset);
	uint16_t tmap1_regs_r(offs_t offset);
	void tmap0_regs_w(offs_t offset, uint16_t data);
	void tmap1_regs_w(offs_t offset, uint16_t data);

	auto write_video_irq_callback() { return m_video_irq_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	address_space_config        m_space_config;

private:
	enum
	{
		PAGE_ENABLE_MASK        = 0x0008,
		PAGE_WALLPAPER_MASK     = 0x0004,

		PAGE_PRIORITY_FLAG_MASK    = 0x3000,
		PAGE_PRIORITY_FLAG_SHIFT   = 12,
		PAGE_TILE_HEIGHT_MASK   = 0x00c0,
		PAGE_TILE_HEIGHT_SHIFT  = 6,
		PAGE_TILE_WIDTH_MASK    = 0x0030,
		PAGE_TILE_WIDTH_SHIFT   = 4,

		TILE_X_FLIP             = 0x0004,
		TILE_Y_FLIP             = 0x0008
	};

	enum flipx_t : bool
	{
		FlipXOff = false,
		FlipXOn = true
	};

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_palram;
	required_shared_ptr<uint16_t> m_palctrlram;
	required_shared_ptr<uint16_t> m_sprtileno;
	required_shared_ptr<uint16_t> m_sprattr1;
	required_shared_ptr<uint16_t> m_sprattr2;

	static uint16_t rgb_to_hsl(uint8_t r, uint8_t g, uint8_t b);
	static std::tuple<uint8_t, uint8_t, uint8_t> hsl_to_rgb(uint16_t hsl);

	void palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t tmap0_regs[0x6];
	uint16_t tmap1_regs[0x6];

	uint16_t m_dma_src_step;
	uint16_t m_dma_dst_step;
	uint16_t m_dma_unk_2061;
	uint16_t m_dma_src_high;

	uint16_t m_dma_dst;
	uint16_t m_dma_src;

	uint16_t m_bg_scrollx;
	uint16_t m_bg_scrolly;
	uint16_t m_2036_scroll;

	uint16_t m_tilebase;

	uint16_t m_video_irq_enable;
	uint16_t m_video_irq_status;
	void check_video_irq();

	void tilemap_write_regs(int which, uint16_t* regs, int regno, uint16_t data);

	template<flipx_t FlipX>
	void draw(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t ctrl, uint32_t bitmap_addr, uint16_t tile, uint8_t yflipmask, uint8_t pal, int32_t h, int32_t w, uint8_t bpp);
	void draw_page(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t bitmap_addr, uint16_t *regs);
	void draw_sprite(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t base_addr);
	void draw_sprites(const rectangle &cliprect, uint32_t scanline, int priority);

	uint32_t m_screenbuf[320 * 240];
	bool m_is_spiderman;

	devcb_write_line m_video_irq_cb;
};

DECLARE_DEVICE_TYPE(SPG110_VIDEO, spg110_video_device)

#endif // MAME_MACHINE_SPG110_VIDEO_H
