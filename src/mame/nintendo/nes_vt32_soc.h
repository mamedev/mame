// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NINTENDO_NES_VT32_SOC_H
#define MAME_NINTENDO_NES_VT32_SOC_H

#pragma once

#include "nes_vt09_soc.h"
#include "cpu/m6502/rp2a03.h"
#include "sound/nes_apu_vt.h"
#include "m6502_vtscr.h"
#include "m6502_swap_op_d5_d6.h"
#include "video/ppu2c0x_vt.h"
#include "screen.h"
#include "speaker.h"

class nes_vt32_soc_device : public nes_vt09_soc_device
{
public:
	nes_vt32_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	nes_vt32_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_fp_map(address_map &map) ATTR_COLD;

	uint8_t vtfp_4119_r();
	void vtfp_411e_w(uint8_t data);
	void vtfp_412c_extbank_w(uint8_t data);
	uint8_t vtfp_412d_r();
	void vtfp_4242_w(uint8_t data);
	void vtfp_4a00_w(uint8_t data);
	void vtfp_411d_w(uint8_t data);
	uint8_t vthh_414a_r();
};

class nes_vt32_soc_pal_device : public nes_vt32_soc_device
{
public:
	nes_vt32_soc_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;
};


DECLARE_DEVICE_TYPE(NES_VT32_SOC, nes_vt32_soc_device)
DECLARE_DEVICE_TYPE(NES_VT32_SOC_PAL, nes_vt32_soc_pal_device)

#endif // MAME_NINTENDO_NES_VT32_SOC_H
