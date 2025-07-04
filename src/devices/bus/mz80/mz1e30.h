// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_MZ80_MZ1E30_H
#define MAME_BUS_MZ80_MZ1E30_H

#pragma once

#include "mz80_exp.h"
#include "sound/ymopl.h"

class mz1e30_device : public mz80_exp_device
{
public:
	mz1e30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 *m_iplpro_rom = nullptr;

	uint32_t m_rom_index = 0;
	u8 m_hrom_index = 0;
	u8 m_lrom_index = 0;

	u8 rom_r(offs_t offset);
	void rom_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(MZ1E30, mz1e30_device)


#endif // MAME_BUS_MZ80_MZ1E35_H
