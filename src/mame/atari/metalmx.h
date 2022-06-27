// license:BSD-3-Clause
// copyright-holders:Philip Bennett
#ifndef MAME_INCLUDES_METALMX_H
#define MAME_INCLUDES_METALMX_H

#pragma once

#include "cage.h"

#include "cpu/adsp2100/adsp2100.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/dsp32/dsp32.h"

class metalmx_state : public driver_device
{
public:
	metalmx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gsp(*this, "gsp"),
		m_adsp(*this, "adsp"),
		m_dsp32c(*this, "dsp32c_%u", 1U),
		m_cage(*this, "cage"),
		m_adsp_internal_program_ram(*this, "adsp_intprog"),
		m_gsp_dram(*this, "gsp_dram"),
		m_gsp_vram(*this, "gsp_vram")
	{ }

	void init_metalmx();
	void metalmx(machine_config &config);

private:
	uint32_t unk_r();
	uint32_t watchdog_r();
	void shifter_w(uint32_t data);
	void motor_w(uint32_t data);
	void reset_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sound_data_r(offs_t offset, uint32_t mem_mask = ~0);
	void sound_data_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Chip> void dsp32c_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Chip> uint32_t dsp32c_r(offs_t offset, uint32_t mem_mask = ~0);
	void host_gsp_w(offs_t offset, uint32_t data);
	uint32_t host_gsp_r(offs_t offset);
	uint32_t host_dram_r(offs_t offset);
	void host_dram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t host_vram_r(offs_t offset);
	void host_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void timer_w(offs_t offset, uint32_t data);
	void cage_irq_callback(uint8_t data);
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_metalmx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void adsp_data_map(address_map &map);
	void adsp_program_map(address_map &map);
	void dsp32c_1_map(address_map &map);
	void dsp32c_2_map(address_map &map);
	void gsp_map(address_map &map);
	void main_map(address_map &map);

	required_device<m68ec020_device> m_maincpu;
	required_device<tms34020_device> m_gsp;
	required_device<adsp2105_device> m_adsp;
	required_device_array<dsp32c_device, 2> m_dsp32c;
	required_device<atari_cage_device> m_cage;

	required_shared_ptr<uint32_t> m_adsp_internal_program_ram;
	required_shared_ptr<uint32_t> m_gsp_dram;
	required_shared_ptr<uint32_t> m_gsp_vram;
};

#endif // MAME_INCLUDES_METALMX_H
