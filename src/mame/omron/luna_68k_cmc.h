// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, O. Galibert
#ifndef MAME_OMRON_LUNA_68K_CMC_H
#define MAME_OMRON_LUNA_68K_CMC_H

#pragma once

#include "cpu/m68000/m68020.h"
#include "machine/am79c90.h"
#include "machine/am9513.h"
#include "machine/tms9914.h"
#include "machine/z80scc.h"

class luna_68k_cmc_device : public device_t {
public:
	luna_68k_cmc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock=0);

	void vme_map(address_map &map);

public:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<m68020_device> m_cpu;
	required_device<am7990_device> m_eth;
	required_device<tms9914_device> m_gpib;
	required_device<am9513_device> m_stc;
	required_device<z80scc_device> m_scc1, m_scc2;
	memory_view m_boot;

	void cpu_map(address_map &map) ATTR_COLD;
	void boot_w(offs_t offset, u32 data);
};

DECLARE_DEVICE_TYPE(LUNA_68K_CMC, luna_68k_cmc_device)

#endif
