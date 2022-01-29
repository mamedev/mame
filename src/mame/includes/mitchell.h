// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Mitchell hardware

*************************************************************************/

#include "machine/74157.h"
#include "machine/nvram.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/msm5205.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "tilemap.h"

class mitchell_state : public driver_device
{
public:
	mitchell_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_nvram(*this, "nvram"),
		m_eeprom(*this, "eeprom"),
		m_msm(*this, "msm"),
		m_adpcm_select(*this, "adpcm_select"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_bank1(*this, "bank1"),
		m_bank0d(*this, "bank0d"),
		m_bank1d(*this, "bank1d"),
		m_soundbank(*this, "soundbank") { }

	void pkladiesbl(machine_config &config);
	void mstworld(machine_config &config);
	void mgakuen(machine_config &config);
	void marukin(machine_config &config);
	void pang(machine_config &config);
	void pangba(machine_config &config);
	void pangnv(machine_config &config);
	void spangbl(machine_config &config);

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

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<nvram_device> m_nvram;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<msm5205_device> m_msm;
	optional_device<ls157_device> m_adpcm_select;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank0d;
	optional_memory_bank m_bank1d;
	optional_memory_bank m_soundbank;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	std::vector<uint8_t> m_objram;           /* Sprite RAM */
	int        m_flipscreen;
	int        m_video_bank;
	int        m_paletteram_bank;
	std::vector<uint8_t> m_paletteram;

	/* sound-related */
	bool       m_sample_select;

	/* misc */
	int        m_input_type;
	int        m_dial[2];
	int        m_dial_selected;
	int        m_dir[2];
	int        m_keymatrix;
	std::unique_ptr<uint8_t[]> m_decoded;

	uint8_t m_irq_source;
	uint8_t pang_port5_r();
	void pang_bankswitch_w(uint8_t data);
	uint8_t block_input_r(offs_t offset);
	void block_dial_control_w(uint8_t data);
	uint8_t mahjong_input_r(offs_t offset);
	void mahjong_input_select_w(uint8_t data);
	uint8_t input_r(offs_t offset);
	void input_w(uint8_t data);
	void mstworld_sound_w(uint8_t data);
	void pang_video_bank_w(uint8_t data);
	void mstworld_video_bank_w(uint8_t data);
	void mgakuen_videoram_w(offs_t offset, uint8_t data);
	uint8_t mgakuen_videoram_r(offs_t offset);
	void mgakuen_objram_w(offs_t offset, uint8_t data);
	uint8_t mgakuen_objram_r(offs_t offset);
	void pang_videoram_w(offs_t offset, uint8_t data);
	uint8_t pang_videoram_r(offs_t offset);
	void pang_colorram_w(offs_t offset, uint8_t data);
	uint8_t pang_colorram_r(offs_t offset);
	void pang_gfxctrl_w(uint8_t data);
	void pangbl_gfxctrl_w(uint8_t data);
	void mstworld_gfxctrl_w(uint8_t data);
	void pang_paletteram_w(offs_t offset, uint8_t data);
	uint8_t pang_paletteram_r(offs_t offset);
	void eeprom_cs_w(uint8_t data);
	void eeprom_clock_w(uint8_t data);
	void eeprom_serial_w(uint8_t data);
	void oki_banking_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_MACHINE_START(mitchell);
	DECLARE_MACHINE_RESET(mitchell);
	DECLARE_VIDEO_START(pang);
	uint32_t screen_update_pang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(mitchell_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void bootleg_decode();
	void configure_banks(void (*decode)(uint8_t *src, uint8_t *dst, int size));
	void sound_bankswitch_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(spangbl_adpcm_int);

	void decrypted_opcodes_map(address_map &map);
	void mgakuen_map(address_map &map);
	void mitchell_io_map(address_map &map);
	void mitchell_map(address_map &map);
	void mstworld_io_map(address_map &map);
	void mstworld_sound_map(address_map &map);
	void pangba_sound_map(address_map &map);
	void pkladiesbl_io_map(address_map &map);
	void spangbl_io_map(address_map &map);
	void spangbl_map(address_map &map);
	void spangbl_sound_map(address_map &map);
};
