// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    deco16ic.h

    Implementation of Data East tilemap ICs
    Data East IC 55 / 56 / 74 / 141

**************************************************************************/
#ifndef MAME_VIDEO_DECO16IC_H
#define MAME_VIDEO_DECO16IC_H

#pragma once


#define DECO_32x32  0
#define DECO_64x32  1
#define DECO_32x64  2
#define DECO_64x64  3

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef device_delegate<int (int bank)> deco16_bank_cb_delegate;

class deco16ic_device : public device_t,
						public device_video_interface
{
public:
	deco16ic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~deco16ic_device() {}

	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
//  void set_palette_tag(const char *tag);
	template <typename... T> void set_bank1_callback(T &&... args) { m_bank1_cb = deco16_bank_cb_delegate(std::forward<T>(args)...); }
	template <typename... T> void set_bank2_callback(T &&... args) { m_bank2_cb = deco16_bank_cb_delegate(std::forward<T>(args)...); }
	void set_split(int split) { m_split = split; }
	void set_pf1_size(int size) { m_pf1_size = size; }
	void set_pf2_size(int size) { m_pf2_size = size; }
	void set_pf1_trans_mask(int mask) { m_pf1_trans_mask = mask; }
	void set_pf2_trans_mask(int mask) { m_pf2_trans_mask = mask; }
	void set_pf1_col_mask(int mask) { m_pf1_colourmask = mask; }
	void set_pf2_col_mask(int mask) { m_pf2_colourmask = mask; }
	void set_pf1_col_bank(int bank) { m_pf1_colour_bank = bank; }
	void set_pf2_col_bank(int bank) { m_pf2_colour_bank = bank; }
	void set_pf12_8x8_bank(int bank) { m_pf12_8x8_gfx_bank = bank; }
	void set_pf12_16x16_bank(int bank) { m_pf12_16x16_gfx_bank = bank; }


	DECLARE_WRITE16_MEMBER( pf1_data_w );
	DECLARE_WRITE16_MEMBER( pf2_data_w );

	DECLARE_READ16_MEMBER( pf1_data_r );
	DECLARE_READ16_MEMBER( pf2_data_r );

	DECLARE_WRITE16_MEMBER( pf_control_w );

	DECLARE_READ16_MEMBER( pf_control_r );

	DECLARE_WRITE32_MEMBER( pf1_data_dword_w );
	DECLARE_WRITE32_MEMBER( pf2_data_dword_w );

	DECLARE_READ32_MEMBER( pf1_data_dword_r );
	DECLARE_READ32_MEMBER( pf2_data_dword_r );

	DECLARE_WRITE32_MEMBER( pf_control_dword_w );

	DECLARE_READ32_MEMBER( pf_control_dword_r );

	void print_debug_info(bitmap_ind16 &bitmap);

	void pf_update(const uint16_t *rowscroll_1_ptr, const uint16_t *rowscroll_2_ptr);

	template<class _BitmapClass>
	void tilemap_1_draw_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int flags, uint32_t priority);
	template<class _BitmapClass>
	void tilemap_2_draw_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int flags, uint32_t priority);
	void tilemap_1_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, uint32_t priority);
	void tilemap_1_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, uint32_t priority);
	void tilemap_2_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, uint32_t priority);
	void tilemap_2_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, uint32_t priority);

	/* used by boogwing, nitrobal */
	void tilemap_12_combine_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, uint32_t priority, int is_tattoo = false);
	void tilemap_12_combine_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, uint32_t priority, int is_tattoo = false);

	/* used by robocop2 */
	void set_tilemap_colour_mask(int tmap, int mask);
	void pf12_set_gfxbank(int small, int big);

	/* used by captaven */
	void set_pf1_8bpp_mode(int mode);

	/* used by stoneage */
	void set_scrolldx(int tmap, int size, int dx, int dx_if_flipped);

	/* used by cninjabl */
	void set_enable(int tmap, int enable );

	template<class _BitmapClass>
	void custom_tilemap_draw(
	screen_device &screen,
	_BitmapClass &bitmap,
	const rectangle &cliprect,
	tilemap_t *tilemap0_8x8,
	tilemap_t *tilemap0_16x16,
	tilemap_t *tilemap1_8x8,
	tilemap_t *tilemap1_16x16,
	const uint16_t *rowscroll_ptr,
	const uint16_t scrollx,
	const uint16_t scrolly,
	const uint16_t control0,
	const uint16_t control1,
	int combine_mask,
	int combine_shift,
	int trans_mask,
	int flags,
	uint32_t priority,
	int is_tattoo
	);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	std::unique_ptr<uint16_t[]> m_pf1_data;
	std::unique_ptr<uint16_t[]> m_pf2_data;
	std::unique_ptr<uint16_t[]> m_pf12_control;

	const uint16_t *m_pf1_rowscroll_ptr, *m_pf2_rowscroll_ptr;

	tilemap_t *m_pf1_tilemap_16x16, *m_pf2_tilemap_16x16;
	tilemap_t *m_pf1_tilemap_8x8, *m_pf2_tilemap_8x8;

	deco16_bank_cb_delegate m_bank1_cb;
	deco16_bank_cb_delegate m_bank2_cb;

	int m_use_custom_pf1, m_use_custom_pf2;
	int m_pf1_bank, m_pf2_bank;
	int m_pf12_last_small, m_pf12_last_big;
	int m_pf1_8bpp_mode;

	int m_split;
	int m_pf1_size;
	int m_pf2_size;
	int m_pf1_trans_mask, m_pf2_trans_mask;
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

#define MCFG_DECO16IC_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#define MCFG_DECO16IC_BANK1_CB(_class, _method) \
	downcast<deco16ic_device &>(*device).set_bank1_callback(&_class::_method, #_class "::" #_method, this);

#define MCFG_DECO16IC_BANK2_CB(_class, _method) \
	downcast<deco16ic_device &>(*device).set_bank2_callback(&_class::_method, #_class "::" #_method, this);

#define MCFG_DECO16IC_SPLIT(_split) \
	downcast<deco16ic_device &>(*device).set_split(_split);

#define MCFG_DECO16IC_PF1_SIZE(_size) \
	downcast<deco16ic_device &>(*device).set_pf1_size(_size);

#define MCFG_DECO16IC_PF2_SIZE(_size) \
	downcast<deco16ic_device &>(*device).set_pf2_size(_size);

#define MCFG_DECO16IC_PF1_TRANS_MASK(_mask) \
	downcast<deco16ic_device &>(*device).set_pf1_trans_mask(_mask);

#define MCFG_DECO16IC_PF2_TRANS_MASK(_mask) \
	downcast<deco16ic_device &>(*device).set_pf2_trans_mask(_mask);

#define MCFG_DECO16IC_PF1_COL_MASK(_mask) \
	downcast<deco16ic_device &>(*device).set_pf1_col_mask(_mask);

#define MCFG_DECO16IC_PF2_COL_MASK(_mask) \
	downcast<deco16ic_device &>(*device).set_pf2_col_mask(_mask);

#define MCFG_DECO16IC_PF1_COL_BANK(_bank) \
	downcast<deco16ic_device &>(*device).set_pf1_col_bank(_bank);

#define MCFG_DECO16IC_PF2_COL_BANK(_bank) \
	downcast<deco16ic_device &>(*device).set_pf2_col_bank(_bank);

#define MCFG_DECO16IC_PF12_8X8_BANK(_bank) \
	downcast<deco16ic_device &>(*device).set_pf12_8x8_bank(_bank);

#define MCFG_DECO16IC_PF12_16X16_BANK(_bank) \
	downcast<deco16ic_device &>(*device).set_pf12_16x16_bank(_bank);

#define MCFG_DECO16IC_GFXDECODE(_gfxtag) \
	downcast<deco16ic_device &>(*device).set_gfxdecode_tag(_gfxtag);

// function definition for a callback
#define DECO16IC_BANK_CB_MEMBER(_name)     int _name(int bank)

#endif // MAME_VIDEO_DECO16IC_H
