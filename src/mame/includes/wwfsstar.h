// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "machine/gen_latch.h"

class wwfsstar_state : public driver_device
{
public:
	wwfsstar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_fg0_videoram(*this, "fg0_videoram"),
		m_bg0_videoram(*this, "bg0_videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_fg0_videoram;
	required_shared_ptr<uint16_t> m_bg0_videoram;

	int m_vblank;
	int m_scrollx;
	int m_scrolly;
	tilemap_t *m_fg0_tilemap;
	tilemap_t *m_bg0_tilemap;

	void scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void irqack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fg0_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg0_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	ioport_value vblank_r(ioport_field &field, void *param);

	void scanline(timer_device &timer, void *ptr, int32_t param);

	void get_fg0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index bg0_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_bg0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
