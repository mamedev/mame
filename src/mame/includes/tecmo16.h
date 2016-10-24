// license:BSD-3-Clause
// copyright-holders:Hau, Nicola Salmoria

#include "machine/gen_latch.h"
#include "video/tecmo_spr.h"
#include "video/tecmo_mix.h"

class tecmo16_state : public driver_device
{
public:
	tecmo16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_mixer(*this, "mixer"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_charram(*this, "charram"),
		m_spriteram(*this, "spriteram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;
	required_device<tecmo_mix_device> m_mixer;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_colorram;
	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint16_t> m_colorram2;
	required_shared_ptr<uint16_t> m_charram;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tile_bitmap_bg;
	bitmap_ind16 m_tile_bitmap_fg;
	int m_flipscreen;
	int m_game_is_riot;
	uint16_t m_scroll_x_w;
	uint16_t m_scroll_y_w;
	uint16_t m_scroll2_x_w;
	uint16_t m_scroll2_y_w;
	uint16_t m_scroll_char_x_w;
	uint16_t m_scroll_char_y_w;

	void sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void colorram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void videoram2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void colorram2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void charram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scroll_x_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scroll_y_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scroll2_x_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scroll2_y_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scroll_char_x_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scroll_char_y_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void fg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tx_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;
	void video_start_ginkun();
	void video_start_riot();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void save_state();
};
