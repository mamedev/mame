// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_DATAEAST_DEC0_H
#define MAME_DATAEAST_DEC0_H

#pragma once

#include "cpu/h6280/h6280.h"
#include "cpu/mcs51/i8051.h"
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
		m_mcu(*this, "mcu"),
		m_soundlatch(*this, "soundlatch"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_tilegen(*this, "tilegen%u", 1U),
		m_spritegen(*this, "spritegen"),
		m_spriteram(*this, "spriteram"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "palette"),
		m_ram(*this, "ram"),
		m_io_inputs(*this, "INPUTS"),
		m_io_system(*this, "SYSTEM"),
		m_io_dsw(*this, "DSW"),
		m_io_an(*this, "AN%u", 0U)
	{ }

	void dec0_base(machine_config &config) ATTR_COLD;
	void dec0(machine_config &config) ATTR_COLD;
	void dec1(machine_config &config) ATTR_COLD;
	void ffantasybl(machine_config &config) ATTR_COLD;
	void midres(machine_config &config) ATTR_COLD;
	void midresbj(machine_config &config) ATTR_COLD;
	void midresb(machine_config &config) ATTR_COLD;
	void robocopb(machine_config &config) ATTR_COLD;

	void init_ffantasybl() ATTR_COLD;

protected:
	virtual void dec0_control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	virtual u16 dec0_controls_r(offs_t offset);

	u16 midres_controls_r(offs_t offset);
	u16 slyspy_controls_r(offs_t offset);
	void priority_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	DECLARE_VIDEO_START(dec0);

	DECLARE_VIDEO_START(dec0_nodma);
	DECLARE_VIDEO_START(slyspy);

	u16 ffantasybl_242024_r();

	u32 screen_update_robocop(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void baddudes_tile_cb(tile_data &tileinfo, u32 &tile, u32 &colour, u32 &flags);

	void midres_colpri_cb(u32 &colour, u32 &pri_mask);
	void robocop_colpri_cb(u32 &colour, u32 &pri_mask);

	void set_screen_raw_params(machine_config &config);

	void h6280_decrypt(const char *cputag) ATTR_COLD;

	void dec0_map(address_map &map) ATTR_COLD;
	void dec0_s_map(address_map &map) ATTR_COLD;
	void ffantasybl_map(address_map &map) ATTR_COLD;
	void midres_map(address_map &map) ATTR_COLD;
	void midres_s_map(address_map &map) ATTR_COLD;
	void midresb_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device_array<deco_bac06_device, 3> m_tilegen;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<palette_device> m_palette;
	required_shared_ptr<u16> m_paletteram;
	required_shared_ptr<u16> m_ram;

	optional_ioport m_io_inputs;
	optional_ioport m_io_system;
	optional_ioport m_io_dsw;
	optional_ioport_array<2> m_io_an;

	u16 *m_buffered_spriteram = nullptr;
	u16 m_pri = 0U;
};


// with I8751 MCU, simulated case (including bootleg)
class drgninjab_state : public dec0_state
{
public:
	drgninjab_state(const machine_config &mconfig, device_type type, const char *tag) :
		dec0_state(mconfig, type, tag)
	{ }

	void drgninjab(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual void dec0_control_w(offs_t offset, u16 data, u16 mem_mask = ~0) override;
	virtual u16 dec0_controls_r(offs_t offset) override;

	virtual void dec0_i8751_w(u16 data);

	void dec0_i8751_reset_w(u16 data = 0);

	DECLARE_VIDEO_START(baddudes);

	u32 screen_update_baddudes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u16 m_i8751_return = 0U;
	u16 m_i8751_command = 0U;
	u8 m_i8751_ports[4]{};
};

// above with emulation
class dec0_8751_state : public drgninjab_state
{
public:
	dec0_8751_state(const machine_config &mconfig, device_type type, const char *tag) :
		drgninjab_state(mconfig, type, tag)
	{ }

	void birdtry(machine_config &config) ATTR_COLD;
	void bandit(machine_config &config) ATTR_COLD;
	void baddudes(machine_config &config) ATTR_COLD;
	void hbarrel(machine_config &config) ATTR_COLD;

protected:
	virtual void dec0_i8751_w(u16 data) override;

private:
	u8 dec0_mcu_port0_r();
	void dec0_mcu_port0_w(u8 data);
	void dec0_mcu_port1_w(u8 data);
	void dec0_mcu_port2_w(u8 data);
	void dec0_mcu_port3_w(u8 data);

	u32 screen_update_bandit(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_birdtry(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_hbarrel(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bandit_colpri_cb(u32 &colour, u32 &pri_mask);
	void hbarrel_colpri_cb(u32 &colour, u32 &pri_mask);

	void dec0_tb_map(address_map &map) ATTR_COLD;
};


// with 'sub' CPU
class robocop_state : public dec0_state
{
public:
	robocop_state(const machine_config &mconfig, device_type type, const char *tag) :
		dec0_state(mconfig, type, tag),
		m_subcpu(*this, "sub")
	{ }

	void robocop(machine_config &config) ATTR_COLD;

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

	void hippodrm(machine_config &config) ATTR_COLD;

	void init_hippodrm() ATTR_COLD;

private:
	u8 prot_r(offs_t offset);
	void prot_w(offs_t offset, u8 data);
	u16 sharedram_r(offs_t offset);
	void sharedram_w(offs_t offset, u16 data);
	void sprite_mirror_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	required_shared_ptr<u8> m_sharedram;

	u8 m_prot_msb = 0;
	u8 m_prot_lsb = 0;
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

	void slyspy(machine_config &config) ATTR_COLD;

	void init_slyspy() ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	u16 prot_r(offs_t offset);
	void prot_state_w(u16 data);
	u16 prot_state_r();
	u8 sound_prot_state_r();
	u8 sound_prot_state_reset_r();

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	memory_view m_pfview;
	memory_view m_sndview;

	u8 m_prot_state = 0;
	u8 m_sound_prot_state = 0;
};

// bootleg hardware
class automat_state : public dec0_state
{
public:
	automat_state(const machine_config &mconfig, device_type type, const char *tag) :
		dec0_state(mconfig, type, tag),
		m_msm(*this, "msm%u", 1U),
		m_adpcm_select(*this, "adpcm_select%u", 1U),
		m_soundbank(*this, "soundbank")
	{
		std::fill(std::begin(m_automat_scroll_regs), std::end(m_automat_scroll_regs), 0);
	}

	void secretab(machine_config &config) ATTR_COLD;
	void automat(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void automat_control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 automat_palette_r(offs_t offset);
	void automat_palette_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void automat_scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0)
	{
		COMBINE_DATA(&m_automat_scroll_regs[offset]);
	}
	void sound_bankswitch_w(u8 data);
	void msm1_vclk_cb(int state);
	void msm2_vclk_cb(int state);

	u32 screen_update_automat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_secretab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void automat_map(address_map &map) ATTR_COLD;
	void automat_s_map(address_map &map) ATTR_COLD;
	void secretab_map(address_map &map) ATTR_COLD;
	void secretab_s_map(address_map &map) ATTR_COLD;

	required_device_array<msm5205_device, 2> m_msm;
	required_device_array<ls157_device, 2> m_adpcm_select;
	required_memory_bank m_soundbank;

	bool m_adpcm_toggle[2]{};
	u16 m_automat_scroll_regs[4]{};
};

#endif // MAME_DATAEAST_DEC0_H
