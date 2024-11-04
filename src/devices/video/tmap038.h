// license:BSD-3-Clause
// copyright-holders:Luca Elia, Paul Priest, David Haywood
#ifndef MAME_VIDEO_TMAP038_H
#define MAME_VIDEO_TMAP038_H

#pragma once

#include "tilemap.h"

class tilemap038_device : public device_t
{
public:
	typedef device_delegate<void (bool tiledim, u32 &color, u32 &pri, u32 &code)> tmap038_cb_delegate;

	tilemap038_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: tilemap038_device(mconfig, tag, owner, (u32)0)
	{
	}

	tilemap038_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configurations
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	template <typename... T> void set_tile_callback(T &&... args) { m_038_cb.set(std::forward<T>(args)...); }
	void set_gfx(u16 no) { m_gfxno = no; }
	void set_xoffs(int xoffs, int flipped_xoffs) { m_xoffs = xoffs; m_flipped_xoffs = flipped_xoffs; }
	void set_yoffs(int yoffs, int flipped_yoffs) { m_yoffs = yoffs; m_flipped_yoffs = flipped_yoffs; }

	// call to do the rendering etc.
	template<class BitmapClass>
	void draw_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, u32 flags, u8 pri = 0, u8 pri_mask = ~0);

	void prepare();
	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u32 flags, u8 pri = 0, u8 pri_mask = ~0);
	void draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 flags, u8 pri = 0, u8 pri_mask = ~0);

	// access
	void vram_map(address_map &map) ATTR_COLD;
	void vram_8x8_map(address_map &map) ATTR_COLD;
	void vram_16x16_map(address_map &map) ATTR_COLD;

	void vram_writeonly_map(address_map &map) ATTR_COLD;
	void vram_16x16_writeonly_map(address_map &map) ATTR_COLD;

	u16 vram_8x8_r(offs_t offset);
	void vram_8x8_w(offs_t offset, u16 data, u16 mem_mask);

	u16 vram_16x16_r(offs_t offset);
	void vram_16x16_w(offs_t offset, u16 data, u16 mem_mask);

	u16 lineram_r(offs_t offset) { return m_lineram[offset]; }
	void lineram_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_lineram[offset]); }

	u16 vregs_r(offs_t offset) { return m_vregs[offset]; }
	void vregs_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_vregs[offset]); }

	void mark_all_dirty() { m_tmap->mark_all_dirty(); }
	void set_flip(u32 attributes) { m_tmap->set_flip(attributes); }
	void set_palette_offset(u32 offset) { m_tmap->set_palette_offset(offset); }
	void set_scroll_rows(u32 scroll_rows) { m_tmap->set_scroll_rows(scroll_rows); }
	void set_scroll_cols(u32 scroll_cols) { m_tmap->set_scroll_cols(scroll_cols); }
	void set_scrollx(int which, int value) { m_tmap->set_scrollx(which, value); }
	void set_scrolly(int which, int value) { m_tmap->set_scrolly(which, value); }

	// getters
	u16 lineram(offs_t offset) const { return m_lineram[offset]; }

	u16 rowscroll(offs_t line) const { return rowscroll_en() ? m_lineram[((line & 0x1ff) * 2) + 0] : 0; }
	u16 rowselect(offs_t line) const { return rowselect_en() ? m_lineram[((line & 0x1ff) * 2) + 1] : 0; }

	u16 vregs(offs_t offset) const { return m_vregs[offset]; }

	// vregs
	bool flipx() const         { return BIT(~m_vregs[0], 15); }
	bool rowscroll_en() const  { return BIT(m_vregs[0], 14) && (m_lineram != nullptr); }
	int scrollx() const        { return (m_vregs[0] & 0x1ff) + (flipx() ? m_flipped_xoffs : m_xoffs); }

	bool flipy() const         { return BIT(~m_vregs[1], 15); }
	bool rowselect_en() const  { return BIT(m_vregs[1], 14) && (m_lineram != nullptr); }
	bool tiledim() const       { return m_tiledim; }
	int scrolly() const        { return (m_vregs[1] & 0x1ff) + (flipy() ? m_flipped_yoffs : m_yoffs); }

	bool enable() const        { return BIT(~m_vregs[2], 4); }
	u16 external() const       { return m_vregs[2] & 0xf; }

	bool tile_is_8x8() const   { return (!m_tiledim) || (m_vram_16x16 == nullptr); }
	bool tile_is_16x16() const { return m_tiledim || (m_vram_8x8 == nullptr); }
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	TILE_GET_INFO_MEMBER(get_tile_info);
	optional_shared_ptr<u16> m_vram_8x8;
	optional_shared_ptr<u16> m_vram_16x16;
	optional_shared_ptr<u16> m_lineram;
	std::unique_ptr<u16[]> m_vregs;
	bool m_tiledim;

	// set when creating device
	required_device<gfxdecode_device> m_gfxdecode;
	u16 m_gfxno;

	tmap038_cb_delegate m_038_cb;
	tilemap_t* m_tmap = nullptr;

	int m_xoffs, m_flipped_xoffs;
	int m_yoffs, m_flipped_yoffs;
};


DECLARE_DEVICE_TYPE(TMAP038, tilemap038_device)

#endif // MAME_VIDEO_TMAP038_H
