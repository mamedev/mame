// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_DATAEAST_DEC0_H
#define MAME_DATAEAST_DEC0_H

#pragma once

#include "cpu/h6280/h6280.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/74157.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "video/bufsprite.h"
#include "decbac06.h"
#include "decmxc06.h"
#include "emupal.h"
#include "screen.h"

class dec0_state : public driver_device
{
public:
	dec0_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_tilegen(*this, "tilegen%u", 1U),
		m_spritegen(*this, "spritegen"),
		m_spriteram(*this, "spriteram"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "palette"),
		m_ram(*this, "ram"),
		m_mcu(*this, "mcu")
	{ }

	void dec0_base(machine_config &config);
	void dec0(machine_config &config);
	void dec1(machine_config &config);
	void midres(machine_config &config);
	void birdtry(machine_config &config);
	void baddudes(machine_config &config);
	void midresbj(machine_config &config);
	void hbarrel(machine_config &config);
	void bandit(machine_config &config);
	void midresb(machine_config &config);
	void ffantasybl(machine_config &config);
	void drgninjab(machine_config &config);
	void robocopb(machine_config &config);

	void init_hbarrel();
	void init_drgninja();
	void init_ffantasybl();

protected:
	virtual void machine_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device_array<deco_bac06_device, 3> m_tilegen;
	optional_device<deco_mxc06_device> m_spritegen;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_ram;

	uint16_t *m_buffered_spriteram = nullptr;
	uint16_t m_pri = 0U;

	uint16_t dec0_controls_r(offs_t offset);
	uint16_t slyspy_controls_r(offs_t offset);
	void priority_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sprite_mirror_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	DECLARE_VIDEO_START(dec0_nodma);
	DECLARE_VIDEO_START(slyspy);

	uint32_t screen_update_robocop(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void robocop_colpri_cb(u32 &colour, u32 &pri_mask);
	void baddudes_tile_cb(tile_data &tileinfo, u32 &tile, u32 &colour, u32 &flags);

	void set_screen_raw_params_data_east(machine_config &config);

	void h6280_decrypt(const char *cputag);
	void dec0_map(address_map &map) ATTR_COLD;

private:
	enum class mcu_type {
		EMULATED,
		BADDUDES_SIM
	};

	optional_device<cpu_device> m_mcu;

	mcu_type m_game{};
	uint16_t m_i8751_return = 0U;
	uint16_t m_i8751_command = 0U;
	uint8_t m_i8751_ports[4]{};

	void dec0_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t midres_controls_r(offs_t offset);
	uint8_t dec0_mcu_port0_r();
	void dec0_mcu_port0_w(uint8_t data);
	void dec0_mcu_port1_w(uint8_t data);
	void dec0_mcu_port2_w(uint8_t data);
	void dec0_mcu_port3_w(uint8_t data);
	uint16_t ffantasybl_242024_r();

	DECLARE_VIDEO_START(dec0);
	DECLARE_VIDEO_START(baddudes);

	uint32_t screen_update_hbarrel(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_baddudes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_birdtry(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void hbarrel_colpri_cb(u32 &colour, u32 &pri_mask);
	void bandit_colpri_cb(u32 &colour, u32 &pri_mask);
	void midres_colpri_cb(u32 &colour, u32 &pri_mask);

	void baddudes_i8751_write(int data);
	void dec0_i8751_write(int data);
	void dec0_i8751_reset();
	void ffantasybl_map(address_map &map) ATTR_COLD;
	void dec0_tb_map(address_map &map) ATTR_COLD;
	void dec0_s_map(address_map &map) ATTR_COLD;
	void midres_map(address_map &map) ATTR_COLD;
	void midres_s_map(address_map &map) ATTR_COLD;
	void midresb_map(address_map &map) ATTR_COLD;
};


// with 'sub' CPU
class robocop_state : public dec0_state
{
public:
	robocop_state(const machine_config &mconfig, device_type type, const char *tag) :
		dec0_state(mconfig, type, tag),
		m_subcpu(*this, "sub")
	{ }

	void robocop(machine_config &config);

protected:
	required_device<h6280_device> m_subcpu;

private:
	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
};


// different protection method with 'sub' CPU
class hippodrm_state : public robocop_state
{
public:
	hippodrm_state(const machine_config &mconfig, device_type type, const char *tag) :
		robocop_state(mconfig, type, tag),
		m_sharedram(*this, "sharedram")
	{ }

	void hippodrm(machine_config &config);

	void init_hippodrm();

private:
	required_shared_ptr<uint8_t> m_sharedram;

	uint8_t m_prot_msb = 0;
	uint8_t m_prot_lsb = 0;

	uint8_t prot_r(offs_t offset);
	void prot_w(offs_t offset, uint8_t data);
	uint16_t sharedram_r(offs_t offset);
	void sharedram_w(offs_t offset, uint16_t data);

	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
};


// with address map scramble
class slyspy_state : public dec0_state
{
public:
	slyspy_state(const machine_config &mconfig, device_type type, const char *tag) :
		dec0_state(mconfig, type, tag),
		m_pfview(*this, "pfview"),
		m_sndview(*this, "sndview")
	{ }

	void slyspy(machine_config &config);

	void init_slyspy();

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	memory_view m_pfview;
	memory_view m_sndview;

	uint8_t m_prot_state = 0;
	uint8_t m_sound_prot_state = 0;

	uint16_t prot_r(offs_t offset);
	void prot_state_w(uint16_t data);
	uint16_t prot_state_r();
	uint8_t sound_prot_state_r();
	uint8_t sound_prot_state_reset_r();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


class dec0_automat_state : public dec0_state
{
public:
	dec0_automat_state(const machine_config &mconfig, device_type type, const char *tag) :
		dec0_state(mconfig, type, tag),
		m_msm(*this, "msm%u", 1U),
		m_adpcm_select(*this, "adpcm_select%u", 1U),
		m_soundbank(*this, "soundbank")
	{
		std::fill(std::begin(m_automat_scroll_regs), std::end(m_automat_scroll_regs), 0);
	}

	void secretab(machine_config &config);
	void automat(machine_config &config);

private:
	required_device_array<msm5205_device, 2> m_msm;
	required_device_array<ls157_device, 2> m_adpcm_select;
	required_memory_bank m_soundbank;

	bool m_adpcm_toggle[2]{};
	uint16_t m_automat_scroll_regs[4]{};

	void automat_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t automat_palette_r(offs_t offset);
	void automat_palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void automat_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0)
	{
		COMBINE_DATA(&m_automat_scroll_regs[offset]);
	}
	void sound_bankswitch_w(uint8_t data);
	void msm1_vclk_cb(int state);
	void msm2_vclk_cb(int state);

	virtual void machine_start() override ATTR_COLD;

	uint32_t screen_update_automat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_secretab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void automat_map(address_map &map) ATTR_COLD;
	void automat_s_map(address_map &map) ATTR_COLD;
	void secretab_map(address_map &map) ATTR_COLD;
	void secretab_s_map(address_map &map) ATTR_COLD;
};

#endif // MAME_DATAEAST_DEC0_H
