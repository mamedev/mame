// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Williams/Midway Y/Z-unit system

**************************************************************************/
#ifndef MAME_INCLUDES_MIDYUNIT_H
#define MAME_INCLUDES_MIDYUNIT_H

#pragma once

#include "audio/williams.h"

#include "cpu/tms34010/tms34010.h"
#include "machine/adc0844.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "emupal.h"


class midyunit_state : public driver_device
{
public:
	midyunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki")
		, m_palette(*this, "palette")
		, m_narc_sound(*this, "narcsnd")
		, m_cvsd_sound(*this, "cvsd")
		, m_adpcm_sound(*this, "adpcm")
		, m_soundlatch(*this, "soundlatch")
		, m_term2_adc(*this, "adc")
		, m_nvram(*this, "nvram")
		, m_generic_paletteram_16(*this, "paletteram")
		, m_gfx_rom(*this, "gfx_rom", 0x800000, ENDIANNESS_BIG)
		, m_mainram(*this, "mainram")
		, m_ports(*this, { { "IN0", "IN1", "IN2", "DSW", "UNK0", "UNK1" } })
	{
	}

	void term2(machine_config &config);
	void yunit_cvsd_4bit_fast(machine_config &config);
	void yunit_adpcm_6bit_fast(machine_config &config);
	void yunit_cvsd_6bit_slow(machine_config &config);
	void yunit_cvsd_4bit_slow(machine_config &config);
	void mkyawdim(machine_config &config);
	void yunit_core(machine_config &config);
	void zunit(machine_config &config);
	void yunit_adpcm_6bit_faster(machine_config &config);

	void init_smashtv();
	void init_strkforc();
	void init_narc();
	void init_term2();
	void init_term2la1();
	void init_term2la3();
	void init_mkyunit();
	void init_trog();
	void init_totcarn();
	void init_mkyawdim();
	void init_mkyawdim2();
	void init_shimpact();
	void init_hiimpact();
	void init_mkyturbo();
	void init_term2la2();

	DECLARE_READ_LINE_MEMBER(narc_talkback_strobe_r);
	DECLARE_CUSTOM_INPUT_MEMBER(narc_talkback_data_r);
	DECLARE_READ_LINE_MEMBER(adpcm_irq_state_r);

private:
	/* protection data types */
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
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	required_device<palette_device> m_palette;
	optional_device<williams_narc_sound_device> m_narc_sound;
	optional_device<williams_cvsd_sound_device> m_cvsd_sound;
	optional_device<williams_adpcm_sound_device> m_adpcm_sound;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<adc0844_device> m_term2_adc;
	required_device<nvram_device> m_nvram;

	required_shared_ptr<uint16_t> m_generic_paletteram_16;
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
	uint8_t m_chip_type = 0;
	uint16_t *m_t2_hack_mem = nullptr;
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
	void midyunit_cmos_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t midyunit_cmos_r(offs_t offset);
	void midyunit_cmos_enable_w(address_space &space, uint16_t data);
	uint16_t midyunit_protection_r();
	uint16_t midyunit_input_r(offs_t offset);
	void midyunit_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t term2_input_r(offs_t offset);
	void term2_sound_w(offs_t offset, uint16_t data);
	void term2_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void term2la3_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void term2la2_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void term2la1_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cvsd_protection_w(offs_t offset, uint8_t data);
	uint16_t mkturbo_prot_r();
	uint16_t midyunit_gfxrom_r(offs_t offset);
	void midyunit_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t midyunit_vram_r(offs_t offset);
	void midyunit_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void midyunit_paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t midyunit_dma_r(offs_t offset);
	void midyunit_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void yawdim_oki_bank_w(uint8_t data);
	void yawdim2_oki_bank_w(uint8_t data);
	uint8_t yawdim2_soundlatch_r();
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);
	DECLARE_MACHINE_RESET(midyunit);
	DECLARE_VIDEO_START(midzunit);
	DECLARE_VIDEO_START(midyunit_4bit);
	DECLARE_VIDEO_START(midyunit_6bit);
	DECLARE_VIDEO_START(mkyawdim);
	DECLARE_VIDEO_START(common);
	TIMER_CALLBACK_MEMBER(dma_callback);
	TIMER_CALLBACK_MEMBER(autoerase_line);

	void main_map(address_map &map);
	void yawdim_sound_map(address_map &map);

	void dma_draw(uint16_t command);
	void init_generic(int bpp, int sound, int prot_start, int prot_end);
	void term2_init_common(write16s_delegate hack_w);
};

#endif // MAME_INCLUDES_MIDYUNIT_H
