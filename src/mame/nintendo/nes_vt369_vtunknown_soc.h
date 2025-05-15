// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NINTENDO_NES_VT369_VTUNKNOWN_SOC_H
#define MAME_NINTENDO_NES_VT369_VTUNKNOWN_SOC_H

#pragma once

#include "nes_vt09_soc.h"
#include "cpu/m6502/rp2a03.h"
#include "cpu/m6502/vt3xx_spu.h"
#include "sound/nes_apu_vt.h"
#include "m6502_swap_op_d5_d6.h"
#include "vt1682_alu.h"
#include "video/ppu2c0x_vt.h"
#include "screen.h"
#include "speaker.h"

class vt3xx_soc_base_device : public nes_vt09_soc_device
{
public:
	vt3xx_soc_base_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	vt3xx_soc_base_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config& config) override;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	void vt369_map(address_map &map) ATTR_COLD;

	uint8_t vt369_41bx_r(offs_t offset);
	void vt369_41bx_w(offs_t offset, uint8_t data);

	uint8_t vt369_414f_r();

	uint8_t extra_rom_prot_4150_r();
	uint8_t extra_rom_prot_4152_r();
	uint8_t extra_rom_prot_4153_r();
	void extra_rom_prot_4150_w(uint8_t data);
	void extra_rom_prot_4152_w(uint8_t data);

	uint8_t vt369_415c_r();

	uint8_t vt369_418a_r();

	uint8_t vt369_6000_r(offs_t offset);
	void vt369_6000_w(offs_t offset, uint8_t data);

	void vt369_soundcpu_control_w(offs_t offset, uint8_t data);
	void vt369_4112_bank6000_select_w(offs_t offset, uint8_t data);
	void vt369_411c_bank6000_enable_w(offs_t offset, uint8_t data);
	void vt369_relative_w(offs_t offset, uint8_t data);

	uint8_t read_internal(offs_t offset);

private:
	void vt369_sound_map(address_map &map) ATTR_COLD;
	void vt369_sound_external_map(address_map& map) ATTR_COLD;

	uint8_t sound_read_external(offs_t offset) { return space(AS_PROGRAM).read_byte(offset); }


	void vt369_soundcpu_timer_w(offs_t offset, uint8_t data);
	void vt369_soundcpu_adder_data_address_w(offs_t offset, uint8_t data);
	uint8_t vt369_soundcpu_adder_result_r(offs_t offset);
	void vt369_soundcpu_mult_data_address_w(offs_t offset, uint8_t data);
	uint8_t vt369_soundcpu_mult_result_r(offs_t offset);
	uint8_t vt369_soundcpu_mult_status_r();
	void vt369_soundcpu_dac_w(offs_t offset, uint8_t data);
	uint8_t vt369_soundcpu_vectors_r(offs_t offset);

	uint8_t vt3xx_palette_r(offs_t offset);
	void vt3xx_palette_w(offs_t offset, uint8_t data);

	virtual void vt_dma_w(uint8_t data) override;

	TIMER_CALLBACK_MEMBER(sound_timer_expired);
	void update_timer();

	required_device<vrt_vt1682_alu_device> m_alu;
	required_device<cpu_device> m_soundcpu;
	uint8_t m_relative[2];
	std::vector<u8> m_6000_ram;
	uint8_t m_bank6000 = 0;
	uint8_t m_bank6000_enable = 0;

	uint16_t m_timerperiod;
	uint8_t m_timercontrol;

	emu_timer *m_sound_timer;

	optional_region_ptr<uint8_t> m_internal_rom;
	required_shared_ptr<uint8_t> m_soundram;
};

class vt3xx_soc_unk_bt_device : public vt3xx_soc_base_device
{
public:
	vt3xx_soc_unk_bt_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_bt_map(address_map &map) ATTR_COLD;

	void vt03_412c_extbank_w(uint8_t data);
};

class vt369_soc_introm_noswap_device : public vt3xx_soc_base_device
{
public:
	vt369_soc_introm_noswap_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	vt369_soc_introm_noswap_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config& config) override;

	void vt369_introm_map(address_map &map) ATTR_COLD;

	uint8_t vthh_414a_r();
	void vtfp_411d_w(uint8_t data);
	void encryption_4169_w(uint8_t data);

	bool m_encryption_allowed;
};

class vt369_soc_introm_swap_device : public vt369_soc_introm_noswap_device
{
public:
	vt369_soc_introm_swap_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_start() override;
};

class vt369_soc_introm_altswap_device : public vt369_soc_introm_noswap_device
{
public:
	vt369_soc_introm_altswap_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_start() override;
};

class vt369_soc_introm_vibesswap_device : public vt369_soc_introm_noswap_device
{
public:
	vt369_soc_introm_vibesswap_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_start() override;
};

class vt3xx_soc_unk_dg_device : public vt3xx_soc_base_device
{
public:
	vt3xx_soc_unk_dg_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	vt3xx_soc_unk_dg_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_dg_map(address_map &map) ATTR_COLD;

	void vt03_411c_w(uint8_t data);
};

class vt3xx_soc_unk_fa_device : public vt3xx_soc_unk_dg_device
{
public:
	vt3xx_soc_unk_fa_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_fa_map(address_map &map) ATTR_COLD;

	uint8_t vtfa_412c_r();
	void vtfa_412c_extbank_w(uint8_t data);
	void vtfp_4242_w(uint8_t data);
};


DECLARE_DEVICE_TYPE(VT3XX_SOC, vt3xx_soc_base_device)

DECLARE_DEVICE_TYPE(VT369_SOC_INTROM_NOSWAP, vt369_soc_introm_noswap_device)
DECLARE_DEVICE_TYPE(VT369_SOC_INTROM_SWAP,   vt369_soc_introm_swap_device)
DECLARE_DEVICE_TYPE(VT369_SOC_INTROM_ALTSWAP,   vt369_soc_introm_altswap_device)
DECLARE_DEVICE_TYPE(VT369_SOC_INTROM_VIBESSWAP,   vt369_soc_introm_vibesswap_device)

DECLARE_DEVICE_TYPE(VT3XX_SOC_UNK_BT, vt3xx_soc_unk_bt_device)

DECLARE_DEVICE_TYPE(VT3XX_SOC_UNK_DG, vt3xx_soc_unk_dg_device)
DECLARE_DEVICE_TYPE(VT3XX_SOC_UNK_FA, vt3xx_soc_unk_fa_device)

#endif // MAME_NINTENDO_NES_VT369_VTUNKNOWN_SOC_H
