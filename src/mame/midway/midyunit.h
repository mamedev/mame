// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Williams/Midway Y/Z-unit system

**************************************************************************/
#ifndef MAME_MIDWAY_MIDYUNIT_H
#define MAME_MIDWAY_MIDYUNIT_H

#pragma once

#include "williamssound.h"

#include "cpu/tms34010/tms34010.h"
#include "machine/adc0844.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "emupal.h"


class midyunit_base_state : public driver_device
{
protected:
	midyunit_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_nvram(*this, "nvram")
		, m_paletteram(*this, "paletteram")
		, m_gfx_rom(*this, "gfx_rom", 0x800000, ENDIANNESS_BIG)
		, m_mainram(*this, "mainram")
		, m_ports(*this, { { "IN0", "IN1", "IN2", "DSW", "UNK0", "UNK1" } })
	{
	}

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// protection data types
	struct protection_data
	{
		uint16_t  reset_sequence[3]{};
		uint16_t  data_sequence[100]{};
	};

	struct dma_state_t
	{
		uint32_t      offset = 0;         // source offset, in bits
		int32_t       rowbytes = 0;       // source bytes to skip each row
		int32_t       xpos = 0;           // x position, clipped
		int32_t       ypos = 0;           // y position, clipped
		int32_t       width = 0;          // horizontal pixel count
		int32_t       height = 0;         // vertical pixel count
		uint16_t      palette = 0;        // palette base
		uint16_t      color = 0;          // current foreground color with palette
	};

	required_device<tms34010_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<nvram_device> m_nvram;

	required_shared_ptr<uint16_t> m_paletteram;
	memory_share_creator<uint8_t> m_gfx_rom;
	required_shared_ptr<uint16_t> m_mainram;
	optional_ioport_array<6> m_ports;

	std::unique_ptr<uint16_t[]> m_cmos_ram;
	std::unique_ptr<uint8_t[]> m_hidden_ram;
	uint32_t m_cmos_page = 0;
	uint16_t m_prot_result = 0;
	uint16_t m_prot_sequence[3]{};
	uint8_t m_prot_index = 0;
	const struct protection_data *m_prot_data = nullptr;
	uint8_t m_cmos_w_enable = 0;
	uint8_t *m_cvsd_protection_base = nullptr;
	uint8_t m_autoerase_enable = 0;
	uint32_t m_palette_mask = 0;
	std::unique_ptr<pen_t[]> m_pen_map;
	std::unique_ptr<uint16_t[]>   m_local_videoram;
	uint8_t m_videobank_select = 0;
	uint8_t m_yawdim_dma = 0;
	uint16_t m_dma_register[16]{};
	dma_state_t m_dma_state;
	emu_timer *m_dma_timer = nullptr;
	emu_timer *m_autoerase_line_timer = nullptr;

	void cmos_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cmos_r(offs_t offset);
	void cmos_enable_w(address_space &space, uint16_t data);
	uint16_t protection_r();
	uint16_t input_r(offs_t offset);
	uint16_t gfxrom_r(offs_t offset);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vram_r(offs_t offset);
	void control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);
	DECLARE_VIDEO_START(midyunit_4bit);
	DECLARE_VIDEO_START(midyunit_6bit);
	TIMER_CALLBACK_MEMBER(dma_callback);
	TIMER_CALLBACK_MEMBER(autoerase_line);

	void dma_draw(uint16_t command);
	void init_gfxrom(int bpp);
	void install_hidden_ram(mc6809e_device &cpu, int prot_start, int prot_end);

	void main_map(address_map &map) ATTR_COLD;

	void yunit_core(machine_config &config);
	void yunit_4bpp(machine_config &config);
	void yunit_6bpp(machine_config &config);
};

class midzunit_state : public midyunit_base_state
{
public:
	midzunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: midyunit_base_state(mconfig, type, tag)
		, m_narc_sound(*this, "narcsnd")
	{
	}

	void zunit(machine_config &config);

	void init_narc();

	int narc_talkback_strobe_r();
	ioport_value narc_talkback_data_r();

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<williams_narc_sound_device> m_narc_sound;

	void narc_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void zunit_main_map(address_map &map) ATTR_COLD;
};

class midyunit_cvsd_state : public midyunit_base_state
{
public:
	midyunit_cvsd_state(const machine_config &mconfig, device_type type, const char *tag)
		: midyunit_base_state(mconfig, type, tag)
		, m_cvsd_sound(*this, "cvsd")
	{
	}

	void yunit_cvsd_4bit_fast(machine_config &config);
	void yunit_cvsd_4bit_slow(machine_config &config);
	void yunit_cvsd_6bit_slow(machine_config &config);

	void init_hiimpact();
	void init_shimpact();
	void init_smashtv();
	void init_strkforc();
	void init_trog();

protected:
	virtual void machine_reset() override ATTR_COLD;

	required_device<williams_cvsd_sound_device> m_cvsd_sound;

	uint8_t *m_cvsd_protection_base = nullptr;

	void cvsd_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cvsd_protection_w(offs_t offset, uint8_t data);

	void init_generic(int bpp, int sound, int prot_start, int prot_end);

	void cvsd_main_map(address_map &map) ATTR_COLD;

	void yunit_cvsd_core(machine_config &config);
};

class midyunit_adpcm_state : public midyunit_base_state
{
public:
	midyunit_adpcm_state(const machine_config &mconfig, device_type type, const char *tag)
		: midyunit_base_state(mconfig, type, tag)
		, m_adpcm_sound(*this, "adpcm")
	{
	}

	void yunit_adpcm_6bit_fast(machine_config &config);
	void yunit_adpcm_6bit_faster(machine_config &config);

	void init_mkla3bl();
	void init_mkyturbo();
	void init_mkyunit();
	void init_totcarn();

	int adpcm_irq_state_r();

protected:
	virtual void machine_reset() override ATTR_COLD;

	required_device<williams_adpcm_sound_device> m_adpcm_sound;

	void adpcm_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t mkturbo_prot_r();

	void init_generic(int bpp, int prot_start, int prot_end);

	void adpcm_main_map(address_map &map) ATTR_COLD;

	void yunit_adpcm_core(machine_config &config);
};

class term2_state : public midyunit_adpcm_state
{
public:
	term2_state(const machine_config &mconfig, device_type type, const char *tag)
		: midyunit_adpcm_state(mconfig, type, tag)
		, m_adc(*this, "adc")
		, m_left_flash(*this, "Left_Flash_%u", 1U)
		, m_right_flash(*this, "Right_Flash_%u", 1U)
		, m_left_gun_recoil(*this, "Left_Gun_Recoil")
		, m_right_gun_recoil(*this, "Right_Gun_Recoil")
		, m_left_gun_green_led(*this, "Left_Gun_Green_Led")
		, m_left_gun_red_led(*this, "Left_Gun_Red_Led")
		, m_right_gun_green_led(*this, "Right_Gun_Green_Led")
		, m_right_gun_red_led(*this, "Right_Gun_Red_Led")
	{
	}

	void term2(machine_config &config);

	void init_term2();
	void init_term2la1();
	void init_term2la2();
	void init_term2la3();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<adc0844_device> m_adc;

	output_finder<4> m_left_flash;
	output_finder<4> m_right_flash;
	output_finder<> m_left_gun_recoil;
	output_finder<> m_right_gun_recoil;
	output_finder<> m_left_gun_green_led;
	output_finder<> m_left_gun_red_led;
	output_finder<> m_right_gun_green_led;
	output_finder<> m_right_gun_red_led;

	uint16_t *m_t2_hack_mem = nullptr;

	uint16_t term2_input_r(offs_t offset);
	void term2_sound_w(offs_t offset, uint16_t data);
	void term2_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void term2la3_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void term2la2_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void term2la1_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void term2_init_common(write16s_delegate hack_w);

	void term2_main_map(address_map &map) ATTR_COLD;
};

class mkyawdim_state : public midyunit_base_state
{
public:
	mkyawdim_state(const machine_config &mconfig, device_type type, const char *tag)
		: midyunit_base_state(mconfig, type, tag)
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
		, m_oki(*this, "oki")
	{
	}

	void mkyawdim(machine_config &config);
	void mkyawdim2(machine_config &config);

	void init_mkyawdim();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<okim6295_device> m_oki;

	void yawdim_oki_bank_w(uint8_t data);
	void yawdim2_oki_bank_w(uint8_t data);
	void yawdim_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void yawdim2_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void yawdim_main_map(address_map &map) ATTR_COLD;
	void yawdim2_main_map(address_map &map) ATTR_COLD;
	void yawdim_sound_map(address_map &map) ATTR_COLD;
	void yawdim2_sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_MIDWAY_MIDYUNIT_H
