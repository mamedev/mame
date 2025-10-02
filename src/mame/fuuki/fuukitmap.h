// license:BSD-3-Clause
// copyright-holders: Luca Elia, David Haywood

#ifndef MAME_FUUKI_FUUKITMAP_H
#define MAME_FUUKI_FUUKITMAP_H

#pragma once

#include "tilemap.h"
#include "screen.h"

class fuukitmap_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	typedef device_delegate<void (u8 layer, u32 &colour)> colour_cb_delegate;

	fuukitmap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> fuukitmap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: fuukitmap_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	template <typename... T> void set_colour_callback(T &&... args) { m_colour_cb.set(std::forward<T>(args)...); }

	auto level_1_irq_callback() { return m_level_1_irq_cb.bind(); }
	auto vblank_irq_callback() { return m_vblank_irq_cb.bind(); }
	auto raster_irq_callback() { return m_raster_irq_cb.bind(); }

	void set_xoffs(int xoffs, int xoffs_flip)
	{
		m_xoffs = xoffs;
		m_xoffs_flip = xoffs_flip;
	}

	void set_yoffs(int yoffs, int yoffs_flip)
	{
		m_yoffs = yoffs;
		m_yoffs_flip = yoffs_flip;
	}

	void set_layer2_xoffs(int xoffs) { m_layer2_xoffs = xoffs; }
	void set_layer2_yoffs(int yoffs) { m_layer2_yoffs = yoffs; }

	// getters, setters
	bool flip_screen() { return m_flip; }
	u8 tmap_front() { return m_tmap_front; }
	u8 tmap_middle() { return m_tmap_middle; }
	u8 tmap_back() { return m_tmap_back; }

	void set_transparent_pen(int layer, pen_t pen) { m_tilemap[layer]->set_transparent_pen(pen); }

	void prepare();
	void draw_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 i, int flag, u8 pri, u8 primask = 0xff);

	void vram_map(address_map &map) ATTR_COLD;
	void vregs_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// configurations
	colour_cb_delegate m_colour_cb;

	devcb_write_line m_level_1_irq_cb;
	devcb_write_line m_vblank_irq_cb;
	devcb_write_line m_raster_irq_cb;

	int m_xoffs, m_xoffs_flip;
	int m_yoffs, m_yoffs_flip;
	int m_layer2_xoffs;
	int m_layer2_yoffs;

	// video-related
	tilemap_t *m_tilemap[3];

	memory_share_array_creator<u16, 4> m_vram;
	u16 m_vregs[0x20 / 2];
	u16 m_unknown[2];
	u16 m_priority;

	bool m_flip;
	u8 m_tmap_front;
	u8 m_tmap_middle;
	u8 m_tmap_back;

	// misc
	emu_timer *m_level_1_interrupt_timer;
	emu_timer *m_vblank_interrupt_timer;
	emu_timer *m_raster_interrupt_timer;

	// handlers
	template <int Layer> u16 vram_r(offs_t offset) { return m_vram[Layer][offset]; }
	template <int Layer> void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <int Layer> void vram_buffered_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u16 vregs_r(offs_t offset) { return m_vregs[offset]; }
	void vregs_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u16 unknown_r(offs_t offset) { return m_unknown[offset]; }
	void unknown_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_unknown[offset]); }

	u16 priority_r() { return m_priority; }
	void priority_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_priority); }

	TIMER_CALLBACK_MEMBER(level1_interrupt);
	TIMER_CALLBACK_MEMBER(vblank_interrupt);
	TIMER_CALLBACK_MEMBER(raster_interrupt);

	template <int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
};

DECLARE_DEVICE_TYPE(FUUKI_TILEMAP, fuukitmap_device)

#endif // MAME_FUUKI_FUUKITMAP_H
