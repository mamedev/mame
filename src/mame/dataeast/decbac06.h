// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#ifndef MAME_DATAEAST_DECBAC06_H
#define MAME_DATAEAST_DECBAC06_H

#pragma once

#include "screen.h"
#include "tilemap.h"

#include <memory>
#include <utility>


class deco_bac06_device : public device_t
{
public:
	deco_bac06_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	typedef device_delegate<void (tile_data &tileinfo, u32 &tile, u32 &colour, u32 &flags)> decbac06_tile_cb_delegate;

	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_gfx_region_wide(int region8x8, int region16x16, int wide)
	{
		m_gfxregion8x8 = region8x8;
		m_gfxregion16x16 = region16x16;
		m_wide = wide;
	}
	void set_thedeep_kludge() { m_thedeep_kludge = true; } // thedeep requires TILE_FLIPX always set, for reasons to be investigated
	void disable_8x8() { m_supports_8x8 = false; }
	void disable_16x16() { m_supports_16x16 = false; }
	void disable_rc_scroll() { m_supports_rc_scroll = false; }
	void set_has_bank(bool has_bank) { m_has_bank = has_bank; }
	template <typename... T> void set_tile_callback(T &&... args) { m_tile_cb.set(std::forward<T>(args)...); }

	void set_transmask(int group, u32 fgmask, u32 bgmask);

	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u32 flags, u8 pri = 0, u8 primask = 0xff);
	void draw_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u32 flags, int mode, int type, u8 pri = 0, u8 primask = 0xff);

	/* I wonder if ctrlreg is really registers, or a selection of pins.

	  For games with multiple chips typically the flip bit only gets set on one of the chips, but
	  is expected to apply to both (and often the sprites as well?)

	  Furthermore we have the m_rambank thing used by Stadium Hero which appears to be used to
	  control the upper address line on some external RAM even if it gets written to the ctrlreg
	  area

	  For now we have this get_flip_state function so that drivers can query the bit and set other
	  flip flags accordingly
	*/
	u8 get_flip_state() { return BIT(m_ctrlreg[0], 7); }

	void set_flip_screen(bool flip);

	/* 16-bit accessors */
	void ctrlreg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 scrollreg_r(offs_t offset);
	void scrollreg_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 vram_r(offs_t offset);
	void rowscroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 rowscroll_r(offs_t offset);
	void colscroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 colscroll_r(offs_t offset);

	/* 8-bit accessors */
	template <bool IsLittleEndian> u8 vram8_r(offs_t offset)
	{
		const u8 shift = BIT(IsLittleEndian ? offset : ~offset, 0) << 3;
		return (vram_r(offset >> 1) >> shift) & 0xff;
	}

	template <bool IsLittleEndian> void vram8_w(offs_t offset, u8 data)
	{
		const u8 shift = BIT(IsLittleEndian ? offset : ~offset, 0) << 3;
		vram_w(offset >> 1, u16(data) << shift, 0xff << shift);
	}

	void ctrlreg8_w(offs_t offset, u8 data)
	{
		ctrlreg_w(offset >> 1, data, 0x00ff); // oscar (mirrors?)
	}

	template <bool IsLittleEndian> void ctrlreg8_packed_w(offs_t offset, u8 data)
	{
		const u8 shift = BIT(IsLittleEndian ? offset : ~offset, 0) << 3;
		ctrlreg_w(offset >> 1, u16(data) << shift, 0xff << shift);
	}

	template <bool IsLittleEndian> u8 scrollreg8_r(offs_t offset)
	{
		const u8 shift = BIT(IsLittleEndian ? offset : ~offset, 0) << 3;
		return (scrollreg_r(offset >> 1) >> shift) & 0xff;
	}

	template <bool IsLittleEndian> void scrollreg8_w(offs_t offset, u8 data)
	{
		const u8 shift = (offset < 4) ?  // these registers are 16-bit?
						BIT(IsLittleEndian ? offset : ~offset, 0) << 3 :
						0; // these registers are 8-bit and mirror? (triothep vs actfancr)
		scrollreg_w(offset >> 1, u16(data) << shift, 0xff << shift);
	}

	template <bool IsLittleEndian> u8 rowscroll8_r(offs_t offset)
	{
		const u8 shift = BIT(IsLittleEndian ? offset : ~offset, 0) << 3;
		return (rowscroll_r(offset >> 1) >> shift) & 0xff;
	}

	template <bool IsLittleEndian> void rowscroll8_w(offs_t offset, u8 data)
	{
		const u8 shift = BIT(IsLittleEndian ? offset : ~offset, 0) << 3;
		rowscroll_w(offset >> 1, u16(data) << shift, 0xff << shift);
	}

	template <bool IsLittleEndian> u8 colscroll8_r(offs_t offset)
	{
		const u8 shift = BIT(IsLittleEndian ? offset : ~offset, 0) << 3;
		return (colscroll_r(offset >> 1) >> shift) & 0xff;
	}

	template <bool IsLittleEndian> void colscroll8_w(offs_t offset, u8 data)
	{
		const u8 shift = BIT(IsLittleEndian ? offset : ~offset, 0) << 3;
		colscroll_w(offset >> 1, u16(data) << shift, 0xff << shift);
	}

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void custom_tilemap_draw(bitmap_ind16 &bitmap,
							bitmap_ind8 &primap,
							const rectangle &cliprect,
							tilemap_t *tilemap_ptr,
							const u16 *rowscroll_ptr,
							const u16 *colscroll_ptr,
							const u16 *control0,
							const u16 *control1,
							u32 flags,
							u8 pri = 0, u8 pmask = 0xff);

	void create_tilemaps(int region8x8, int region16x16);

	TILEMAP_MAPPER_MEMBER(tile_shape0_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape1_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape2_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape0_8x8_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape1_8x8_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape2_8x8_scan);
	TILE_GET_INFO_MEMBER(get_tile_info_8x8);
	TILE_GET_INFO_MEMBER(get_tile_info_16x16);

	required_device<gfxdecode_device> m_gfxdecode;

	memory_share_creator<u16> m_vram;
	memory_share_creator<u16> m_rowscroll;
	memory_share_creator<u16> m_colscroll;

	decbac06_tile_cb_delegate m_tile_cb;

	tilemap_t* m_tilemap_8x8[3]{};
	tilemap_t* m_tilemap_16x16[3]{};
	int m_tile_region_8;
	int m_tile_region_16;

	u8 m_gfxregion8x8;
	u8 m_gfxregion16x16;
	int m_wide;

	// some bootlegs (eg midresb / midresbj) don't appear to actually support the alt modes, they set them and end up with broken gfx on later levels.
	bool m_supports_8x8;
	bool m_supports_16x16;
	bool m_supports_rc_scroll;

	bool m_thedeep_kludge;

	bool m_has_bank;

	u16 m_ctrlreg[8];
	u16 m_scrollreg[8];

	u32 m_rambank; // external connection?
	bool m_flip_screen;

};

DECLARE_DEVICE_TYPE(DECO_BAC06, deco_bac06_device)

#endif // MAME_DATAEAST_DECBAC06_H
