// license:BSD-3-Clause
// copyright-holders:Yochizo, Takahiro Nogi
#ifndef MAME_INCLUDES_SRMP2_H
#define MAME_INCLUDES_SRMP2_H

#pragma once

#include "sound/msm5205.h"
#include "video/seta001.h"
#include "emupal.h"

class srmp2_state : public driver_device
{
public:
	struct iox_t
	{
		int reset,ff_event,ff_1,protcheck[4],protlatch[4];
		uint8_t data;
		uint8_t mux;
		uint8_t ff;
	};

	srmp2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seta001(*this, "spritegen"),
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
	required_device<seta001_device> m_seta001;
	required_device<msm5205_device> m_msm;
	required_region_ptr<uint8_t> m_adpcm_rom;
	optional_memory_bank m_mainbank;

	int m_color_bank;
	int m_gfx_bank;
	int m_adpcm_bank;
	int m_adpcm_data;
	uint32_t m_adpcm_sptr;
	uint32_t m_adpcm_eptr;
	iox_t m_iox;

	// common
	DECLARE_READ8_MEMBER(vox_status_r);
	DECLARE_READ8_MEMBER(iox_mux_r);
	DECLARE_READ8_MEMBER(iox_status_r);
	DECLARE_WRITE8_MEMBER(iox_command_w);
	DECLARE_WRITE8_MEMBER(iox_data_w);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	// mjuugi
	DECLARE_WRITE16_MEMBER(mjyuugi_flags_w);
	DECLARE_WRITE16_MEMBER(mjyuugi_adpcm_bank_w);
	DECLARE_READ8_MEMBER(mjyuugi_irq2_ack_r);
	DECLARE_READ8_MEMBER(mjyuugi_irq4_ack_r);

	// rmgoldyh
	DECLARE_WRITE8_MEMBER(rmgoldyh_rombank_w);

	// srmp2
	DECLARE_WRITE8_MEMBER(srmp2_irq2_ack_w);
	DECLARE_WRITE8_MEMBER(srmp2_irq4_ack_w);
	DECLARE_WRITE16_MEMBER(srmp2_flags_w);
	DECLARE_WRITE8_MEMBER(adpcm_code_w);

	// srmp3
	DECLARE_WRITE8_MEMBER(srmp3_rombank_w);
	DECLARE_WRITE8_MEMBER(srmp3_flags_w);
	DECLARE_WRITE8_MEMBER(srmp3_irq_ack_w);

	virtual void machine_start() override;
	DECLARE_MACHINE_START(srmp2);
	void srmp2_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(srmp3);
	void srmp3_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(rmgoldyh);
	DECLARE_MACHINE_START(mjyuugi);

	uint32_t screen_update_srmp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_srmp3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mjyuugi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	SETA001_SPRITE_GFXBANK_CB_MEMBER(srmp3_gfxbank_callback);

	uint8_t iox_key_matrix_calc(uint8_t p_side);

	void mjyuugi_map(address_map &map);
	void rmgoldyh_io_map(address_map &map);
	void rmgoldyh_map(address_map &map);
	void srmp2_map(address_map &map);
	void srmp3_io_map(address_map &map);
	void srmp3_map(address_map &map);
};

#endif // MAME_INCLUDES_SRMP2_H
