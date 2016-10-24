// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Mitchell hardware

*************************************************************************/

#include "sound/okim6295.h"
#include "machine/nvram.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class mitchell_state : public driver_device
{
public:
	mitchell_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki") ,
		m_nvram(*this, "nvram"),
		m_eeprom(*this, "eeprom"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_bank1(*this, "bank1"),
		m_bank0d(*this, "bank0d"),
		m_bank1d(*this, "bank1d") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<nvram_device> m_nvram;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank0d;
	optional_memory_bank m_bank1d;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	std::vector<uint8_t> m_objram;           /* Sprite RAM */
	int        m_flipscreen;
	int        m_video_bank;
	int        m_paletteram_bank;
	std::vector<uint8_t> m_paletteram;

	/* sound-related */
	int        m_sample_buffer;
	int        m_sample_select;

	/* misc */
	int        m_input_type;
	int        m_dial[2];
	int        m_dial_selected;
	int        m_dir[2];
	int        m_keymatrix;
	uint8_t       m_dummy_nvram;

	uint8_t m_irq_source;
	uint8_t pang_port5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pang_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t block_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void block_dial_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mahjong_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mahjong_input_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void input_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mstworld_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pang_video_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mstworld_video_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mgakuen_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mgakuen_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mgakuen_objram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mgakuen_objram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pang_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pang_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pang_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pang_colorram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pang_gfxctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pangbl_gfxctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mstworld_gfxctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pang_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pang_paletteram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void eeprom_cs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void eeprom_clock_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void eeprom_serial_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void oki_banking_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_mgakuen2();
	void init_block();
	void init_pangb();
	void init_qtono1();
	void init_mgakuen();
	void init_hatena();
	void init_mstworld();
	void init_spangbl();
	void init_pkladiesbl();
	void init_spang();
	void init_cworld();
	void init_spangj();
	void init_qsangoku();
	void init_marukin();
	void init_pang();
	void init_sbbros();
	void init_pkladies();
	void init_blockbl();
	void init_dokaben();
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_mitchell();
	void machine_reset_mitchell();
	void video_start_pang();
	uint32_t screen_update_pang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mitchell_irq(timer_device &timer, void *ptr, int32_t param);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void bootleg_decode();
	void configure_banks(void (*decode)(uint8_t *src, uint8_t *dst, int size));
	void spangbl_adpcm_int(int state);
};
