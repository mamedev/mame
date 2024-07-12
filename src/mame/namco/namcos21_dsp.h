// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NAMCO_NAMCOS21_DSP_H
#define MAME_NAMCO_NAMCOS21_DSP_H

#pragma once

#include "cpu/tms32025/tms32025.h"
#include "namcos21_3d.h"

#define WINRUN_MAX_POLY_PARAM (1+256*3)

#define PTRAM_SIZE 0x20000

class namcos21_dsp_device : public device_t
{
public:
	namcos21_dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// config
	template <typename T> void set_renderer_tag(T &&tag) { m_renderer.set_tag(std::forward<T>(tag)); }

	void winrun_dspbios_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t winrun_68k_dspcomram_r(offs_t offset);
	void winrun_68k_dspcomram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t winrun_dspcomram_control_r(offs_t offset);
	void winrun_dspcomram_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void pointram_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pointram_data_r();
	void pointram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

	void winrun_dsp_data(address_map &map);
	void winrun_dsp_io(address_map &map);
	void winrun_dsp_program(address_map &map);
private:

	required_device<cpu_device> m_dsp;
	required_shared_ptr<uint16_t> m_winrun_dspbios;
	required_shared_ptr<uint16_t> m_winrun_polydata;
	required_region_ptr<uint16_t> m_ptrom16;

	required_device<namcos21_3d_device> m_renderer;
	std::unique_ptr<uint8_t[]> m_pointram;
	int m_pointram_idx = 0;
	uint16_t m_pointram_control = 0;

	uint16_t m_winrun_dspcomram_control[8]{};
	std::unique_ptr<uint16_t[]> m_winrun_dspcomram;
	uint16_t m_winrun_poly_buf[WINRUN_MAX_POLY_PARAM]{};
	unsigned m_winrun_poly_index = 0;
	uint32_t m_winrun_pointrom_addr = 0;
	int m_winrun_dsp_alive = 0;

	void winrun_flush_poly();

	int m_poly_frame_width = 0;
	int m_poly_frame_height = 0;

	uint16_t winrun_cuskey_r();
	void winrun_cuskey_w(uint16_t data);
	uint16_t winrun_dspcomram_r(offs_t offset);
	void winrun_dspcomram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t winrun_table_r(offs_t offset);
	void winrun_dsp_complete_w(uint16_t data);
	void winrun_dsp_render_w(uint16_t data);
	uint16_t winrun_poly_reset_r();
	void winrun_dsp_pointrom_addr_w(offs_t offset, uint16_t data);
	uint16_t winrun_dsp_pointrom_data_r();

	TIMER_CALLBACK_MEMBER(suspend_callback);
	emu_timer *m_suspend_timer = nullptr;

};

DECLARE_DEVICE_TYPE(NAMCOS21_DSP, namcos21_dsp_device)

#endif // MAME_NAMCO_NAMCOS21_DSP_H
