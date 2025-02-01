// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina, Nicola Salmoria, Ville Linde, hap
#ifndef MAME_SEIBU_SEI025X_RISE1X_SPR_H
#define MAME_SEIBU_SEI025X_RISE1X_SPR_H

#pragma once

class sei25x_rise1x_device : public device_t, public device_video_interface, public device_gfx_interface
{
public:
	typedef device_delegate<u32 (u8 pri)> pri_cb_delegate;
	typedef device_delegate<u32 (u32 code, u8 ext)> gfxbank_cb_delegate;

	sei25x_rise1x_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> sei25x_rise1x_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: sei25x_rise1x_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	template <typename... T> void set_pri_callback(T &&... args) { m_pri_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_gfxbank_callback(T &&... args) { m_gfxbank_cb.set(std::forward<T>(args)...); }
	void set_offset(s32 xoffset, s32 yoffset)
	{
		m_xoffset = xoffset;
		m_yoffset = yoffset;
	}
	void set_transpen(u32 transpen) { m_transpen = transpen; }
	void set_pix_raw_shift(u32 raw_shift) { m_pix_raw_shift = raw_shift; }
	void set_pri_raw_shift(u32 raw_shift) { m_pri_raw_shift = raw_shift; }

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle cliprect, u16* spriteram, u16 size);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle cliprect, u16* spriteram, u16 size);

	void alloc_sprite_bitmap();
	bitmap_ind16& get_sprite_temp_bitmap() { assert(m_sprite_bitmap.valid()); return m_sprite_bitmap; }

protected:
	sei25x_rise1x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;

private:
	template<class T>
	void draw(screen_device &screen, T &bitmap, const rectangle cliprect, u16* spriteram, u16 size);

	pri_cb_delegate     m_pri_cb;
	gfxbank_cb_delegate m_gfxbank_cb;

	s32 m_xoffset;
	s32 m_yoffset;
	u32 m_transpen;
	u32 m_pix_raw_shift;
	u32 m_pri_raw_shift;
	bitmap_ind16 m_sprite_bitmap;
};

DECLARE_DEVICE_TYPE(SEI25X_RISE1X, sei25x_rise1x_device)

#endif // MAME_SEIBU_SEI025X_RISE1X_SPR_H
