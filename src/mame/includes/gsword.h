// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,Jarek Parchanski

#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"

class gsword_state : public driver_device
{
public:
	gsword_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_ay0(*this, "ay1"),
		m_ay1(*this, "ay2"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spritetile_ram(*this, "spritetile_ram"),
		m_spritexy_ram(*this, "spritexy_ram"),
		m_spriteattrib_ram(*this, "spriteattram"),
		m_videoram(*this, "videoram"),
		m_cpu2_ram(*this, "cpu2_ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_subcpu;
	required_device<ay8910_device> m_ay0;
	required_device<ay8910_device> m_ay1;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch; // not josvolly

	required_shared_ptr<uint8_t> m_spritetile_ram;
	required_shared_ptr<uint8_t> m_spritexy_ram;
	required_shared_ptr<uint8_t> m_spriteattrib_ram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_cpu2_ram;

	int m_coins; //currently initialized but not used
	int m_fake8910_0;
	int m_fake8910_1;
	int m_nmi_enable;
	int m_protect_hack;
	int m_charbank;
	int m_charpalbank;
	int m_flipscreen;
	tilemap_t *m_bg_tilemap;

	// common
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void charbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_soundcommand_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_set_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_control_port_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_control_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fake_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t fake_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// gsword specific
	uint8_t gsword_hack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gsword_adpcm_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gsword_8741_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t gsword_8741_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void init_gsword();
	void init_gsword2();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_gsword(palette_device &palette);
	void palette_init_josvolly(palette_device &palette);

	uint32_t screen_update_gsword(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gsword_snd_interrupt(device_t &device);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
