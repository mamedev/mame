// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Saitek OSA Module: Kasparov Sparc

***************************************************************************/

#ifndef MAME_BUS_SAITEKOSA_SPARC_H
#define MAME_BUS_SAITEKOSA_SPARC_H

#pragma once

#include "expansion.h"

#include "cpu/sparc/sparc.h"

DECLARE_DEVICE_TYPE(OSA_SPARC, saitekosa_sparc_device)


class saitekosa_sparc_device : public device_t, public device_saitekosa_expansion_interface
{
public:
	// construction/destruction
	saitekosa_sparc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// from host
	virtual u8 data_r() override;
	virtual void nmi_w(int state) override;
	virtual void ack_w(int state) override;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<mb86930_device> m_maincpu;
	required_region_ptr<u32> m_rom;
	required_shared_ptr<u32> m_ram;

	void debugger_map(address_map &map);

	u32 rom_r(offs_t offset, u32 mem_mask) { return m_rom[offset & m_rom_mask]; }
	u32 ram_r(offs_t offset, u32 mem_mask) { return m_ram[offset & m_ram_mask]; }
	void ram_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_ram[offset & m_ram_mask]); }
	u32 host_io_r(offs_t offset, u32 mem_mask = ~0U);
	void host_io_w(offs_t offset, u32 data, u32 mem_mask = ~0U);

	void set_ram_mask(u8 n) { m_ram_mask = ((1 << n) / 4) - 1; }

	u32 m_data_out = 0;
	u32 m_rom_mask = 0;
	u32 m_ram_mask = 0;
	bool m_installed = false;
};

#endif // MAME_BUS_SAITEKOSA_SPARC_H
