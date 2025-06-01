// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_TVGAMES_VT32_SOC_H
#define MAME_TVGAMES_VT32_SOC_H

#pragma once

#include "m6502_swap_op_d5_d6.h"
#include "vt09_soc.h"
#include "rp2a03_vtscr.h"

#include "cpu/m6502/rp2a03.h"
#include "sound/nes_apu_vt.h"
#include "video/ppu2c0x_vt.h"

#include "screen.h"
#include "speaker.h"


class vt32_soc_device : public vt09_soc_device
{
public:
	vt32_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	vt32_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock);

	virtual void device_add_mconfig(machine_config& config) override ATTR_COLD;

	void vt32_soc_map(address_map &map) ATTR_COLD;

	virtual void scrambled_8000_w(u16 offset, u8 data) override;

	u8 read_onespace_bus(offs_t offset);

	u8 vtfp_4119_r();
	void vtfp_411e_encryption_state_w(u8 data);
	void vtfp_412c_extbank_w(u8 data);
	u8 vtfp_412d_r();
	void vtfp_4242_w(u8 data);
	void vtfp_4a00_w(u8 data);
	void vtfp_411d_w(u8 data);
	u8 vthh_414a_r();
	virtual u8 spr_r(offs_t offset) override;
	virtual u8 chr_r(offs_t offset) override;

private:
	u8 vt32_palette_r(offs_t offset);
	void vt32_palette_w(offs_t offset, u8 data);

	int m_ppu_chr_data_scramble;
};

class vt32_soc_pal_device : public vt32_soc_device
{
public:
	vt32_soc_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override ATTR_COLD;

	virtual void do_pal_timings_and_ppu_replacement(machine_config& config) override;
};


DECLARE_DEVICE_TYPE(VT32_SOC, vt32_soc_device)
DECLARE_DEVICE_TYPE(VT32_SOC_PAL, vt32_soc_pal_device)

#endif // MAME_TVGAMES_VT32_SOC_H
