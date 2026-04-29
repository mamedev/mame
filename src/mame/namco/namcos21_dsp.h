// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, David Haywood
#ifndef MAME_NAMCO_NAMCOS21_DSP_H
#define MAME_NAMCO_NAMCOS21_DSP_H

#pragma once

#include "namcos21_3d.h"

#include "cpu/tms320c2x/tms320c2x.h"


class namcos21_dsp_device : public device_t
{
public:
	static constexpr unsigned PTRAM_SIZE = 0x20000;
	static constexpr unsigned WINRUN_MAX_POLY_PARAM = 1+256*3;

	namcos21_dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// config
	template <typename T> void set_renderer_tag(T &&tag) { m_renderer.set_tag(std::forward<T>(tag)); }

	void winrun_dspbios_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 winrun_68k_dspcomram_r(offs_t offset);
	void winrun_68k_dspcomram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 winrun_dspcomram_control_r(offs_t offset);
	void winrun_dspcomram_control_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void pointram_control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 pointram_data_r();
	void pointram_data_w(offs_t offset, u16 data, u16 mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void winrun_dsp_data(address_map &map) ATTR_COLD;
	void winrun_dsp_io(address_map &map) ATTR_COLD;
	void winrun_dsp_program(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_dsp;
	required_shared_ptr<u16> m_winrun_dspbios;
	required_shared_ptr<u16> m_winrun_polydata;
	required_region_ptr<u16> m_ptrom16;

	required_device<namcos21_3d_device> m_renderer;
	std::unique_ptr<u8[]> m_pointram;
	int m_pointram_idx;
	u16 m_pointram_control;

	u16 m_winrun_dspcomram_control[8];
	std::unique_ptr<u16[]> m_winrun_dspcomram;
	u16 m_winrun_poly_buf[WINRUN_MAX_POLY_PARAM]{};
	int m_winrun_poly_index;
	u32 m_winrun_pointrom_addr;
	int m_winrun_dsp_alive;

	void winrun_flush_poly();

	u16 winrun_cuskey_r();
	void winrun_cuskey_w(u16 data);
	u16 winrun_dspcomram_r(offs_t offset);
	void winrun_dspcomram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 winrun_table_r(offs_t offset);
	void winrun_dsp_complete_w(u16 data);
	void winrun_dsp_render_w(u16 data);
	u16 winrun_poly_reset_r();
	void winrun_dsp_pointrom_addr_w(offs_t offset, u16 data);
	u16 winrun_dsp_pointrom_data_r();

	TIMER_CALLBACK_MEMBER(suspend_callback);
	emu_timer *m_suspend_timer;
};

DECLARE_DEVICE_TYPE(NAMCOS21_DSP, namcos21_dsp_device)

#endif // MAME_NAMCO_NAMCOS21_DSP_H
