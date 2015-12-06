// license:BSD-3-Clause
// copyright-holders:Yochizo
#include "video/jalblend.h"

class argus_state : public driver_device
{
public:
	argus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_blend(*this, "blend"),
		m_bg0_scrollx(*this, "bg0_scrollx"),
		m_bg0_scrolly(*this, "bg0_scrolly"),
		m_bg1_scrollx(*this, "bg1_scrollx"),
		m_bg1_scrolly(*this, "bg1_scrolly"),
		m_paletteram(*this, "paletteram"),
		m_txram(*this, "txram"),
		m_bg1ram(*this, "bg1ram"),
		m_spriteram(*this, "spriteram"),
		m_butasan_bg1ram(*this, "butasan_bg1ram")  { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<jaleco_blend_device> m_blend;

	optional_shared_ptr<UINT8> m_bg0_scrollx;
	optional_shared_ptr<UINT8> m_bg0_scrolly;
	required_shared_ptr<UINT8> m_bg1_scrollx;
	required_shared_ptr<UINT8> m_bg1_scrolly;
	required_shared_ptr<UINT8> m_paletteram;
	optional_shared_ptr<UINT8> m_txram;
	optional_shared_ptr<UINT8> m_bg1ram;
	required_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_butasan_bg1ram;

	// common
	UINT8 m_bg_status;
	UINT8 m_flipscreen;
	UINT16 m_palette_intensity;

	// argus specific
	UINT8 *m_dummy_bg0ram;
	int m_lowbitscroll;
	int m_prvscrollx;

	// butasan specific
	UINT8 *m_butasan_txram;
	UINT8 *m_butasan_bg0ram;
	UINT8 *m_butasan_bg0backram;
	UINT8 *m_butasan_txbackram;
	UINT8 *m_butasan_pagedram[2];
	UINT8 m_butasan_page_latch;
	UINT8 m_butasan_bg1_status;
	UINT8 m_butasan_unknown;

	// valtric specific
	UINT8 m_valtric_mosaic;
	bitmap_rgb32 m_mosaicbitmap;
	UINT8 m_valtric_unknown;
	int m_mosaic;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg0_tilemap;
	tilemap_t *m_bg1_tilemap;

	// common
	DECLARE_WRITE8_MEMBER(bankselect_w);
	DECLARE_WRITE8_MEMBER(valtric_mosaic_w);
	DECLARE_WRITE8_MEMBER(txram_w);
	DECLARE_WRITE8_MEMBER(bg1ram_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);

	// argus specific
	DECLARE_WRITE8_MEMBER(argus_bg_status_w);
	DECLARE_WRITE8_MEMBER(argus_paletteram_w);

	// butasan specific
	DECLARE_WRITE8_MEMBER(butasan_bg0_status_w);
	DECLARE_WRITE8_MEMBER(butasan_bg1_status_w);
	DECLARE_WRITE8_MEMBER(butasan_paletteram_w);
	DECLARE_READ8_MEMBER(butasan_bg1ram_r);
	DECLARE_WRITE8_MEMBER(butasan_bg1ram_w);
	DECLARE_WRITE8_MEMBER(butasan_pageselect_w);
	DECLARE_READ8_MEMBER(butasan_pagedram_r);
	DECLARE_WRITE8_MEMBER(butasan_pagedram_w);
	DECLARE_WRITE8_MEMBER(butasan_unknown_w);

	// valtric specific
	DECLARE_WRITE8_MEMBER(valtric_bg_status_w);
	DECLARE_WRITE8_MEMBER(valtric_paletteram_w);
	DECLARE_WRITE8_MEMBER(valtric_unknown_w);

	TILE_GET_INFO_MEMBER(argus_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(argus_get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(argus_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(valtric_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(valtric_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(butasan_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(butasan_get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(butasan_get_bg1_tile_info);

	virtual void machine_start() override;
	DECLARE_VIDEO_START(argus);
	DECLARE_VIDEO_RESET(argus);
	DECLARE_VIDEO_START(valtric);
	DECLARE_VIDEO_RESET(valtric);
	DECLARE_VIDEO_START(butasan);
	DECLARE_VIDEO_RESET(butasan);

	UINT32 screen_update_argus(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_valtric(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_butasan(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(butasan_scanline);

	void reset_common();
	void change_palette(int color, int lo_offs, int hi_offs);
	void change_bg_palette(int color, int lo_offs, int hi_offs);
	void bg_setting();

	// argus specific
	void argus_bg0_scroll_handle();
	void argus_write_dummy_rams(int dramoffs, int vromoffs);
	void argus_draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int priority);

	// butasan specific
	void butasan_draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void butasan_log_vram();

	// valtric specific
	void valtric_draw_mosaic(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void valtric_draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
};
