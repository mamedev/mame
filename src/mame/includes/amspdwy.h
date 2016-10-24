// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    American Speedway

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/ym2151.h"

class amspdwy_state : public driver_device
{
public:
	amspdwy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ym2151(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ym2151_device> m_ym2151;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int m_flipscreen;

	/* misc */
	uint8_t m_wheel_old[2];
	uint8_t m_wheel_return[2];

	uint8_t amspdwy_wheel_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t amspdwy_wheel_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void amspdwy_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void amspdwy_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void amspdwy_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void amspdwy_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t amspdwy_sound_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index tilemap_scan_cols_back(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);

	uint32_t screen_update_amspdwy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	uint8_t amspdwy_wheel_r( int index );

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
};
