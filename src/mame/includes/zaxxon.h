// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Sega Zaxxon hardware

***************************************************************************/
#include "sound/samples.h"

class zaxxon_state : public driver_device
{
public:
	zaxxon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_dials(*this, "DIAL.%u", 0),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	required_device<cpu_device> m_maincpu;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_ioport_array<2> m_dials;

	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	uint8_t m_int_enabled;
	uint8_t m_coin_status[3];
	uint8_t m_coin_enable[3];

	uint8_t m_razmataz_dial_pos[2];
	uint16_t m_razmataz_counter;

	uint8_t m_sound_state[3];
	uint8_t m_bg_enable;
	uint8_t m_bg_color;
	uint16_t m_bg_position;
	uint8_t m_fg_color;

	uint8_t m_congo_fg_bank;
	uint8_t m_congo_color_bank;
	uint8_t m_congo_custom[4];

	const uint8_t *m_color_codes;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	void int_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t razmataz_counter_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void zaxxon_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zaxxon_coin_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zaxxon_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zaxxon_fg_color_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zaxxon_bg_position_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zaxxon_bg_color_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zaxxon_bg_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void congo_fg_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void congo_color_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zaxxon_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void congo_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void congo_sprite_custom_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value razmataz_dial_r(ioport_field &field, void *param);
	ioport_value zaxxon_coin_r(ioport_field &field, void *param);
	void service_switch(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void zaxxon_coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void init_razmataz();
	void init_zaxxonj();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void zaxxon_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void razmataz_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void congo_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_zaxxon(palette_device &palette);
	void video_start_razmataz();
	void video_start_congo();
	uint32_t screen_update_zaxxon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_futspy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_razmataz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_congo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_int(device_t &device);
	void zaxxon_sound_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zaxxon_sound_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zaxxon_sound_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void congo_sound_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void congo_sound_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void video_start_common(tilemap_get_info_delegate fg_tile_info);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int skew);
	inline int find_minimum_y(uint8_t value, int flip);
	inline int find_minimum_x(uint8_t value, int flip);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t flipxmask, uint16_t flipymask);
};


/*----------- defined in audio/zaxxon.c -----------*/
MACHINE_CONFIG_EXTERN( zaxxon_samples );
MACHINE_CONFIG_EXTERN( congo_samples );
