// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, David Haywood
#ifndef MAME_GAELCO_SPLASH_H
#define MAME_GAELCO_SPLASH_H

#pragma once

#include "machine/74259.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
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
		m_msm(*this, "msm%u", 1U),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_outlatch(*this, "outlatch"),
		m_pixelram(*this, "pixelram"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_sound_bank(*this, "sound_bank")
	{ }

	void splash(machine_config &config) ATTR_COLD;

	void init_splash10() ATTR_COLD;
	void init_splash() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// common
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <unsigned Which> void coin_lockout_w(int state);
	template <unsigned Which> void coin_counter_w(int state);

	// splash specific
	void splash_msm5205_int(int state);
	void splash_adpcm_data_w(uint8_t data);
	void splash_adpcm_control_w(uint8_t data);

	//roldfrog and funystrp specific
	void sound_bank_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info_tilemap0);
	TILE_GET_INFO_MEMBER(get_tile_info_tilemap1);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	void funystrp_sound_map(address_map &map) ATTR_COLD;
	void splash_map(address_map &map) ATTR_COLD;
	void splash_sound_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device_array<msm5205_device, 2> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_device<ls259_device> m_outlatch;

	required_shared_ptr<uint16_t> m_pixelram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;

	optional_memory_bank m_sound_bank;

	// driver init configuration
	int m_sprite_attr2_shift = 0;

	tilemap_t *m_bg_tilemap[2]{};

	// splash specific
	uint8_t m_adpcm_data = 0;
};

class roldfrog_state : public splash_state
{
public:
	roldfrog_state(const machine_config &mconfig, device_type type, const char *tag) :
		splash_state(mconfig, type, tag),
		m_bitmap_mode(*this, "bitmap_mode")
	{ }

	void roldfrog(machine_config &config) ATTR_COLD;

	void init_roldfrog() ATTR_COLD;
	void init_rebus() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect) override;

private:
	INTERRUPT_GEN_MEMBER(roldfrog_interrupt);
	void roldfrog_update_irq();

	void roldfrog_map(address_map &map) ATTR_COLD;
	void roldfrog_sound_io_map(address_map &map) ATTR_COLD;
	void roldfrog_sound_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint16_t> m_bitmap_mode;

	//roldfrog specific
	uint16_t m_ret = 0;
	uint8_t m_vblank_irq = 0;
	uint8_t m_sound_irq = 0;

	// roldfrog specific
	uint16_t roldfrog_bombs_r();
	void roldfrog_vblank_ack_w(uint8_t data);
	uint8_t roldfrog_unk_r();
	void ym_irq(int state);
};

class funystrp_state : public splash_state
{
public:
	funystrp_state(const machine_config &mconfig, device_type type, const char *tag) :
		splash_state(mconfig, type, tag),
		m_eeprom(*this, "eeprom"),
		m_funystrp_val(0),
		m_funystrp_ff3cc7_val(0),
		m_funystrp_ff3cc8_val(0)
	{ }

	void funystrp(machine_config &config) ATTR_COLD;
	void ringball(machine_config &config) ATTR_COLD;

	void init_funystrp() ATTR_COLD;
	void init_ringball() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint16_t spr_read(offs_t offset);
	void spr_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t int_source_r();
	template <unsigned Which> void msm_data_w(uint8_t data);
	template <unsigned Which> void msm_interrupt_w(uint8_t data);
	template <unsigned Which> void adpcm_int(int state);
	void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t protection_r(offs_t offset);
	void eeprom_w(uint8_t data);

	uint32_t screen_update_funystrp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void funystrp_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void funystrp_map(address_map &map) ATTR_COLD;
	void funystrp_sound_io_map(address_map &map) ATTR_COLD;
	void ringball_map(address_map &map) ATTR_COLD;

	required_device<eeprom_serial_93cxx_device> m_eeprom;

	uint8_t m_funystrp_val;
	uint8_t m_funystrp_ff3cc7_val;
	uint8_t m_funystrp_ff3cc8_val;
	uint8_t m_msm_data[2]{};
	uint8_t m_msm_toggle[2]{};
	int32_t m_msm_source = 0;
	int32_t m_snd_interrupt_enable[2]{};
};

#endif // MAME_GAELCO_SPLASH_H
