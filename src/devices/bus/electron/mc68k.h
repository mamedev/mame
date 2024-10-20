// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Electron 68000 Expansion

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_MC68K_M
#define MAME_BUS_ELECTRON_MC68K_M

#pragma once

#include "exp.h"
#include "cpu/m68000/m68000.h"
#include "machine/6821pia.h"
#include "machine/ram.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_mc68k_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_mc68k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual u8 expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, u8 data) override;

private:
	required_device<m68000_base_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device_array<pia6821_device, 2> m_pia;
	required_memory_region m_boot_rom;
	required_memory_region m_exp_rom;

	memory_passthrough_handler m_rom_shadow_tap;

	void mem_map(address_map &map) ATTR_COLD;

	u8 m_romsel;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_MC68K, electron_mc68k_device)


#endif // MAME_BUS_ELECTRON_MC68K_M
