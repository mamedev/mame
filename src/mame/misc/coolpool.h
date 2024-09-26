// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nicola Salmoria
#ifndef MAME_MISC_COOLPOOL_H
#define MAME_MISC_COOLPOOL_H

#pragma once

#include "cpu/tms34010/tms34010.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "video/tlc34076.h"

#include "emupal.h"

class coolpool_base_state : public driver_device
{
protected:
	coolpool_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsp(*this, "dsp")
		, m_main2dsp(*this, "main2dsp")
		, m_dsp2main(*this, "dsp2main")
		, m_nvram_timer(*this, "nvram_timer")
		, m_vram_base(*this, "vram_base")
		, m_nvram(*this, "nvram")
		, m_dsp_rom(*this, "dspdata")
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	static constexpr unsigned NVRAM_UNLOCK_SEQ_LEN = 10;

	required_device<tms34010_device> m_maincpu;
	required_device<cpu_device> m_dsp;

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
	uint8_t m_same_cmd_count = 0U;

	void nvram_thrash_w(offs_t offset, uint16_t data);
	void nvram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nvram_thrash_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t dsp_rom_r();
	void dsp_romaddr_w(offs_t offset, uint16_t data);

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);

	TIMER_DEVICE_CALLBACK_MEMBER(nvram_write_timeout);
};

class amerdart_state : public coolpool_base_state
{
public:
	amerdart_state(const machine_config &mconfig, device_type type, const char *tag)
		: coolpool_base_state(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_in_xaxis(*this, "XAXIS%u", 1U)
		, m_in_yaxis(*this, "YAXIS%u", 1U)
	{ }

	void amerdart(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<palette_device> m_palette;

	required_ioport_array<2> m_in_xaxis;
	required_ioport_array<2> m_in_yaxis;

	bool m_old_cmd = false;

	void misc_w(uint16_t data);
	int dsp_bio_line_r();
	uint16_t amerdart_trackball_r(offs_t offset);

	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline);

	TIMER_DEVICE_CALLBACK_MEMBER(amerdart_audio_int_gen);

	int amerdart_trackball_direction(int num, int data);

	void dsp_io_map(address_map &map) ATTR_COLD;
	void dsp_pgm_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};

class _9ballsht_state : public coolpool_base_state
{
public:
	_9ballsht_state(const machine_config &mconfig, device_type type, const char *tag)
		: coolpool_base_state(mconfig, type, tag)
		, m_tlc34076(*this, "tlc34076")
	{ }

	void _9ballsht(machine_config &config);

	void init_9ballsht();

protected:
	required_device<tlc34076_device> m_tlc34076;

	void misc_w(uint16_t data);
	uint16_t dsp_bio_line_r();
	uint16_t dsp_hold_line_r();

	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline);

	void dsp_io_base_map(address_map &map) ATTR_COLD;
	void dsp_pgm_map(address_map &map) ATTR_COLD;

private:
	void nballsht_dsp_io_map(address_map &map) ATTR_COLD;
	void nballsht_map(address_map &map) ATTR_COLD;
};

class coolpool_state : public _9ballsht_state
{
public:
	coolpool_state(const machine_config &mconfig, device_type type, const char *tag)
		: _9ballsht_state(mconfig, type, tag)
		, m_in1(*this, "IN1")
		, m_xaxis(*this, "XAXIS")
		, m_yaxis(*this, "YAXIS")
	{ }

	void coolpool(machine_config &config);

private:
	required_ioport m_in1;
	required_ioport m_xaxis;
	required_ioport m_yaxis;

	uint16_t coolpool_input_r(offs_t offset);

	void coolpool_dsp_io_map(address_map &map) ATTR_COLD;
	void coolpool_map(address_map &map) ATTR_COLD;
};

#endif // MAME_MISC_COOLPOOL_H
