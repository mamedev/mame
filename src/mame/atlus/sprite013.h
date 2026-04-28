// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_ATLUS_SPRITE013_H
#define MAME_ATLUS_SPRITE013_H

#pragma once

#include "screen.h"

class sprite013_device : public device_t, public device_video_interface, public device_gfx_interface
{
public:
	using sprite013_colpri_delegate = device_delegate<bool (u8 &dstpri, u32 &colpri)>;
	using sprite013_decrypt_delegate = device_delegate<void ()>;

	sprite013_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> sprite013_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: sprite013_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	template <typename T> void set_spriteram_tag(T &&tag) { m_spriteram.set_tag(std::forward<T>(tag)); }
	template <typename... T> void set_colpri_callback(T &&... args) { m_colpri_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_decrypt_callback(T &&... args) { m_decrypt_cb.set(std::forward<T>(args)...); }
	void set_alt_format(bool alt) { m_alt_format = alt; }
	void set_delay_sprite(bool delay) { m_delay_sprite = delay; }
	void set_offsets(int x_offset, int y_offset, int x_offset_flip, int y_offset_flip)
	{
		m_x_offset = x_offset;
		m_y_offset = y_offset;
		m_x_offset_flip = x_offset_flip;
		m_y_offset_flip = y_offset_flip;
	}
	void set_transpen(int transpen) { m_transpen = transpen; }
	void set_granularity(int granularity) { m_granularity = granularity; }

	// read/write handlers
	u16 videoregs_r(offs_t offset);
	void videoregs_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void get_sprite_info(const rectangle &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// devices
	required_shared_ptr<u16> m_spriteram;
	memory_share_creator<u16> m_videoregs;

	// configurations
	sprite013_colpri_delegate m_colpri_cb;
	sprite013_decrypt_delegate m_decrypt_cb;

	bool m_alt_format;
	bool m_delay_sprite;

	int m_x_offset, m_x_offset_flip;
	int m_y_offset, m_y_offset_flip;
	int m_transpen;
	int m_granularity;

	// runtime configurated
	std::unique_ptr<u8[]> m_sprite_gfx;
	offs_t m_sprite_gfx_mask;
	u32 m_max_sprite_clk;

	// internal states
	bitmap_ind16 m_sprite_bitmap;

	u8 m_spriteram_bank, m_spriteram_bank_delay;

	void draw_sprites(const rectangle &cliprect);
	void draw_single_sprite_nozoom(const rectangle &cliprect,
			u32 code, u16 colpri,
			int x, int y,
			bool flipx, bool flipy,
			int width, int height);
	void draw_single_sprite_zoom(const rectangle &cliprect,
			u32 code, u16 colpri,
			int x, int y,
			bool flipx, bool flipy,
			int width, int height,
			int zoomx, int zoomy);
};

DECLARE_DEVICE_TYPE(SPRITE013, sprite013_device)

#endif // MAME_ATLUS_SPRITE013_H
