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


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class deco16ic_device : public device_t, public device_video_interface
{
public:
	static constexpr unsigned DECO_32x32 = 0;
	static constexpr unsigned DECO_64x32 = 1;
	static constexpr unsigned DECO_32x64 = 2;
	static constexpr unsigned DECO_64x64 = 3;

	using deco16_tile_cb_delegate = device_delegate<void (u32 &tile, u32 &colour, int layer, bool is_8x8)>;
	using deco16_bank_cb_delegate = device_delegate<int (int bank)>;
	using deco16_mix_cb_delegate = device_delegate<u16 (u16 p, u16 p2)>;

	deco16ic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
//  void set_palette_tag(const char *tag);
	template <typename... T> void set_tile_callback(T &&... args) { m_tile_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_mix_callback(T &&... args) { m_mix_cb.set(std::forward<T>(args)...); }
	template <unsigned Which, typename... T> void set_bank_callback(T &&... args) { m_tmap[Which].m_bank_cb.set(std::forward<T>(args)...); }
	template <unsigned Which> void set_size(int size) { m_tmap[Which].m_size = size; }
	template <unsigned Which> void set_col_bank(int bank) { m_tmap[Which].m_colour_bank = bank; }
	template <unsigned Which> void set_col_mask(int mask) { m_tmap[Which].m_colour_mask = mask; }
	void set_8x8_bank(int bank) { m_8x8_gfx_bank = bank; }
	void set_16x16_bank(int bank) { m_16x16_gfx_bank = bank; }

	template <unsigned Which> void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0)
	{
		vram_common_w(Which, offset, data, mem_mask);
	}

	template <unsigned Which> u16 vram_r(offs_t offset)
	{
		return vram_common_r(Which, offset);
	}

	void control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 control_r(offs_t offset);

	template <unsigned Which> void vram32_w(offs_t offset, u32 data, u32 mem_mask = ~0)
	{
		mem_mask &= 0xffff;
		vram_common_w(Which, offset, data, mem_mask);
	}

	template <unsigned Which> u32 vram32_r(offs_t offset)
	{
		return vram_common_r(Which, offset) | 0xffff0000;
	}

	void control32_w(offs_t offset, u32 data, u32 mem_mask = ~0)
	{
		mem_mask &= 0xffff;
		control_w(offset, data, mem_mask);
	}

	u32 control32_r(offs_t offset)
	{
		return control_r(offset) | 0xffff0000;
	}

	void print_debug_info(bitmap_ind16 &bitmap);

	void update(const u16 *rowscroll_1_ptr, const u16 *rowscroll_2_ptr);

	template <class BitmapClass>
	void tilemap_1_draw_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);
	template <class BitmapClass>
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
	void set_gfxbank(int small, int big);

	/* used by cninja */
	void set_transmask(int tmap, int group, u32 fgmask, u32 bgmask);

	/* used by stoneage */
	void set_scrolldx(int tmap, int size, int dx, int dx_if_flipped);

	/* used by cninjabl */
	void set_enable(int tmap, int enable);

	/* used by nslasher */
	void set_tilemap_colour_bank(int tmap, int bank);

	template <class BitmapClass>
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
	// device_t implementation overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// devices
	required_device<gfxdecode_device> m_gfxdecode;

	// memory pointers
	memory_share_array_creator<u16, 2> m_vram;

	// internal state
	struct deco16_tmap
	{
		deco16_tmap(deco16ic_device &owner)
			: m_bank_cb(owner)
			, m_rowscroll_ptr(nullptr)
			, m_tilemap_16x16(nullptr)
			, m_tilemap_8x8(nullptr)
			, m_use_custom_pf(false)
			, m_bank(0)
			, m_size(0)
			, m_colour_bank(0)
			, m_colour_mask(0xf)
		{
		}

		// callbacks
		deco16_bank_cb_delegate m_bank_cb;

		// internal states
		const u16 *m_rowscroll_ptr;
		tilemap_t *m_tilemap_16x16;
		tilemap_t *m_tilemap_8x8;

		bool m_use_custom_pf;
		s32 m_bank;
		s32 m_size;
		s32 m_colour_bank;
		s32 m_colour_mask;

		// registers
		u16 m_scrollx;
		u16 m_scrolly;
		u8 m_control0;
		u8 m_control1;
		u8 m_bankval;

		void update();

		void mark_both_all_dirty()
		{
			m_tilemap_16x16->mark_all_dirty();
			m_tilemap_8x8->mark_all_dirty();
		}
	};

	deco16_tmap m_tmap[2];

	deco16_tile_cb_delegate m_tile_cb;

	deco16_mix_cb_delegate m_mix_cb;

	s32 m_last_small, m_last_big;

	s32 m_8x8_gfx_bank, m_16x16_gfx_bank;

	u16 m_control[8];

	u16 vram_common_r(u8 index, offs_t offset);
	void vram_common_w(u8 index, offs_t offset, u16 data, u16 mem_mask = ~0);

	TILEMAP_MAPPER_MEMBER(scan_rows);
	template <unsigned Which, bool Is8x8> TILE_GET_INFO_MEMBER(get_tile_info);
	template <unsigned Which> TILE_GET_INFO_MEMBER(get_tile_info_8x8);
};

DECLARE_DEVICE_TYPE(DECO16IC, deco16ic_device)

#endif // MAME_DATAEAST_DECO16IC_H
