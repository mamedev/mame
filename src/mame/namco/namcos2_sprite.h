// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino, Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles, Nicola Salmoria

#ifndef MAME_NAMCO_NAMCOS2_SPRITE_H
#define MAME_NAMCO_NAMCOS2_SPRITE_H

#pragma once

#include "screen.h"
#include "emupal.h"

class namcos2_sprite_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	using ns2_priority_delegate = device_delegate<u32 (u32 pri)>;
	using ns2_mix_delegate = device_delegate<bool (u16 &dest, u8 &destpri, u16 colbase, u16 src, u32 prival)>;

	// construction/destruction
	namcos2_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template <typename T> namcos2_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: namcos2_sprite_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}
	template <typename T> namcos2_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: namcos2_sprite_device(mconfig, tag, owner, 0, std::forward<T>(palette_tag), gfxinfo)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	template <typename T> void set_spriteram_tag(T &&tag) { m_spriteram.set_tag(std::forward<T>(tag)); }

	template <typename... T> void set_priority_callback(T &&... args) { m_pri_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_mix_callback(T &&... args) { m_mix_cb.set(std::forward<T>(args)...); }

	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int control);
	void draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int control);

	void clear_screen_bitmap() { m_screenbitmap.fill(0xffff); }
	void clear_screen_bitmap(const rectangle cliprect) { m_screenbitmap.fill(0xffff, cliprect); }
	bitmap_ind16 &screen_bitmap() { return m_screenbitmap; }

protected:
	// C146: object pixel serialiser and line buffer steering
	class c146
	{
	public:
		void load(u32 ch, bool flip);
		void shift();

		u8 out() const;
		u8 tra() const { return BIT(m_sr_tra, 3); }
		u8 lda(int v1) const { return v1 ? out() : 0xff; }
		u8 ldb(int v1) const { return v1 ? 0xff : out(); }

		// there is no reset on the die, start out idle
		u32 m_sr_col = 0xffffffff;
		u8 m_sr_tra = 0x0f;
	};

	// one entry of the object table, as latched for a frame
	struct sprite_entry
	{
		gfx_element *gfx;
		u32 code;
		u16 pri;
		u16 pal;
		int sx, sy;
		int sizex, sizey;
		u16 lutbank;
		u8 srcx, srcy;
		u8 size;
		bool flipx;
	};

	namcos2_sprite_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 xmask);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// general
	virtual void get_sprites(int control);

	virtual void get_tilenum_and_size(const u16 word0, const u16 word1, u32 &sprn, bool &is_32);

	void copybitmap(screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip);
	void copybitmap(screen_device &screen, bitmap_rgb32 &dest_bmp, const rectangle &clip);

	template <class BitmapClass> void draw_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, int control);

	void add_sprite(gfx_element *gfx, u32 code, u32 color, bool flipx, bool flipy, int sx, int sy, int sizex, int sizey, u32 prival, u8 srcx, u8 srcy, u8 size);
	void draw_line(int y, const rectangle &cliprect);

	required_shared_ptr<u16> m_spriteram;
	required_region_ptr<u8> m_scalelut_region;

	ns2_priority_delegate m_pri_cb;
	ns2_mix_delegate m_mix_cb;

	bitmap_ind16 m_renderbitmap;
	bitmap_ind16 m_screenbitmap;

	u16 m_xmask;

	c146 m_c146;
	sprite_entry m_sprite[128];
	int m_sprite_count = 0;
	u16 m_linebuf[2][0x800]; // line buffers A and B
};

class namcos2_sprite_finallap_device : public namcos2_sprite_device
{
public:
	// construction/destruction
	namcos2_sprite_finallap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> namcos2_sprite_finallap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: namcos2_sprite_finallap_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

protected:
	virtual void get_tilenum_and_size(const u16 word0, const u16 word1, u32& sprn, bool& is_32) override;
};


class namcos2_sprite_metalhawk_device : public namcos2_sprite_device
{
public:
	// construction/destruction
	namcos2_sprite_metalhawk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template <typename T> namcos2_sprite_metalhawk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: namcos2_sprite_metalhawk_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}
	template <typename T> namcos2_sprite_metalhawk_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: namcos2_sprite_metalhawk_device(mconfig, tag, owner, 0, std::forward<T>(palette_tag), gfxinfo)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

protected:
	virtual void get_sprites(int control) override;
};



// device type definition
DECLARE_DEVICE_TYPE(NAMCOS2_SPRITE, namcos2_sprite_device)
DECLARE_DEVICE_TYPE(NAMCOS2_SPRITE_FINALLAP, namcos2_sprite_finallap_device)
DECLARE_DEVICE_TYPE(NAMCOS2_SPRITE_METALHAWK, namcos2_sprite_metalhawk_device)

#endif // MAME_NAMCO_NAMCOS2_SPRITE_H
