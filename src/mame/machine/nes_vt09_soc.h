// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_NES_VT09_SOC_H
#define MAME_MACHINE_NES_VT09_SOC_H

#pragma once

#include "nes_vt_soc.h"
#include "cpu/m6502/n2a03.h"
#include "sound/nes_apu_vt.h"
#include "machine/m6502_vtscr.h"
#include "machine/m6502_swap_op_d5_d6.h"
#include "video/ppu2c0x_vt.h"
#include "screen.h"
#include "speaker.h"

class nes_vt09_soc_device : public nes_vt02_vt03_soc_device
{
public:
	nes_vt09_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	auto upper_read_412c_callback() { return m_upper_read_412c_callback.bind(); }
	auto upper_read_412d_callback() { return m_upper_read_412d_callback.bind(); }

	auto upper_write_412c_callback() { return m_upper_write_412c_callback.bind(); }


protected:
	nes_vt09_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);
	void device_start() override;

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_4k_ram_map(address_map& map);

	devcb_write8 m_upper_write_412c_callback;

	devcb_read8 m_upper_read_412c_callback;
	devcb_read8 m_upper_read_412d_callback;
};

class nes_vt09_soc_cy_device : public nes_vt09_soc_device
{
public:
	nes_vt09_soc_cy_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;
	void device_start() override;

	void nes_vt_cy_map(address_map& map);

	uint8_t vt03_41bx_r(offs_t offset);
	void vt03_41bx_w(offs_t offset, uint8_t data);

	uint8_t vt03_413x_r(offs_t offset);
	void vt03_413x_w(offs_t offset, uint8_t data);

	uint8_t vt03_414f_r();

	uint8_t vt03_415c_r();

	void vt03_48ax_w(offs_t offset, uint8_t data);
	uint8_t vt03_48ax_r(offs_t offset);

	uint8_t m_413x[8]; // CY only?
};

class nes_vt09_soc_bt_device : public nes_vt09_soc_device
{
public:
	nes_vt09_soc_bt_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_bt_map(address_map& map);

	void vt03_412c_extbank_w(uint8_t data);
};

class nes_vt369_soc_device : public nes_vt09_soc_device
{
public:
	nes_vt369_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	nes_vt369_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_hh_map(address_map& map);

	uint8_t vthh_414a_r();
	void vtfp_411d_w(uint8_t data);

};

class nes_vt32_soc_device : public nes_vt369_soc_device
{
public:
	nes_vt32_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	nes_vt32_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_fp_map(address_map& map);

	uint8_t vtfp_4119_r();
	void vtfp_411e_w(uint8_t data);
	void vtfp_412c_extbank_w(uint8_t data);
	uint8_t vtfp_412d_r();
	void vtfp_4242_w(uint8_t data);
	void vtfp_4a00_w(uint8_t data);
};

class nes_vt32_soc_pal_device : public nes_vt32_soc_device
{
public:
	nes_vt32_soc_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;
};


class nes_vt3x_soc_dg_device : public nes_vt09_soc_device
{
public:
	nes_vt3x_soc_dg_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	nes_vt3x_soc_dg_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_dg_map(address_map& map);

	void vt03_411c_w(uint8_t data);
};

class nes_vt3x_soc_fa_device : public nes_vt3x_soc_dg_device
{
public:
	nes_vt3x_soc_fa_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_fa_map(address_map& map);

	uint8_t vtfa_412c_r();
	void vtfa_412c_extbank_w(uint8_t data);
	void vtfp_4242_w(uint8_t data);
};


DECLARE_DEVICE_TYPE(NES_VT09_SOC, nes_vt09_soc_device)
DECLARE_DEVICE_TYPE(NES_VT09_SOC_CY, nes_vt09_soc_cy_device)
DECLARE_DEVICE_TYPE(NES_VT09_SOC_BT, nes_vt09_soc_bt_device)
DECLARE_DEVICE_TYPE(NES_VT369_SOC, nes_vt369_soc_device)

DECLARE_DEVICE_TYPE(NES_VT32_SOC, nes_vt32_soc_device)
DECLARE_DEVICE_TYPE(NES_VT32_SOC_PAL, nes_vt32_soc_pal_device)

DECLARE_DEVICE_TYPE(NES_VT3X_SOC_DG, nes_vt3x_soc_dg_device)
DECLARE_DEVICE_TYPE(NES_VT3X_SOC_FA, nes_vt3x_soc_fa_device)

#endif // MAME_MACHINE_NES_VT09_SOC_H
