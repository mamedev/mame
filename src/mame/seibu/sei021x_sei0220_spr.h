// license:BSD-3-Clause
// copyright-holders:David Graves, Angelo Salese, David Haywood, Tomasz Slanina, Carlos A. Lozano, Bryan McPhail, Pierpaolo Prazzoli
#ifndef MAME_SEIBU_SEI021X_SEI0220_SPR_H
#define MAME_SEIBU_SEI021X_SEI0220_SPR_H

#pragma once

class sei0210_device : public device_t, public device_gfx_interface
{
public:
	typedef device_delegate<u32 (u8 pri, u8 ext)> pri_cb_delegate;
	typedef device_delegate<u32 (u32 code, u8 ext, u8 y)> gfxbank_cb_delegate;

	sei0210_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> sei0210_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: sei0210_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	template <typename... T> void set_pri_callback(T &&... args) { m_pri_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_gfxbank_callback(T &&... args) { m_gfxbank_cb.set(std::forward<T>(args)...); }
	void set_alt_format(bool alt_format) { m_alt_format = alt_format; }
	void set_offset(s32 xoffset, s32 yoffset)
	{
		m_xoffset = xoffset;
		m_yoffset = yoffset;
	}

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle cliprect, u16* spriteram, u16 size);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle cliprect, u16* spriteram, u16 size);

protected:
	sei0210_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual s32 get_coordinate(s32 coordinate)
	{
		return (coordinate & 0x1ff) - ((coordinate & 0x8000) ? 0x200 : 0);
	}

private:
	template<class T>
	void draw(screen_device &screen, T &bitmap, const rectangle cliprect, u16* spriteram, u16 size);

	pri_cb_delegate     m_pri_cb;
	gfxbank_cb_delegate m_gfxbank_cb;

	bool m_alt_format;
	s32 m_xoffset;
	s32 m_yoffset;
};

class sei0211_device : public sei0210_device
{
public:
	sei0211_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> sei0211_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: sei0211_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

protected:
	virtual s32 get_coordinate(s32 coordinate) override
	{
		coordinate &= 0x1ff;
		return (coordinate >= 0x180) ? (coordinate - 0x200) : coordinate;
	}
};

DECLARE_DEVICE_TYPE(SEI0210, sei0210_device)
DECLARE_DEVICE_TYPE(SEI0211, sei0211_device)

#endif // MAME_SEIBU_SEI021X_SEI0220_SPR_H
