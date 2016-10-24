// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Zero Hour / Red Clash

*************************************************************************/

#include "includes/ladybug.h"

class redclash_state : public ladybug_state
{
public:
	redclash_state(const machine_config &mconfig, device_type type, const char *tag)
		: ladybug_state(mconfig, type, tag) { }

	tilemap_t    *m_fg_tilemap; // redclash
	int        m_gfxbank;   // redclash only

	/* misc */
	uint8_t      m_sraider_0x30;
	uint8_t      m_sraider_0x38;

	uint8_t sraider_sound_low_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sraider_sound_high_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sraider_sound_low_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sraider_sound_high_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sraider_8005_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sraider_misc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sraider_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void left_coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void right_coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void init_redclash();
	void machine_start_sraider();
	void machine_reset_sraider();
	void video_start_sraider();
	void palette_init_sraider(palette_device &palette);
	void machine_start_redclash();
	void machine_reset_redclash();
	void video_start_redclash();
	void palette_init_redclash(palette_device &palette);
	uint32_t screen_update_sraider(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_redclash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_sraider(screen_device &screen, bool state);
	void screen_eof_redclash(screen_device &screen, bool state);
	void redclash_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void redclash_gfxbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void redclash_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void redclash_star0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void redclash_star1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void redclash_star2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void redclash_star_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irqack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/* sraider uses the zerohour star generator board */
	void redclash_set_stars_enable(uint8_t on);
	void redclash_update_stars_state();
	void redclash_set_stars_speed(uint8_t speed);
	void redclash_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t palette_offset, uint8_t sraider, uint8_t firstx, uint8_t lastx);
	void redclash_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void redclash_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
