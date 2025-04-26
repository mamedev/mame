// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NINTENDO_NES_VT09_SOC_H
#define MAME_NINTENDO_NES_VT09_SOC_H

#pragma once

#include "nes_vt_soc.h"
#include "cpu/m6502/rp2a03.h"
#include "sound/nes_apu_vt.h"
#include "m6502_swap_op_d5_d6.h"
#include "video/ppu2c0x_vt.h"
#include "screen.h"
#include "speaker.h"

class nes_vt09_soc_device : public nes_vt02_vt03_soc_device
{
public:
	nes_vt09_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	// are these even part of vt09, or should they be moved out of here rather than this being treated as a base class for them?
	auto upper_read_412c_callback() { return m_upper_read_412c_callback.bind(); }
	auto upper_read_412d_callback() { return m_upper_read_412d_callback.bind(); }

	auto upper_write_412c_callback() { return m_upper_write_412c_callback.bind(); }


protected:
	nes_vt09_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_4k_ram_map(address_map &map) ATTR_COLD;

	// are these even part of vt09, or should they be moved out of here rather than this being treated as a base class for them?
	devcb_write8 m_upper_write_412c_callback;

	devcb_read8 m_upper_read_412c_callback;
	devcb_read8 m_upper_read_412d_callback;
};


DECLARE_DEVICE_TYPE(NES_VT09_SOC, nes_vt09_soc_device)


#endif // MAME_NINTENDO_NES_VT09_SOC_H
