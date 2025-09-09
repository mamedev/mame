// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*
    Konami Twin16 Video subsystem
*/
#ifndef MAME_KONAMI_TWIN16_V_H
#define MAME_KONAMI_TWIN16_V_H

#pragma once

#include "screen.h"
#include "tilemap.h"

class konami_twin16_video_device : public device_t, public device_video_interface, public device_gfx_interface
{
public:
	// TODO: probably cleaner to implemen these as address spaces where the driver can install ROM/RAM rather than delegates
	using sprite_cb_delegate = device_delegate<uint16_t (int addr)>;
	using tile_cb_delegate = device_delegate<uint32_t (uint16_t data)>;

	// constructor/destructor
	konami_twin16_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> konami_twin16_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: konami_twin16_video_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configurations
	auto virq_callback() { return m_virq_cb.bind(); }
	template <typename... T> void set_sprite_callback(T &&... args) { m_sprite_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_tile_callback(T &&... args) { m_tile_cb.set(std::forward<T>(args)...); }

	// read/write handlers
	void sprite_process_enable_w(uint8_t data);
	uint16_t sprite_status_r();
	void video_register_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t spriteram_r(offs_t offset) { return m_spriteram[0][offset]; }
	uint16_t fixram_r(offs_t offset) { return m_fixram[offset]; }
	template <unsigned Which> uint16_t videoram_r(offs_t offset) { return m_videoram[Which][offset]; }
	void spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fixram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <unsigned Which> void videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0)
	{
		COMBINE_DATA(&m_videoram[Which][offset]);
		m_scroll_tmap[Which]->mark_tile_dirty(offset);
	}

	// accessors
	void mark_scroll_dirty()
	{
		m_scroll_tmap[0]->mark_all_dirty();
		m_scroll_tmap[1]->mark_all_dirty();
	}

	void spriteram_process();

	void screen_vblank(int state);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	memory_share_creator<uint16_t> m_fixram;
	memory_share_array_creator<uint16_t, 2> m_videoram;

	memory_share_array_creator<uint16_t, 2> m_spriteram;
	std::unique_ptr<uint16_t []> m_sprite_buffer;

	emu_timer *m_sprite_timer;
	uint8_t m_sprite_process_enable;
	uint8_t m_sprite_busy;
	uint8_t m_need_process_spriteram;
	uint16_t m_scrollx[3];
	uint16_t m_scrolly[3];
	uint16_t m_video_register;
	tilemap_t *m_fixed_tmap;
	tilemap_t *m_scroll_tmap[2];

	devcb_write_line m_virq_cb;
	sprite_cb_delegate m_sprite_cb;
	tile_cb_delegate m_tile_cb;

	TILE_GET_INFO_MEMBER(fix_tile_info);
	template <unsigned Which> TILE_GET_INFO_MEMBER(scroll_tile_info);
	uint32_t default_tile(uint16_t data) { return data; }

	TIMER_CALLBACK_MEMBER(sprite_tick);

	int set_sprite_timer();
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	bool spriteram_process_enable();
};

DECLARE_DEVICE_TYPE(KONAMI_TWIN16_VIDEO,  konami_twin16_video_device)

#endif // MAME_KONAMI_TWIN16_V_H
