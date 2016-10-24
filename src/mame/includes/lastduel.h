// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Last Duel

*************************************************************************/

#include "machine/gen_latch.h"
#include "video/bufsprite.h"

class lastduel_state : public driver_device
{
public:
	lastduel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_vram(*this, "vram"),
		m_scroll1(*this, "scroll1"),
		m_scroll2(*this, "scroll2"),
		m_paletteram(*this, "paletteram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<uint16_t> m_vram;
	required_shared_ptr<uint16_t> m_scroll1;
	required_shared_ptr<uint16_t> m_scroll2;
	required_shared_ptr<uint16_t> m_paletteram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_tx_tilemap;
	uint16_t      m_scroll[8];
	int         m_sprite_flipy_mask;
	int         m_sprite_pri_mask;
	int         m_tilemap_priority;

	void lastduel_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mg_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lastduel_flip_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void lastduel_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void lastduel_scroll1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void lastduel_scroll2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void lastduel_vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void madgear_scroll1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void madgear_scroll2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void lastduel_palette_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ld_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void ld_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fix_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	void machine_start_lastduel();
	void video_start_lastduel();
	void machine_start_madgear();
	void video_start_madgear();
	uint32_t screen_update_lastduel(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_madgear(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void lastduel_timer_cb(timer_device &timer, void *ptr, int32_t param);
	void madgear_timer_cb(timer_device &timer, void *ptr, int32_t param);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
};
