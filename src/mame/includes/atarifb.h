// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Patrick Lawrence, Brad Oliver
/*************************************************************************

    Atari Football hardware

*************************************************************************/

#include "sound/discrete.h"


/* Discrete Sound Input Nodes */
#define ATARIFB_WHISTLE_EN      NODE_01
#define ATARIFB_CROWD_DATA      NODE_02
#define ATARIFB_ATTRACT_EN      NODE_03
#define ATARIFB_NOISE_EN        NODE_04
#define ATARIFB_HIT_EN          NODE_05


class atarifb_state : public driver_device
{
public:
	atarifb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_alphap1_videoram(*this, "p1_videoram"),
		m_alphap2_videoram(*this, "p2_videoram"),
		m_field_videoram(*this, "field_videoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll_register(*this, "scroll_register"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"){ }

	/* video-related */
	required_shared_ptr<uint8_t> m_alphap1_videoram;
	required_shared_ptr<uint8_t> m_alphap2_videoram;
	required_shared_ptr<uint8_t> m_field_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll_register;

	tilemap_t  *m_alpha1_tilemap;
	tilemap_t  *m_alpha2_tilemap;
	tilemap_t  *m_field_tilemap;

	/* sound-related */
	int m_CTRLD;
	int m_sign_x_1;
	int m_sign_y_1;
	int m_sign_x_2;
	int m_sign_y_2;
	int m_sign_x_3;
	int m_sign_y_3;
	int m_sign_x_4;
	int m_sign_y_4;
	int m_counter_x_in0;
	int m_counter_y_in0;
	int m_counter_x_in0b;
	int m_counter_y_in0b;
	int m_counter_x_in2;
	int m_counter_y_in2;
	int m_counter_x_in2b;
	int m_counter_y_in2b;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void atarifb_out1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void atarifb4_out1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void abaseb_out1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void soccer_out1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void atarifb_out2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void soccer_out2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void atarifb_out3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t atarifb_in0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t atarifb_in2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t atarifb4_in0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t atarifb4_in2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void atarifb_alpha1_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void atarifb_alpha2_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void atarifb_field_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void alpha1_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void alpha2_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void field_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_atarifb(palette_device &palette);
	uint32_t screen_update_atarifb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_abaseb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_soccer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void get_tile_info_common( tile_data &tileinfo, tilemap_memory_index tile_index, uint8_t *alpha_videoram );
	void draw_playfield_and_alpha( bitmap_ind16 &bitmap, const rectangle &cliprect, int playfield_x_offset, int playfield_y_offset );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int gfx, int is_soccer );
};

/*----------- defined in audio/atarifb.c -----------*/
DISCRETE_SOUND_EXTERN( atarifb );
DISCRETE_SOUND_EXTERN( abaseb );
