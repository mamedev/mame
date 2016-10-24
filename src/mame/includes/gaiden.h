// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino

/***************************************************************************

    Ninja Gaiden

***************************************************************************/

#include "machine/gen_latch.h"
#include "video/tecmo_spr.h"
#include "video/tecmo_mix.h"

class gaiden_state : public driver_device
{
public:
	gaiden_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_mixer(*this, "mixer"),
		m_soundlatch(*this, "soundlatch")
		{ }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint16_t> m_videoram3;
	required_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	tilemap_t   *m_text_layer;
	tilemap_t   *m_foreground;
	tilemap_t   *m_background;
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tile_bitmap_bg;
	bitmap_ind16 m_tile_bitmap_fg;
	uint16_t      m_tx_scroll_x;
	uint16_t      m_tx_scroll_y;
	uint16_t      m_bg_scroll_x;
	uint16_t      m_bg_scroll_y;
	uint16_t      m_fg_scroll_x;
	uint16_t      m_fg_scroll_y;
	int8_t        m_tx_offset_y;
	int8_t        m_bg_offset_y;
	int8_t        m_fg_offset_y;
	int8_t        m_spr_offset_y;

	/* misc */
	int         m_sprite_sizey;
	int         m_prot;
	int         m_jumpcode;
	const int   *m_raiga_jumppoints;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<tecmo_spr_device> m_sprgen;
	optional_device<tecmo_mix_device> m_mixer;
	required_device<generic_latch_8_device> m_soundlatch;

	void gaiden_sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void drgnbowl_sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wildfang_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t wildfang_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void raiga_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t raiga_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void gaiden_flip_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_txscrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_txscrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_fgscrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_fgscrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_bgscrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_bgscrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_txoffsety_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_fgoffsety_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_bgoffsety_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_sproffsety_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_videoram3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_videoram2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaiden_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_raiga();
	void init_drgnbowl();
	void init_drgnbowla();
	void init_mastninj();
	void init_shadoww();
	void init_wildfang();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info_raiga(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_raiga();
	void machine_reset_raiga();
	void video_start_gaiden();
	void video_start_drgnbowl();
	void video_start_raiga();
	uint32_t screen_update_gaiden(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_drgnbowl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_raiga(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void drgnbowl_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void descramble_drgnbowl(int descramble_cpu);
	void descramble_mastninj_gfx(uint8_t* src);
};
