// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#ifndef MAME_SHARED_DECOSPR_H
#define MAME_SHARED_DECOSPR_H

#pragma once

typedef device_delegate<uint16_t (uint16_t pri, bool extpri)> decospr_pri_cb_delegate;
typedef device_delegate<uint16_t (uint16_t col, bool extcol)> decospr_col_cb_delegate;


// function definition for a callback
#define DECOSPR_PRIORITY_CB_MEMBER(_name)   uint16_t _name(uint16_t pri, bool extpri)
#define DECOSPR_COLOUR_CB_MEMBER(_name)     uint16_t _name(uint16_t col, bool extcol)


class decospr_device : public device_t, public device_video_interface, public device_gfx_interface
{
public:
	decospr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> decospr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: decospr_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	template <typename... T> void set_pri_callback(T &&... args) { m_pri_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_col_callback(T &&... args) { m_col_cb.set(std::forward<T>(args)...); }
	void set_alt_format(bool alt) { m_alt_format = alt; }
	void set_is_bootleg(bool is_bootleg) { m_is_bootleg = is_bootleg; }
	void set_bootleg_type(int bootleg_type) { m_bootleg_type = bootleg_type; }
	void set_flipallx(int flipallx) { m_flipallx = flipallx; }
	void set_transpen(int transpen) { m_transpen = transpen; }
	void set_offsets(int x_offset, int y_offset)
	{
		m_x_offset = x_offset;
		m_y_offset = y_offset;
	}

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t* spriteram, int sizewords);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t* spriteram, int sizewords);
	void set_pix_mix_mask(uint16_t mask) { m_pixmask = mask; }
	void set_pix_raw_shift(uint16_t shift) { m_raw_shift = shift; }
	void set_flip_screen(bool flip) { m_flip_screen = flip; }

	void alloc_sprite_bitmap();
	void inefficient_copy_sprite_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t pri, uint16_t priority_mask, uint16_t colbase, uint16_t palmask, uint8_t alpha = 0xff);
	bitmap_ind16& get_sprite_temp_bitmap() { assert(m_sprite_bitmap.valid()); return m_sprite_bitmap; }

	DECOSPR_PRIORITY_CB_MEMBER(default_col_cb);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	decospr_pri_cb_delegate m_pri_cb;
	decospr_col_cb_delegate m_col_cb;
	bitmap_ind16 m_sprite_bitmap;// optional sprite bitmap (should be INDEXED16)
	bool m_alt_format;
	uint16_t m_pixmask;
	uint16_t m_raw_shift;
	bool m_flip_screen;

	// used by various bootleg / clone chips.
	bool m_is_bootleg; // used by various bootlegs (disables masking of sprite tile number when multi-sprite is used)
	int m_bootleg_type; // for Puzzlove, has sprite bits moved around (probably to prevent board swaps)
	int m_x_offset, m_y_offset; // used by various bootlegs
	bool m_flipallx; // used by esd16.cpp - hedpanico, multchmp, and nmg5.cpp
	int m_transpen; // used by fncywld (tumbleb.cpp)

private:
	template<class BitmapClass>
	void draw_sprites_common(BitmapClass &bitmap, const rectangle &cliprect, uint16_t* spriteram, int sizewords);
};

DECLARE_DEVICE_TYPE(DECO_SPRITE, decospr_device)

#endif // MAME_SHARED_DECOSPR_H
