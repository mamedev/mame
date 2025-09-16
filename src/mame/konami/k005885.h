// license:BSD-3-Clause
// copyright-holders: hap, Curt Coder, Nicola Salmoria, Mirko Buffoni, Couriersud, Manuel Abadia
// thanks-to: Kenneth Lin (original driver author)
#ifndef MAME_KONAMI_K005885_H
#define MAME_KONAMI_K005885_H

#pragma once

#include "screen.h"
#include "tilemap.h"

class k005885_device : public device_t, public device_video_interface, public device_gfx_interface
{
public:
	using sprite_delegate = device_delegate<void (screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int size, bool flip, gfx_element *sgfx, int &code, int &color, int &sx, int &sy, bool flipx, bool flipy)>;
	using tile_delegate = device_delegate<void (int layer, int attr, int &gfx, int &code, int &color, int &flags, int codebank)>;

	k005885_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template<typename T, typename U>
	k005885_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, const gfx_decode_entry *gfxinfo, T &&palette_tag, U &&screen_tag)
		: k005885_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
		set_screen(std::forward<U>(screen_tag));
	}

	void set_split_tilemap(bool split) { m_split_tilemap = split; }
	auto set_irq_cb() { return m_irq_cb.bind(); }
	auto set_firq_cb() { return m_firq_cb.bind(); }
	auto set_nmi_cb() { return m_nmi_cb.bind(); }
	auto set_flipscreen_cb() { return m_flipscreen_cb.bind(); }
	template <typename... T> void set_sprite_callback(T &&... args) { m_sprite_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_tile_callback(T &&... args) { m_tile_cb.set(std::forward<T>(args)...); }

	bool flipscreen() { return m_flipscreen; }

	// scroll RAM
	u8 scroll_r(offs_t offset) { return m_scrollram[offset & 0x3f]; }
	void scroll_w(offs_t offset, u8 data) { m_scrollram[offset & 0x3f] = data; }

	// tilemap RAM
	u8 vram_r(offs_t offset) { return m_vram[offset]; }
	void vram_w(offs_t offset, u8 data);

	// sprite RAM
	u8 spriteram_r(offs_t offset) { return m_spriteram[offset]; }
	void spriteram_w(offs_t offset, u8 data) { m_spriteram[offset] = data; }

	// registers
	u8 yscroll_r();
	u8 xscroll_lsb_r();
	u8 scrollmode_r();
	u8 bank_r();
	u8 irq_flip_r();
	void yscroll_w(u8 data);
	void xscroll_lsb_w(u8 data);
	void scrollmode_w(u8 data);
	void bank_w(u8 data);
	void irq_flip_w(u8 data);

	// helpers
	u16 get_xscroll() { return m_xscroll; }

	u8 get_scrollmode() { return m_scrollmode; }
	u8 get_tilebank() { return m_tilebank; }
	u8 get_spriterambank() { return m_spriterambank; }
	bool get_nmi_enable() { return m_nmi_enable; }
	bool get_irq_enable() { return m_irq_enable; }
	bool get_firq_enable() { return m_firq_enable; }
	void nmi_set(int state = ASSERT_LINE) { if (state && m_nmi_enable) m_nmi_cb(ASSERT_LINE); }
	void irq_set(int state = ASSERT_LINE) { if (state && m_irq_enable) m_irq_cb(ASSERT_LINE); }
	void firq_set(int state = ASSERT_LINE) { if (state && m_firq_enable) m_firq_cb(ASSERT_LINE); }

	u8 *spriteram() { return &m_spriteram[0]; }

	void tilemap_set_scroll_rows(int i, u32 rows) { m_tilemap[i]->set_scroll_rows(rows); }
	void tilemap_set_scroll_cols(int i, u32 cols) { m_tilemap[i]->set_scroll_cols(cols); }
	void tilemap_set_scrollx(int i, int which, int value) { m_tilemap[i]->set_scrollx(which, value); }
	void tilemap_set_scrolly(int i, int which, int value) { m_tilemap[i]->set_scrolly(which, value); }
	void tilemap_set_flip(int i, u32 attributes) { m_tilemap[i]->set_flip(attributes); }
	void tilemap_set_transparent_pen(int i, pen_t pen) { m_tilemap[i]->set_transparent_pen(pen); }
	void tilemap_draw(int i, screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, u32 flags = TILEMAP_DRAW_ALL_CATEGORIES, u8 priority = 0, u8 priority_mask = 0xff);
	void tilemap_draw(int i, screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect, u32 flags = TILEMAP_DRAW_ALL_CATEGORIES, u8 priority = 0, u8 priority_mask = 0xff);

	void sprite_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bool flip, u8 bank, u32 len, u8 gfxnum);

	void regs_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	memory_share_creator<u8> m_vram;
	memory_share_creator<u8> m_spriteram;

	u8 m_scrollram[0x40];
	u16 m_xscroll;
	u8 m_yscroll;
	u8 m_scrollmode;
	u8 m_tilebank;
	u8 m_spriterambank;
	bool m_nmi_enable;
	bool m_irq_enable;
	bool m_firq_enable;
	bool m_flipscreen;
	tilemap_t *m_tilemap[2];

	// configurations
	bool m_split_tilemap; // or controlled by register?
	devcb_write_line m_flipscreen_cb;
	devcb_write_line m_irq_cb;
	devcb_write_line m_firq_cb;
	devcb_write_line m_nmi_cb;
	sprite_delegate m_sprite_cb;
	tile_delegate m_tile_cb;

	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	template<unsigned Which> TILE_GET_INFO_MEMBER(get_tile_info_split);
	TILE_GET_INFO_MEMBER(get_tile_info_big);
	template <class BitmapClass> void tilemap_draw_common(int i, screen_device &screen, BitmapClass &dest, const rectangle &cliprect, u32 flags = TILEMAP_DRAW_ALL_CATEGORIES, u8 priority = 0, u8 priority_mask = 0xff);
};

DECLARE_DEVICE_TYPE(K005885, k005885_device)

#endif // MAME_KONAMI_K005885_H
