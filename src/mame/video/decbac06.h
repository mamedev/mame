// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#ifndef MAME_VIDEO_DECOBAC06_H
#define MAME_VIDEO_DECOBAC06_H

#pragma once

#include "screen.h"
#include "tilemap.h"

#include <memory>
#include <utility>


typedef device_delegate<void (tile_data &tileinfo, u32 &tile, u32 &colour, u32 &flags)> decbac06_tile_cb_delegate;

class deco_bac06_device : public device_t
{
public:
	deco_bac06_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_gfx_region_wide(int region8x8, int region16x16, int wide)
	{
		m_gfxregion8x8 = region8x8;
		m_gfxregion16x16 = region16x16;
		m_wide = wide;
	}
	void set_thedeep_kludge() { m_thedeep_kludge = 1; } // thedeep requires TILE_FLIPX always set, for reasons to be investigated
	void disable_8x8() { m_supports_8x8 = false; }
	void disable_16x16() { m_supports_16x16 = false; }
	void disable_rc_scroll() { m_supports_rc_scroll = false; }
	template <typename... T> void set_tile_callback(T &&... args) { m_tile_cb.set(std::forward<T>(args)...); }

	void set_transmask(int group, u32 fgmask, u32 bgmask);

	void deco_bac06_pf_draw(screen_device &screen,bitmap_ind16 &bitmap,const rectangle &cliprect,u32 flags, u8 pri = 0, u8 primask = 0xff);
	void deco_bac06_pf_draw_bootleg(screen_device &screen,bitmap_ind16 &bitmap,const rectangle &cliprect,u32 flags, int mode, int type, u8 pri = 0, u8 primask = 0xff);

	/* I wonder if pf_control_0 is really registers, or a selection of pins.

	  For games with multiple chips typically the flip bit only gets set on one of the chips, but
	  is expected to apply to both (and often the sprites as well?)

	  Furthermore we have the m_rambank thing used by Stadium Hero which appears to be used to
	  control the upper address line on some external RAM even if it gets written to the control_0
	  area

	  For now we have this get_flip_state function so that drivers can query the bit and set other
	  flip flags accordingly
	*/
	u8 get_flip_state(void) { return m_pf_control_0[0] & 0x80; }

	void set_flip_screen(bool flip);

	/* 16-bit accessors */

	void pf_control_0_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 pf_control_1_r(offs_t offset);
	void pf_control_1_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void pf_data_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 pf_data_r(offs_t offset);
	void pf_rowscroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 pf_rowscroll_r(offs_t offset);
	void pf_colscroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 pf_colscroll_r(offs_t offset);

	/* 8-bit accessors */

	/* for dec8.cpp, pcktgal.cpp (Big endian CPU (6809, etc) based games) */
	u8 pf_data_8bit_r(offs_t offset);
	void pf_data_8bit_w(offs_t offset, u8 data);

	void pf_control0_8bit_w(offs_t offset, u8 data);
	u8 pf_control1_8bit_r(offs_t offset);
	void pf_control1_8bit_w(offs_t offset, u8 data);

	u8 pf_rowscroll_8bit_r(offs_t offset);
	void pf_rowscroll_8bit_w(offs_t offset, u8 data);

	/* for hippodrm (dec0.cpp) and actfancr / triothep (Little endian CPU (HuC6280, etc) based games)*/
	void pf_control0_8bit_packed_w(offs_t offset, u8 data);
	void pf_control1_8bit_swap_w(offs_t offset, u8 data);
	u8 pf_data_8bit_swap_r(offs_t offset);
	void pf_data_8bit_swap_w(offs_t offset, u8 data);
	u8 pf_rowscroll_8bit_swap_r(offs_t offset);
	void pf_rowscroll_8bit_swap_w(offs_t offset, u8 data);
	u8 pf_colscroll_8bit_swap_r(offs_t offset);
	void pf_colscroll_8bit_swap_w(offs_t offset, u8 data);

// TODO: privatize values
	std::unique_ptr<u16[]> m_pf_data;
	std::unique_ptr<u16[]> m_pf_rowscroll;
	std::unique_ptr<u16[]> m_pf_colscroll;

	tilemap_t* m_pf8x8_tilemap[3];
	tilemap_t* m_pf16x16_tilemap[3];
	int    m_tile_region_8;
	int    m_tile_region_16;

	// some bootlegs (eg midresb / midresbj) don't appear to actually support the alt modes, they set them and end up with broken gfx on later levels.
	bool    m_supports_8x8;
	bool    m_supports_16x16;
	bool    m_supports_rc_scroll;

	void create_tilemaps(int region8x8,int region16x16);
	u16 m_pf_control_0[8];
	u16 m_pf_control_1[8];

	int m_rambank; // external connection?
	bool m_flip_screen;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	u8 m_gfxregion8x8;
	u8 m_gfxregion16x16;
	int m_wide;

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

private:
	TILEMAP_MAPPER_MEMBER(tile_shape0_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape1_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape2_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape0_8x8_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape1_8x8_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape2_8x8_scan);
	TILE_GET_INFO_MEMBER(get_pf8x8_tile_info);
	TILE_GET_INFO_MEMBER(get_pf16x16_tile_info);
	required_device<gfxdecode_device> m_gfxdecode;

	decbac06_tile_cb_delegate m_tile_cb;
	bool m_thedeep_kludge;
};

DECLARE_DEVICE_TYPE(DECO_BAC06, deco_bac06_device)

#endif // MAME_VIDEO_DECOBAC06_H
