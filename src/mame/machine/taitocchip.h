// license:BSD-3-Clause
// copyright-holders:David Haywood, Jonathan Gevaryahu

#ifndef MAME_MACHINE_TAITOCCHIP_H
#define MAME_MACHINE_TAITOCCHIP_H

#pragma once

#include "cpu/upd7810/upd7811.h"
#include "machine/bankdev.h"

DECLARE_DEVICE_TYPE(TAITO_CCHIP, taito_cchip_device)

#define MCFG_TAITO_CCHIP_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, TAITO_CCHIP, _clock)


class taito_cchip_device :  public device_t
{
public:
	// construction/destruction
	taito_cchip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// can be accessed externally
	DECLARE_READ8_MEMBER(asic_r);
	DECLARE_WRITE8_MEMBER(asic_w);
	DECLARE_READ8_MEMBER(mem_r);
	DECLARE_WRITE8_MEMBER(mem_w);

	void cchip_map(address_map &map);
	void cchip_ram_bank(address_map &map);
protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_upd7811;
	required_device<address_map_bank_device> m_upd4464_bank;
	required_shared_ptr<uint8_t> m_upd4464;
	uint8_t m_asic_ram[4];
};

#endif // MAME_MACHINE_CCHIP_DEV_H
