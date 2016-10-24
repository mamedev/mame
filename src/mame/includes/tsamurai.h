// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino

#include "machine/gen_latch.h"

class tsamurai_state : public driver_device
{
public:
	tsamurai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_audio2(*this, "audio2"),
		m_audio3(*this, "audio3"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_audio2;
	optional_device<cpu_device> m_audio3;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch; // vsgongf only

	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_background;
	tilemap_t *m_foreground;

	//common
	int m_flicker;
	int m_textbank1;
	int m_nmi_enabled;

	// tsamurai and m660 specific
	int m_bgcolor;
	int m_sound_command1;
	int m_sound_command2;

	//m660 specific
	int m_textbank2;
	int m_sound_command3;

	//vsgongf specific
	int m_vsgongf_sound_nmi_enabled;
	int m_vsgongf_color;
	int m_key_count; //debug only

	// common
	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coincounter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void textbank1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// tsamurai and m660 specific
	void bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fg_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgcolor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t unknown_d806_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t unknown_d900_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t unknown_d938_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_command1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_command1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sound_command2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// tsamurai specific
	uint8_t tsamurai_unknown_d803_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// m660 specific
	void m660_textbank2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t m660_unknown_d803_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m660_sound_command3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t m660_sound_command3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// vsgongf specific
	void vsgongf_color_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vsgongf_sound_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t vsgongf_a006_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t vsgongf_a100_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vsgongf_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_vsgongf_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	void machine_start_m660();
	void machine_start_tsamurai();
	void machine_start_vsgongf();
	virtual void video_start() override;
	void video_start_m660();
	void video_start_tsamurai();
	void video_start_vsgongf();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_vsgongf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

	void interrupt(device_t &device);
	void vsgongf_sound_interrupt(device_t &device);
};
