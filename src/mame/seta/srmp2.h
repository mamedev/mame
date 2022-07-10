// license:BSD-3-Clause
// copyright-holders:Yochizo, Takahiro Nogi
#ifndef MAME_INCLUDES_SRMP2_H
#define MAME_INCLUDES_SRMP2_H

#pragma once

#include "sound/msm5205.h"
#include "video/x1_001.h"
#include "emupal.h"

class srmp2_state : public driver_device
{
public:
	struct iox_t
	{
		int reset = 0, ff_event,ff_1 = 0, protcheck[4]{}, protlatch[4]{};
		uint8_t data = 0;
		uint8_t mux = 0;
		uint8_t ff = 0;
	};

	srmp2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_spritegen(*this, "spritegen"),
		m_msm(*this, "msm"),
		m_adpcm_rom(*this, "adpcm"),
		m_mainbank(*this, "mainbank")
	{ }

	void mjyuugi(machine_config &config);
	void srmp2(machine_config &config);
	void rmgoldyh(machine_config &config);
	void srmp3(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<x1_001_device> m_spritegen;
	required_device<msm5205_device> m_msm;
	required_region_ptr<uint8_t> m_adpcm_rom;
	optional_memory_bank m_mainbank;

	uint8_t m_color_bank = 0;
	uint8_t m_gfx_bank = 0;
	uint8_t m_adpcm_bank = 0;
	int16_t m_adpcm_data = 0;
	uint32_t m_adpcm_sptr = 0;
	uint32_t m_adpcm_eptr = 0;
	iox_t m_iox;

	// common
	uint8_t vox_status_r();
	uint8_t iox_mux_r();
	uint8_t iox_status_r();
	void iox_command_w(uint8_t data);
	void iox_data_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	// mjuugi
	void mjyuugi_flags_w(uint16_t data);
	void mjyuugi_adpcm_bank_w(uint16_t data);
	uint8_t mjyuugi_irq2_ack_r();
	uint8_t mjyuugi_irq4_ack_r();

	// rmgoldyh
	void rmgoldyh_rombank_w(uint8_t data);

	// srmp2
	void srmp2_irq2_ack_w(uint8_t data);
	void srmp2_irq4_ack_w(uint8_t data);
	void srmp2_flags_w(uint16_t data);
	void adpcm_code_w(uint8_t data);

	// srmp3
	void srmp3_rombank_w(uint8_t data);
	void srmp3_flags_w(uint8_t data);
	void srmp3_irq_ack_w(uint8_t data);

	virtual void machine_start() override;
	DECLARE_MACHINE_START(srmp2);
	void srmp2_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(srmp3);
	void srmp3_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(rmgoldyh);
	DECLARE_MACHINE_START(mjyuugi);

	uint32_t screen_update_srmp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_srmp3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	X1_001_SPRITE_GFXBANK_CB_MEMBER(srmp3_gfxbank_callback);

	uint8_t iox_key_matrix_calc(uint8_t p_side);

	void mjyuugi_map(address_map &map);
	void rmgoldyh_io_map(address_map &map);
	void rmgoldyh_map(address_map &map);
	void srmp2_map(address_map &map);
	void srmp3_io_map(address_map &map);
	void srmp3_map(address_map &map);
};

#endif // MAME_INCLUDES_SRMP2_H
