// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Nicola Salmoria, Luca Elia
/***************************************************************************

 Lasso and similar hardware

***************************************************************************/

#include "machine/gen_latch.h"
#include "sound/sn76496.h"

class lasso_state : public driver_device
{
public:
	lasso_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_back_color(*this, "back_color"),
		m_chip_data(*this, "chip_data"),
		m_bitmap_ram(*this, "bitmap_ram"),
		m_last_colors(*this, "last_colors"),
		m_track_scroll(*this, "track_scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_sn_1(*this, "sn76489.1"),
		m_sn_2(*this, "sn76489.2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_back_color;
	optional_shared_ptr<uint8_t> m_chip_data;
	optional_shared_ptr<uint8_t> m_bitmap_ram;    /* 0x2000 bytes for a 256 x 256 x 1 bitmap */
	optional_shared_ptr<uint8_t> m_last_colors;
	optional_shared_ptr<uint8_t> m_track_scroll;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_track_tilemap;
	uint8_t    m_gfxbank;     /* used by lasso, chameleo, wwjgtin and pinbo */
	uint8_t    m_track_enable;    /* used by wwjgtin */

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<sn76489_device> m_sn_1;
	optional_device<sn76489_device> m_sn_2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lasso_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lasso_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lasso_flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lasso_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wwjgtin_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pinbo_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void lasso_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void wwjgtin_get_track_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void pinbo_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_lasso(palette_device &palette);
	void machine_start_wwjgtin();
	void machine_reset_wwjgtin();
	void video_start_wwjgtin();
	void palette_init_wwjgtin(palette_device &palette);
	void video_start_pinbo();
	uint32_t screen_update_lasso(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_chameleo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_wwjgtin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	rgb_t get_color( int data );
	void wwjgtin_set_last_four_colors();
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int reverse );
	void draw_lasso( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
