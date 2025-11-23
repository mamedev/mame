// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_SSF_H
#define MAME_BUS_MEGADRIVE_CART_SSF_H

#pragma once

#include "machine/nvram.h"

#include "rom.h"
#include "slot.h"

class megadrive_hb_ssf_device : public megadrive_rom_ssf2_device
{
public:
	megadrive_hb_ssf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void time_io_map(address_map &map) override ATTR_COLD;
protected:
	megadrive_hb_ssf_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void time_f0_w(offs_t offset, u16 data, u16 mem_mask);

	bool m_nvram_write_protect;
};

class megadrive_hb_ssf_sram_device : public megadrive_hb_ssf_device
{
public:
	megadrive_hb_ssf_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	u16 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u16 data, u16 mem_mask);
private:
	required_device<nvram_device> m_nvram;
	std::unique_ptr<uint8_t[]> m_nvram_ptr;
};

class megadrive_hb_ssf_ex_device : public megadrive_hb_ssf_device
{
public:
	megadrive_hb_ssf_ex_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::MEDIA | feature::DISK; }

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;
protected:
//  virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void time_f0_w(offs_t offset, u16 data, u16 mem_mask) override;
private:
	struct math_unit_t {
		u16 arg;
		u16 mul;
		u16 div;
	};

	math_unit_t m_math[2];
};



DECLARE_DEVICE_TYPE(MEGADRIVE_HB_SSF,      megadrive_hb_ssf_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_HB_SSF_SRAM, megadrive_hb_ssf_sram_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_HB_SSF_EX,   megadrive_hb_ssf_ex_device)

#endif // MAME_BUS_MEGADRIVE_CART_SSF_H
