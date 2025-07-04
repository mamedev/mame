// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NINTENDO_NES_VT369_VTUNKNOWN_SOC_H
#define MAME_NINTENDO_NES_VT369_VTUNKNOWN_SOC_H

#pragma once

#include "nes_vt_soc.h"
#include "vt369_adpcm.h"

#include "sound/dac.h"

class vt3xx_soc_base_device : public nes_vt02_vt03_soc_device
{
public:
	vt3xx_soc_base_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	vt3xx_soc_base_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock);

	virtual void device_add_mconfig(machine_config& config) override ATTR_COLD;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	void vt369_map(address_map &map) ATTR_COLD;

	u8 vt369_41bx_r(offs_t offset);
	void vt369_41bx_w(offs_t offset, u8 data);

	u8 vt369_414f_r();

	u8 extra_rom_prot_4150_r();
	u8 extra_rom_prot_4152_r();
	u8 extra_rom_prot_4153_r();
	void extra_rom_prot_4150_w(u8 data);
	void extra_rom_prot_4152_w(u8 data);

	void extra_io_41e6_w(u8 data);

	u8 vt369_415c_r();

	u8 vt369_418a_r();

	u8 vt369_6000_r(offs_t offset);
	void vt369_6000_w(offs_t offset, u8 data);

	void highres_sprite_dma_w(u8 data);

	void vt369_soundcpu_control_w(u8 data);
	void vt369_4112_bank6000_select_w(u8 data);
	void vt369_411c_bank6000_enable_w(u8 data);
	void vt369_411d_w(u8 data);
	void vt369_411e_w(u8 data);
	void vt369_relative_w(offs_t offset, u8 data);

	u8 read_internal(offs_t offset);

private:
	void vt369_sound_map(address_map &map) ATTR_COLD;
	void vt369_sound_external_map(address_map& map) ATTR_COLD;

	u8 sound_read_external(offs_t offset) { return this->space(AS_PROGRAM).read_byte(get_relative() + offset); }


	void vt369_soundcpu_timer_w(offs_t offset, u8 data);
	void vt369_soundcpu_adder_data_address_w(offs_t offset, u8 data);
	void vt369_soundcpu_adder_result_w(offs_t offset, u8 data);
	u8 vt369_soundcpu_adder_result_r(offs_t offset);
	void vt369_soundcpu_adpcm_data_address_w(offs_t offset, u8 data);
	u8 vt369_soundcpu_adpcm_result_r(offs_t offset);
	u8 vt369_soundcpu_adpcm_status_r();
	void vt369_soundcpu_dac_w(offs_t offset, u8 data);
	u8 vt369_soundcpu_vectors_r(offs_t offset);

	u8 vt3xx_palette_r(offs_t offset);
	void vt3xx_palette_w(offs_t offset, u8 data);

	u8 read_onespace_bus_with_relative_offset(offs_t offset);

	virtual void vt_dma_w(u8 data) override;

	u8 alu_r(offs_t offset);
	void alu_w(offs_t offset, u8 data);

	void do_sound_adder();
	void do_sound_adpcm_decode();

	TIMER_CALLBACK_MEMBER(sound_timer_expired);
	void update_timer();

	required_device<cpu_device> m_soundcpu;
	std::vector<u8> m_6000_ram;

	u8 m_bank6000 = 0;
	u8 m_bank6000_enable = 0;

	u16 m_timerperiod;
	u8 m_timercontrol;
	u8 m_alu_params[8];

	emu_timer *m_sound_timer;
	u8 m_sound_adder_addr[2];
	u8 m_sound_adpcm_addr[2];
	u8 m_sound_adder_result[2];
	u8 m_sound_adpcm_result[2];
	u8 m_sound_dac[4];

	optional_region_ptr<u8> m_internal_rom;
	required_shared_ptr<u8> m_soundram;
	required_device<vt369_adpcm_decoder_device> m_vt369adpcm;
	required_device<dac_16bit_r2r_twos_complement_device> m_leftdac;
	required_device<dac_16bit_r2r_twos_complement_device> m_rightdac;
};

class vt369_soc_introm_noswap_device : public vt3xx_soc_base_device
{
public:
	vt369_soc_introm_noswap_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	vt369_soc_introm_noswap_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config& config) override ATTR_COLD;

	void vt369_introm_map(address_map &map) ATTR_COLD;

	u8 vthh_414a_r();
	void encryption_4169_w(u8 data);

	bool m_encryption_allowed;
};

class vt369_soc_introm_swap_device : public vt369_soc_introm_noswap_device
{
public:
	vt369_soc_introm_swap_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	virtual void device_start() override;
};

class vt369_soc_introm_altswap_device : public vt369_soc_introm_noswap_device
{
public:
	vt369_soc_introm_altswap_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	virtual void device_start() override;
};

class vt369_soc_introm_vibesswap_device : public vt369_soc_introm_noswap_device
{
public:
	vt369_soc_introm_vibesswap_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	vt369_soc_introm_vibesswap_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock);

	virtual void device_add_mconfig(machine_config& config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void vibes_411c_w(u8 data);

	void nes_vt_vibes_map(address_map &map) ATTR_COLD;
};

class vt369_soc_introm_gbox2020_device : public vt369_soc_introm_vibesswap_device
{
public:
	vt369_soc_introm_gbox2020_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class vt3xx_soc_unk_dg_device : public vt3xx_soc_base_device
{
public:
	vt3xx_soc_unk_dg_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

protected:
	vt3xx_soc_unk_dg_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock);

	virtual void device_add_mconfig(machine_config& config) override;

	void nes_vt_dg_map(address_map &map) ATTR_COLD;

	void vt03_411c_w(u8 data);
};


DECLARE_DEVICE_TYPE(VT3XX_SOC, vt3xx_soc_base_device)

DECLARE_DEVICE_TYPE(VT369_SOC_INTROM_NOSWAP, vt369_soc_introm_noswap_device)
DECLARE_DEVICE_TYPE(VT369_SOC_INTROM_SWAP,   vt369_soc_introm_swap_device)
DECLARE_DEVICE_TYPE(VT369_SOC_INTROM_ALTSWAP,   vt369_soc_introm_altswap_device)
DECLARE_DEVICE_TYPE(VT369_SOC_INTROM_VIBESSWAP,   vt369_soc_introm_vibesswap_device)
DECLARE_DEVICE_TYPE(VT369_SOC_INTROM_GBOX2020,   vt369_soc_introm_gbox2020_device)

DECLARE_DEVICE_TYPE(VT3XX_SOC_UNK_DG, vt3xx_soc_unk_dg_device)

#endif // MAME_NINTENDO_NES_VT369_VTUNKNOWN_SOC_H
