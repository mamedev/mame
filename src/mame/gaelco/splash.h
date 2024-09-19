// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, David Haywood
#ifndef MAME_GAELCO_SPLASH_H
#define MAME_GAELCO_SPLASH_H

#pragma once

#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/74259.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "tilemap.h"

class splash_state : public driver_device
{
public:
	splash_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_outlatch(*this, "outlatch"),
		m_pixelram(*this, "pixelram"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_bitmap_mode(*this, "bitmap_mode")
	{ }

	void roldfrog(machine_config &config);
	void splash(machine_config &config);

	void init_splash10();
	void init_roldfrog();
	void init_splash();
	void init_rebus();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_device<ls259_device> m_outlatch;

	required_shared_ptr<uint16_t> m_pixelram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_bitmap_mode;

	// driver init configuration
	int m_bitmap_type = 0;
	int m_sprite_attr2_shift = 0;

	tilemap_t *m_bg_tilemap[2]{};

	// splash specific
	int m_adpcm_data = 0;

	//roldfrog specific
	int m_ret = 0;
	int m_vblank_irq = 0;
	int m_sound_irq = 0;

	// common
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void coin1_lockout_w(int state);
	void coin2_lockout_w(int state);
	void coin1_counter_w(int state);
	void coin2_counter_w(int state);

	// splash specific
	void splash_msm5205_int(int state);
	void splash_adpcm_data_w(uint8_t data);
	void splash_adpcm_control_w(uint8_t data);

	// roldfrog specific
	uint16_t roldfrog_bombs_r();
	void roldfrog_vblank_ack_w(uint8_t data);
	uint8_t roldfrog_unk_r();
	void ym_irq(int state);

	//roldfrog and funystrp specific
	void sound_bank_w(uint8_t data);

	virtual void video_start() override ATTR_COLD;
	DECLARE_MACHINE_START(splash);
	DECLARE_MACHINE_START(roldfrog);
	DECLARE_MACHINE_RESET(splash);

	TILE_GET_INFO_MEMBER(get_tile_info_tilemap0);
	TILE_GET_INFO_MEMBER(get_tile_info_tilemap1);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(roldfrog_interrupt);
	void roldfrog_update_irq(  );

	void funystrp_sound_map(address_map &map) ATTR_COLD;
	void roldfrog_map(address_map &map) ATTR_COLD;
	void roldfrog_sound_io_map(address_map &map) ATTR_COLD;
	void roldfrog_sound_map(address_map &map) ATTR_COLD;
	void splash_map(address_map &map) ATTR_COLD;
	void splash_sound_map(address_map &map) ATTR_COLD;
};

class funystrp_state : public splash_state
{
public:
	funystrp_state(const machine_config &mconfig, device_type type, const char *tag) :
		splash_state(mconfig, type, tag),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_eeprom(*this, "eeprom"),
		m_funystrp_val(0),
		m_funystrp_ff3cc7_val(0),
		m_funystrp_ff3cc8_val(0)
	{ }

	void funystrp(machine_config &config);
	void ringball(machine_config &config);

	void init_funystrp();
	void init_ringball();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint16_t spr_read(offs_t offset);
	void spr_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t int_source_r();
	void msm1_data_w(uint8_t data);
	void msm1_interrupt_w(uint8_t data);
	void msm2_interrupt_w(uint8_t data);
	void msm2_data_w(uint8_t data);
	void adpcm_int1(int state);
	void adpcm_int2(int state);
	void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t protection_r(offs_t offset);
	void eeprom_w(uint8_t data);

	uint32_t screen_update_funystrp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void funystrp_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void funystrp_map(address_map &map) ATTR_COLD;
	void funystrp_sound_io_map(address_map &map) ATTR_COLD;
	void ringball_map(address_map &map) ATTR_COLD;

	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	uint8_t m_funystrp_val;
	uint8_t m_funystrp_ff3cc7_val;
	uint8_t m_funystrp_ff3cc8_val;
	int m_msm_data1 = 0;
	int m_msm_data2 = 0;
	int m_msm_toggle1 = 0;
	int m_msm_toggle2 = 0;
	int m_msm_source = 0;
	int m_snd_interrupt_enable1 = 0;
	int m_snd_interrupt_enable2 = 0;
};

#endif // MAME_GAELCO_SPLASH_H
