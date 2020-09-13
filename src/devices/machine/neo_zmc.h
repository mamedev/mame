// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

    SNK NEO-ZMC(2) Z80 Memory controller emulation

***************************************************************************/

#ifndef MAME_MACHINE_NEO_ZMC_H
#define MAME_MACHINE_NEO_ZMC_H

#pragma once

#include "machine/alpha_8921.h"

// ======================> neo_zmc_core_device

class neo_zmc_core_device : public device_t, public device_memory_interface
{
public:
	// configuration
	template <typename T> void set_default_rom_tag(T &&tag) { m_default_rom.set_tag(std::forward<T>(tag)); }

	// ROM handlers
	virtual u8 rom_r(offs_t offset) = 0;
	template<unsigned Slot> u8 banked_rom_r(offs_t offset) { return rom_r(m_bank[Slot] | (offset & ((1 << (11 + Slot)) - 1))); }

	// Bank change handlers
	void bank_w(offs_t offset, u8 data = 0); // no data bus pin

	void map(address_map &map);

protected:
	// construction/destruction
	neo_zmc_core_device(const machine_config &mconfig, device_type &type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;

	optional_region_ptr<u8> m_default_rom; // default mapped ROM

	u32 m_bank[4] = {0,0,0,0};
};

// ======================> neo_zmc_device

class neo_zmc_device : public neo_zmc_core_device
{
public:
	// construction/destruction
	neo_zmc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0); // No any clock pin in chip

	// ROM handlers
	virtual u8 rom_r(offs_t offset) override { return m_cache.read_byte(offset); }

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_data_config;
	memory_access<19, 0, 0, ENDIANNESS_LITTLE>::cache m_cache; // or 22?
};

// ======================> neo_zmc2_device

class neo_zmc2_device : public neo_zmc_core_device
{
public:
	// construction/destruction
	neo_zmc2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// ROM handlers
	virtual u8 rom_r(offs_t offset) override { return m_cache.read_byte(offset); }

	// (integrated)PRO-CT0 part
	alpha_8921_device &pro_ct0() { return *m_pro_ct0; }

	// inputs
	DECLARE_WRITE_LINE_MEMBER(clk_w) { m_pro_ct0->clk_w(state); }
	DECLARE_WRITE_LINE_MEMBER(load_w) { m_pro_ct0->load_w(state); }
	DECLARE_WRITE_LINE_MEMBER(even_w) { m_pro_ct0->even_w(state); }
	DECLARE_WRITE_LINE_MEMBER(h_w) { m_pro_ct0->h_w(state); }
	void c_w(u32 data) { m_pro_ct0->c_w(data); }

	// outputs
	u8 gad_r() { return m_pro_ct0->gad_r(); }
	u8 gbd_r() { return m_pro_ct0->gbd_r(); }
	DECLARE_READ_LINE_MEMBER(dota_r) { return m_pro_ct0->dota_r(); }
	DECLARE_READ_LINE_MEMBER(dotb_r) { return m_pro_ct0->dotb_r(); }

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_data_config;
	memory_access<22, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;

	required_device<alpha_8921_device> m_pro_ct0; // integrated
};


// device type definition
DECLARE_DEVICE_TYPE(NEO_ZMC,  neo_zmc_device)
DECLARE_DEVICE_TYPE(NEO_ZMC2, neo_zmc2_device)

#endif // MAME_MACHINE_NEO_ZMC_H
