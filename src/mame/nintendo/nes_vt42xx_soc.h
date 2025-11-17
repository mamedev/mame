// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NINTENDO_NES_VT42XX_SOC_H
#define MAME_NINTENDO_NES_VT42XX_SOC_H

#pragma once

#include "nes_vt09_soc.h"


class nes_vt42xx_soc_device : public nes_vt09_soc_device
{
public:
	nes_vt42xx_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	nes_vt42xx_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock);

	virtual void device_add_mconfig(machine_config& config) override ATTR_COLD;

	void nes_vt42xx_soc_map(address_map &map) ATTR_COLD;

	u8 read_onespace_bus(offs_t offset);

	u8 vtfp_412c_r();
	void vtfp_412c_extbank_w(u8 data);
	u8 vtfp_412d_r();

	void vt_420x_w(offs_t offset, u8 data);
	void vt_422x_w(offs_t offset, u8 data);
	void vt_423x_w(offs_t offset, u8 data);
	void vt_4233_w(u8 data);

private:
	u8 m_420x[15];
	u8 m_422x[8];
	u8 m_423x[2];
};

class nes_vt42xx_soc_pal_device : public nes_vt42xx_soc_device
{
public:
	nes_vt42xx_soc_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override ATTR_COLD;

	virtual void do_pal_timings_and_ppu_replacement(machine_config& config) override;
};


DECLARE_DEVICE_TYPE(NES_VT42XX_SOC, nes_vt42xx_soc_device)
DECLARE_DEVICE_TYPE(NES_VT42XX_SOC_PAL, nes_vt42xx_soc_pal_device)

#endif // MAME_NINTENDO_NES_VT42XX_SOC_H
