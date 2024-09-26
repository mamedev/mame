// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    deco16ic.h

    Implementation of Data East tilemap ICs
    Data East IC 55 / 56 / 74 / 141

**************************************************************************/
#ifndef MAME_DATAEAST_DECO16IC_H
#define MAME_DATAEAST_DECO16IC_H

#pragma once

#include "tilemap.h"


#define DECO_32x32  0
#define DECO_64x32  1
#define DECO_32x64  2
#define DECO_64x64  3

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef device_delegate<void (u32 &tile, u32 &colour, int layer, bool is_8x8)> deco16_tile_cb_delegate;
typedef device_delegate<int (int bank)> deco16_bank_cb_delegate;
typedef device_delegate<u16 (u16 p, u16 p2)> deco16_mix_cb_delegate;

class deco16ic_device : public device_t, public device_video_interface
{
public:
	deco16ic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
//  void set_palette_tag(const char *tag);
	template <typename... T> void set_tile_callback(T &&... args) { m_tile_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_bank1_callback(T &&... args) { m_bank1_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_bank2_callback(T &&... args) { m_bank2_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_mix_callback(T &&... args) { m_mix_cb.set(std::forward<T>(args)...); }
	void set_pf1_size(int size) { m_pf1_size = size; }
	void set_pf2_size(int size) { m_pf2_size = size; }
	void set_pf1_col_mask(int mask) { m_pf1_colourmask = mask; }
	void set_pf2_col_mask(int mask) { m_pf2_colourmask = mask; }
	void set_pf1_col_bank(int bank) { m_pf1_colour_bank = bank; }
	void set_pf2_col_bank(int bank) { m_pf2_colour_bank = bank; }
	void set_pf12_8x8_bank(int bank) { m_pf12_8x8_gfx_bank = bank; }
	void set_pf12_16x16_bank(int bank) { m_pf12_16x16_gfx_bank = bank; }

	void pf1_data_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void pf2_data_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u16 pf1_data_r(offs_t offset);
	u16 pf2_data_r(offs_t offset);

	void pf_control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 pf_control_r(offs_t offset);

	void pf1_data_dword_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void pf2_data_dword_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 pf1_data_dword_r(offs_t offset);
	u32 pf2_data_dword_r(offs_t offset);

	void pf_control_dword_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pf_control_dword_r(offs_t offset);

	void print_debug_info(bitmap_ind16 &bitmap);

	void pf_update(const u16 *rowscroll_1_ptr, const u16 *rowscroll_2_ptr);

	template<class BitmapClass>
	void tilemap_1_draw_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);
	template<class BitmapClass>
	void tilemap_2_draw_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);
	void tilemap_1_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);
	void tilemap_1_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);
	void tilemap_2_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);
	void tilemap_2_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);

	/* used by boogwing, nitrobal */
	void tilemap_12_combine_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);
	void tilemap_12_combine_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);

	/* used by robocop2 */
	void set_tilemap_colour_mask(int tmap, int mask);
	void pf12_set_gfxbank(int small, int big);

	/* used by cninja */
	void set_transmask(int tmap, int group, u32 fgmask, u32 bgmask);

	/* used by stoneage */
	void set_scrolldx(int tmap, int size, int dx, int dx_if_flipped);

	/* used by cninjabl */
	void set_enable(int tmap, int enable );

	/* used by nslasher */
	void set_tilemap_colour_bank(int tmap, int bank);

	template<class BitmapClass>
	void custom_tilemap_draw(
			screen_device &screen,
			BitmapClass &bitmap,
			const rectangle &cliprect,
			tilemap_t *tilemap0_8x8,
			tilemap_t *tilemap0_16x16,
			tilemap_t *tilemap1_8x8,
			tilemap_t *tilemap1_16x16,
			const u16 *rowscroll_ptr,
			const u16 scrollx,
			const u16 scrolly,
			const u16 control0,
			const u16 control1,
			int combine_mask,
			int combine_shift,
			int flags,
			u8 priority,
			u8 pmask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	std::unique_ptr<u16[]> m_pf1_data;
	std::unique_ptr<u16[]> m_pf2_data;
	std::unique_ptr<u16[]> m_pf12_control;

	const u16 *m_pf1_rowscroll_ptr, *m_pf2_rowscroll_ptr;

	tilemap_t *m_pf1_tilemap_16x16 = nullptr, *m_pf2_tilemap_16x16 = nullptr;
	tilemap_t *m_pf1_tilemap_8x8 = nullptr, *m_pf2_tilemap_8x8 = nullptr;

	deco16_tile_cb_delegate m_tile_cb;

	deco16_bank_cb_delegate m_bank1_cb;
	deco16_bank_cb_delegate m_bank2_cb;

	deco16_mix_cb_delegate m_mix_cb;

	int m_use_custom_pf1, m_use_custom_pf2;
	int m_pf1_bank, m_pf2_bank;
	int m_pf12_last_small, m_pf12_last_big;

	int m_pf1_size;
	int m_pf2_size;
	int m_pf1_colour_bank, m_pf2_colour_bank;
	int m_pf1_colourmask, m_pf2_colourmask;
	int m_pf12_8x8_gfx_bank, m_pf12_16x16_gfx_bank;

	TILEMAP_MAPPER_MEMBER(deco16_scan_rows);
	TILE_GET_INFO_MEMBER(get_pf2_tile_info);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(get_pf2_tile_info_b);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info_b);
	required_device<gfxdecode_device> m_gfxdecode;
};

DECLARE_DEVICE_TYPE(DECO16IC, deco16ic_device)



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

// function definition for a callback
#define DECO16IC_BANK_CB_MEMBER(_name)     int _name(int bank)

#endif // MAME_DATAEAST_DECO16IC_H
