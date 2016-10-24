// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Atari Centipede hardware

*************************************************************************/

#include "machine/eepromser.h"
#include "sound/ay8910.h"

class centiped_state : public driver_device
{
public:
	centiped_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_rambase(*this, "rambase"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_bullsdrt_tiles_bankram(*this, "bullsdrt_bank"),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_aysnd(*this, "aysnd") { }

	optional_shared_ptr<uint8_t> m_rambase;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_paletteram;
	optional_shared_ptr<uint8_t> m_bullsdrt_tiles_bankram;

	required_device<cpu_device> m_maincpu;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<ay8910_device> m_aysnd;

	uint8_t m_oldpos[4];
	uint8_t m_sign[4];
	uint8_t m_dsw_select;
	uint8_t m_control_select;
	uint8_t m_flipscreen;
	uint8_t m_prg_bank;
	uint8_t m_gfx_bank;
	uint8_t m_bullsdrt_sprites_bank;
	uint8_t m_penmask[64];
	tilemap_t *m_bg_tilemap;

	// drivers/centiped.c
	void irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t centiped_IN0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t centiped_IN2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t milliped_IN1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t milliped_IN2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void input_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void control_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mazeinv_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mazeinv_input_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bullsdrt_data_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_count_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bullsdrt_coin_count_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t caterplr_unknown_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void caterplr_AY8910_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t caterplr_AY8910_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t multiped_eeprom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void multiped_eeprom_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void multiped_prgbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// video/centiped.c
	void centiped_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void centiped_flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void multiped_gfxbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bullsdrt_tilesbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bullsdrt_sprites_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void centiped_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void milliped_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mazeinv_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_multiped();
	void init_bullsdrt();
	void centiped_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void warlords_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void milliped_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bullsdrt_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_centiped();
	void machine_reset_centiped();
	void video_start_centiped();
	void video_start_bullsdrt();
	void machine_reset_magworm();
	void video_start_milliped();
	void video_start_warlords();
	void palette_init_warlords(palette_device &palette);
	uint32_t screen_update_centiped(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bullsdrt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_milliped(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_warlords(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void generate_interrupt(timer_device &timer, void *ptr, int32_t param);
	void init_penmask();
	void init_common();
	void milliped_set_color(offs_t offset, uint8_t data);
	inline int read_trackball(int idx, int switch_port);
};
