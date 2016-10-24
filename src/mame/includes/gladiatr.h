// license:BSD-3-Clause
// copyright-holders:Victor Trucco,Steve Ellenoff,Phil Stroffolino,Tatsuyuki Satoh,Tomasz Slanina,Nicola Salmoria

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class gladiatr_state : public driver_device
{
public:
	gladiatr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_nvram(*this, "nvram") ,
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_textram(*this, "textram"),
		m_generic_paletteram_8(*this, "paletteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_nvram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_textram;
	required_shared_ptr<uint8_t> m_generic_paletteram_8;

	int m_data1;
	int m_data2;
	int m_flag1;
	int m_flag2;
	int m_video_attributes;
	int m_fg_scrollx;
	int m_fg_scrolly;
	int m_bg_scrollx;
	int m_bg_scrolly;
	int m_sprite_bank;
	int m_sprite_buffer;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_fg_tile_bank;
	int m_bg_tile_bank;

	// common
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void textram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spritebuffer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// gladiator specific
	uint8_t gladiator_dsw1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t gladiator_dsw2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t gladiator_controls_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t gladiator_button3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gladiatr_spritebank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gladiatr_video_registers_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gladiatr_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gladiator_cpu_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gladiator_cpu_sound_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gladiatr_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gladiatr_irq_patch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gladiator_int_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gladiator_adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gladiator_ym_irq(int state);

	// ppking specific
	uint8_t ppking_f1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ppking_f6a3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ppking_qx0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ppking_qx1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ppking_qx2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ppking_qx3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ppking_qx2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ppking_qx3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ppking_qx0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ppking_qx1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ppking_video_registers_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_gladiatr();
	void init_ppking();

	void bg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void machine_reset_ppking();
	void video_start_ppking();
	void machine_reset_gladiator();
	void video_start_gladiatr();

	uint32_t screen_update_ppking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gladiatr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void swap_block(uint8_t *src1,uint8_t *src2,int len);
};
