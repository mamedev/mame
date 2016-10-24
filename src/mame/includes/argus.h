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

	optional_shared_ptr<uint8_t> m_bg0_scrollx;
	optional_shared_ptr<uint8_t> m_bg0_scrolly;
	required_shared_ptr<uint8_t> m_bg1_scrollx;
	required_shared_ptr<uint8_t> m_bg1_scrolly;
	required_shared_ptr<uint8_t> m_paletteram;
	optional_shared_ptr<uint8_t> m_txram;
	optional_shared_ptr<uint8_t> m_bg1ram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_butasan_bg1ram;

	// common
	uint8_t m_bg_status;
	uint8_t m_flipscreen;
	uint16_t m_palette_intensity;

	// argus specific
	std::unique_ptr<uint8_t[]> m_dummy_bg0ram;
	int m_lowbitscroll;
	int m_prvscrollx;

	// butasan specific
	uint8_t *m_butasan_txram;
	uint8_t *m_butasan_bg0ram;
	uint8_t *m_butasan_bg0backram;
	uint8_t *m_butasan_txbackram;
	std::unique_ptr<uint8_t[]> m_butasan_pagedram[2];
	uint8_t m_butasan_page_latch;
	uint8_t m_butasan_bg1_status;
	uint8_t m_butasan_unknown;

	// valtric specific
	uint8_t m_valtric_mosaic;
	bitmap_rgb32 m_mosaicbitmap;
	uint8_t m_valtric_unknown;
	int m_mosaic;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg0_tilemap;
	tilemap_t *m_bg1_tilemap;

	// common
	void bankselect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void valtric_mosaic_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void txram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg1ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// argus specific
	void argus_bg_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void argus_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// butasan specific
	void butasan_bg0_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void butasan_bg1_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void butasan_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void butasan_bg1ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void butasan_pageselect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t butasan_pagedram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void butasan_pagedram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void butasan_unknown_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// valtric specific
	void valtric_bg_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void valtric_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void valtric_unknown_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void argus_get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void argus_get_bg0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void argus_get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void valtric_get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void valtric_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void butasan_get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void butasan_get_bg0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void butasan_get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	void video_start_argus();
	void video_reset_argus();
	void video_start_valtric();
	void video_reset_valtric();
	void video_start_butasan();
	void video_reset_butasan();

	uint32_t screen_update_argus(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_valtric(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_butasan(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void scanline(timer_device &timer, void *ptr, int32_t param);
	void butasan_scanline(timer_device &timer, void *ptr, int32_t param);

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
