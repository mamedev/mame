// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Blue Print

***************************************************************************/

#include "machine/gen_latch.h"

class blueprnt_state : public driver_device
{
public:
	blueprnt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram") { }

	/* device/memory pointers */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int     m_gfx_bank;

	/* misc */
	int     m_dipsw;

	uint8_t blueprnt_sh_dipsw_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t grasspin_sh_dipsw_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void blueprnt_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blueprnt_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blueprnt_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blueprnt_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blueprnt_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dipsw_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void video_start_blueprnt();
	void palette_init_blueprnt(palette_device &palette);
	uint32_t screen_update_blueprnt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
