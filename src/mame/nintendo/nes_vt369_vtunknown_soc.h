// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NINTENDO_NES_VT369_VTUNKNOWN_SOC_H
#define MAME_NINTENDO_NES_VT369_VTUNKNOWN_SOC_H

#pragma once

#include "nes_vt09_soc.h"
#include "cpu/m6502/rp2a03.h"
#include "sound/nes_apu_vt.h"
#include "m6502_swap_op_d5_d6.h"
#include "vt1682_alu.h"
#include "video/ppu2c0x_vt.h"
#include "screen.h"
#include "speaker.h"

class nes_vt369_soc_device : public nes_vt09_soc_device
{
public:
	nes_vt369_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	void nes_vt369_map(address_map &map) ATTR_COLD;

	uint8_t vt369_41bx_r(offs_t offset);
	void vt369_41bx_w(offs_t offset, uint8_t data);

	uint8_t vt369_414f_r();
	uint8_t vt369_415c_r();

	void vt369_48ax_w(offs_t offset, uint8_t data);
	uint8_t vt369_48ax_r(offs_t offset);

	uint8_t vt369_6000_r(offs_t offset);
	void vt369_6000_w(offs_t offset, uint8_t data);

	void vt369_soundcpu_control_w(offs_t offset, uint8_t data);
	void vt369_4112_bank6000_select_w(offs_t offset, uint8_t data);
	void vt369_411c_bank6000_enable_w(offs_t offset, uint8_t data);
	void vt369_relative_w(offs_t offset, uint8_t data);

private:
	void vt369_sound_map(address_map &map) ATTR_COLD;

	required_device<vrt_vt1682_alu_device> m_alu;
	required_device<cpu_device> m_soundcpu;
	uint8_t m_relative[2];
	std::vector<u8> m_6000_ram;
	uint8_t m_bank6000 = 0;
	uint8_t m_bank6000_enable = 0;
};

class nes_vtunknown_soc_bt_device : public nes_vt09_soc_device
{
public:
	nes_vtunknown_soc_bt_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_bt_map(address_map &map) ATTR_COLD;

	void vt03_412c_extbank_w(uint8_t data);
};

class nes_vt369_alt_soc_device : public nes_vt09_soc_device
{
public:
	nes_vt369_alt_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	nes_vt369_alt_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_hh_map(address_map &map) ATTR_COLD;

	uint8_t extra_rom_r();
	uint8_t vthh_414a_r();
	void vtfp_411d_w(uint8_t data);
};

class nes_vt369_alt_swap_d5_d6_soc_device : public nes_vt369_alt_soc_device
{
public:
	nes_vt369_alt_swap_d5_d6_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config& config) override;

private:
	void encryption_4169_w(uint8_t data);
	void nes_vt_hh_swap_map(address_map &map) ATTR_COLD;
};


class nes_vtunknown_soc_dg_device : public nes_vt09_soc_device
{
public:
	nes_vtunknown_soc_dg_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	nes_vtunknown_soc_dg_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_dg_map(address_map &map) ATTR_COLD;

	void vt03_411c_w(uint8_t data);
};

class nes_vtunknown_soc_fa_device : public nes_vtunknown_soc_dg_device
{
public:
	nes_vtunknown_soc_fa_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_fa_map(address_map &map) ATTR_COLD;

	uint8_t vtfa_412c_r();
	void vtfa_412c_extbank_w(uint8_t data);
	void vtfp_4242_w(uint8_t data);
};


DECLARE_DEVICE_TYPE(NES_VTUNKNOWN_SOC_CY, nes_vt369_soc_device)
DECLARE_DEVICE_TYPE(NES_VTUNKNOWN_SOC_BT, nes_vtunknown_soc_bt_device)
DECLARE_DEVICE_TYPE(NES_VT369_SOC, nes_vt369_alt_soc_device)
DECLARE_DEVICE_TYPE(NES_VT369_SOC_SWAP, nes_vt369_alt_swap_d5_d6_soc_device)

DECLARE_DEVICE_TYPE(NES_VTUNKNOWN_SOC_DG, nes_vtunknown_soc_dg_device)
DECLARE_DEVICE_TYPE(NES_VTUNKNOWN_SOC_FA, nes_vtunknown_soc_fa_device)

#endif // MAME_NINTENDO_NES_VT369_VTUNKNOWN_SOC_H
