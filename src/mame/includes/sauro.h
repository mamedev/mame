// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari

#include "machine/gen_latch.h"
#include "sound/sp0256.h"


class sauro_state : public driver_device
{
public:
	sauro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sp0256(*this, "speech"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<sp0256_device> m_sp0256;
	optional_device<generic_latch_8_device> m_soundlatch; // sauro only

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_videoram2;
	optional_shared_ptr<uint8_t> m_colorram2;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	uint8_t m_palette_bank;

	// common
	void coin1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_bg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// sauro specific
	void sauro_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sauro_sound_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sauro_palette_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sauro_scroll_fg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sauro_videoram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sauro_colorram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_tile_info_bg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_fg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void init_tecfri();
	void video_start_trckydoc();
	void video_start_sauro();

	uint32_t screen_update_trckydoc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sauro(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sauro_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void trckydoc_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
