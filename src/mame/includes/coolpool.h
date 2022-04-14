// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nicola Salmoria
#ifndef MAME_INCLUDES_COOLPOOL_H
#define MAME_INCLUDES_COOLPOOL_H

#pragma once

#include "cpu/tms34010/tms34010.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "video/tlc34076.h"

class coolpool_state : public driver_device
{
public:
	coolpool_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsp(*this, "dsp")
		, m_tlc34076(*this, "tlc34076")
		, m_main2dsp(*this, "main2dsp")
		, m_dsp2main(*this, "dsp2main")
		, m_nvram_timer(*this, "nvram_timer")
		, m_vram_base(*this, "vram_base")
		, m_nvram(*this, "nvram")
		, m_dsp_rom(*this, "dspdata")
	{ }

	void _9ballsht(machine_config &config);
	void coolpool(machine_config &config);
	void amerdart(machine_config &config);

	void init_amerdart();
	void init_9ballsht();

protected:
	virtual void machine_start() override;

private:
	static constexpr unsigned NVRAM_UNLOCK_SEQ_LEN = 10;

	required_device<tms34010_device> m_maincpu;
	required_device<cpu_device> m_dsp;
	optional_device<tlc34076_device> m_tlc34076;

	required_device<generic_latch_16_device> m_main2dsp;
	required_device<generic_latch_16_device> m_dsp2main;

	required_device<timer_device> m_nvram_timer;

	required_shared_ptr<uint16_t> m_vram_base;
	required_shared_ptr<uint16_t> m_nvram;
	required_region_ptr<uint8_t> m_dsp_rom;

	int m_iop_romaddr = 0;

	uint8_t m_newx[3]{};
	uint8_t m_newy[3]{};
	uint8_t m_oldx[3]{};
	uint8_t m_oldy[3]{};
	int m_dx[3]{};
	int m_dy[3]{};

	uint16_t m_result = 0U;
	uint16_t m_lastresult = 0U;

	uint16_t m_nvram_write_seq[NVRAM_UNLOCK_SEQ_LEN]{};
	uint8_t m_nvram_write_enable = 0U;
	bool m_old_cmd = false;
	uint8_t m_same_cmd_count = 0U;

	void nvram_thrash_w(offs_t offset, uint16_t data);
	void nvram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nvram_thrash_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void amerdart_misc_w(uint16_t data);
	DECLARE_READ_LINE_MEMBER(amerdart_dsp_bio_line_r);
	uint16_t amerdart_trackball_r(offs_t offset);
	void coolpool_misc_w(uint16_t data);
	uint16_t dsp_bio_line_r();
	uint16_t dsp_hold_line_r();
	uint16_t dsp_rom_r();
	void dsp_romaddr_w(offs_t offset, uint16_t data);
	uint16_t coolpool_input_r(offs_t offset);

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(amerdart_scanline);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(coolpool_scanline);

	DECLARE_MACHINE_RESET(amerdart);
	DECLARE_MACHINE_RESET(coolpool);

	TIMER_DEVICE_CALLBACK_MEMBER(nvram_write_timeout);
	TIMER_DEVICE_CALLBACK_MEMBER(amerdart_audio_int_gen);

	int amerdart_trackball_direction(int num, int data);

	void amerdart_dsp_io_map(address_map &map);
	void amerdart_dsp_pgm_map(address_map &map);
	void amerdart_map(address_map &map);
	void coolpool_dsp_io_map(address_map &map);
	void coolpool_dsp_io_base_map(address_map &map);
	void coolpool_dsp_pgm_map(address_map &map);
	void coolpool_map(address_map &map);
	void nballsht_dsp_io_map(address_map &map);
	void nballsht_map(address_map &map);
};

#endif // MAME_INCLUDES_COOLPOOL_H
