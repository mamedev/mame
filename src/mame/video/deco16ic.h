/*************************************************************************

    deco16ic.h

    Implementation of Data East tilemap ICs
    Data East IC 55 / 56 / 74 / 141

**************************************************************************/

#pragma once
#ifndef __DECO16IC_H__
#define __DECO16IC_H__



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef int (*deco16_bank_cb)( const int bank );


struct deco16ic_interface
{
	int                m_split;
	int                m_full_width12;

	int                m_pf1_trans_mask, m_pf2_trans_mask;
	int                m_pf1_colour_bank, m_pf2_colour_bank;
	int                m_pf1_colourmask, m_pf2_colourmask;
	deco16_bank_cb     m_bank_cb0, m_bank_cb1;
	int                m_pf12_8x8_gfx_bank, m_pf12_16x16_gfx_bank;
};

class deco16ic_device : public device_t,
						public device_video_interface,
						public deco16ic_interface
{
public:
	deco16ic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~deco16ic_device() {}
	
	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	

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

	void pf_update(const UINT16 *rowscroll_1_ptr, const UINT16 *rowscroll_2_ptr);

	template<class _BitmapClass>
	void tilemap_1_draw_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int flags, UINT32 priority);
	template<class _BitmapClass>
	void tilemap_2_draw_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int flags, UINT32 priority);
	void tilemap_1_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority);
	void tilemap_1_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, UINT32 priority);
	void tilemap_2_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority);
	void tilemap_2_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, UINT32 priority);

	/* used by boogwing, nitrobal */
	void tilemap_12_combine_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority, int is_tattoo = false);
	void tilemap_12_combine_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, UINT32 priority, int is_tattoo = false);

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
	const UINT16 *rowscroll_ptr,
	const UINT16 scrollx,
	const UINT16 scrolly,
	const UINT16 control0,
	const UINT16 control1,
	int combine_mask,
	int combine_shift,
	int trans_mask,
	int flags,
	UINT32 priority,
	int is_tattoo
	);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT16 *m_pf1_data, *m_pf2_data;
	UINT16 *m_pf12_control;

	const UINT16 *m_pf1_rowscroll_ptr, *m_pf2_rowscroll_ptr;

	tilemap_t *m_pf1_tilemap_16x16, *m_pf2_tilemap_16x16;
	tilemap_t *m_pf1_tilemap_8x8, *m_pf2_tilemap_8x8;

	deco16_bank_cb  m_bank_cb_func[2];

	int m_use_custom_pf1, m_use_custom_pf2;

	int m_pf1_bank, m_pf2_bank;

	int m_pf12_last_small, m_pf12_last_big;

	int m_pf1_8bpp_mode;

	TILEMAP_MAPPER_MEMBER(deco16_scan_rows);
	TILE_GET_INFO_MEMBER(get_pf2_tile_info);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(get_pf2_tile_info_b);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info_b);
	required_device<gfxdecode_device> m_gfxdecode;
};

extern const device_type DECO16IC;



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DECO16IC_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, DECO16IC, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_DECO16IC_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#define MCFG_DECO16IC_GFXDECODE(_gfxtag) \
	deco16ic_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#endif
