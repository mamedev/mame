// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert
/***************************************************************************

    Galivan - Cosmo Police

***************************************************************************/

#include "includes/nb1414m4.h"
#include "machine/gen_latch.h"
#include "video/bufsprite.h"

class galivan_state : public driver_device
{
public:
	galivan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_nb1414m4(*this, "nb1414m4"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_device<buffered_spriteram8_device> m_spriteram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_tx_tilemap;
	uint16_t       m_scrollx;
	uint16_t       m_scrolly;
	uint8_t       m_galivan_scrollx[2],m_galivan_scrolly[2];
	uint8_t       m_write_layers;
	uint8_t       m_layers;
	uint8_t       m_ninjemak_dispdisable;

	uint8_t       m_shift_scroll; //youmab
	uint32_t      m_shift_val;
	void galivan_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t soundlatch_clear_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t IO_port_c0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void blit_trigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void youmab_extra_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t youmab_8a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void youmab_81_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void youmab_84_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void youmab_86_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galivan_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galivan_gfxbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ninjemak_gfxbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galivan_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galivan_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_youmab();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void ninjemak_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void ninjemak_get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void palette_init_galivan(palette_device &palette);
	void machine_start_galivan();
	void machine_reset_galivan();
	void video_start_galivan();
	void machine_start_ninjemak();
	void machine_reset_ninjemak();
	void video_start_ninjemak();
	uint32_t screen_update_galivan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ninjemak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	optional_device<nb1414m4_device> m_nb1414m4;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
};
