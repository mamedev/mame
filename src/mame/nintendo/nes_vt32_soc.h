// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NINTENDO_NES_VT32_SOC_H
#define MAME_NINTENDO_NES_VT32_SOC_H

#pragma once

#include "m6502_swap_op_d5_d6.h"
#include "nes_vt09_soc.h"
#include "rp2a03_vtscr.h"

#include "cpu/m6502/rp2a03.h"
#include "sound/nes_apu_vt.h"
#include "video/ppu2c0x_vt.h"

#include "screen.h"
#include "speaker.h"


class nes_vt32_soc_device : public nes_vt09_soc_device
{
public:
	nes_vt32_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	nes_vt32_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock);

	virtual void device_add_mconfig(machine_config& config) override ATTR_COLD;

	void nes_vt32_soc_map(address_map &map) ATTR_COLD;

	virtual void update_banks() override;

	virtual void scrambled_8000_w(u16 offset, u8 data) override;

	u8 read_onespace_bus(offs_t offset);

	u8 vtfp_4119_r();
	void vtfp_411e_encryption_state_w(u8 data);
	u8 vtfp_412c_r();
	void vtfp_412c_extbank_w(u8 data);
	u8 vtfp_412d_r();
	void vtfp_4242_w(u8 data);
	void vtfp_4a00_w(u8 data);
	void vtfp_411d_w(u8 data);
	u8 vthh_414a_r();
	u8 vt32_4132_r();
	void vt32_4132_w(u8 data);
	u8 vt32_4134_r();
	void vt32_4134_w(u8 data);
	virtual u8 spr_r(offs_t offset) override;
	virtual u8 chr_r(offs_t offset) override;

private:
	u8 vt32_palette_r(offs_t offset);
	void vt32_palette_w(offs_t offset, u8 data);

	int m_ppu_chr_data_scramble;
	u8 m_mmc1_shift_reg;
	u8 m_mmc1_control;
	u8 m_mmc1_prg_bank;

	u8 m_4132;
	u8 m_4134;
};

class nes_vt32_soc_pal_device : public nes_vt32_soc_device
{
public:
	nes_vt32_soc_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override ATTR_COLD;

	virtual void do_pal_timings_and_ppu_replacement(machine_config& config) override;
};

class nes_vt32_soc_unk_device : public nes_vt32_soc_device
{
public:
	nes_vt32_soc_unk_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(NES_VT32_SOC, nes_vt32_soc_device)
DECLARE_DEVICE_TYPE(NES_VT32_SOC_PAL, nes_vt32_soc_pal_device)
DECLARE_DEVICE_TYPE(NES_VT32_SOC_UNK, nes_vt32_soc_unk_device)

#endif // MAME_NINTENDO_NES_VT32_SOC_H
