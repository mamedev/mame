// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_F108_H
#define MAME_APPLE_F108_H

#pragma once

#include "iosb.h"

#include "bus/ata/ataintf.h"
#include "machine/ncr53c90.h"
#include "machine/nscsi_bus.h"
#include "machine/z80scc.h"

// ======================> f108_device

/// \brief Device class for Apple F108 system controller ASIC.
///
/// F108 includes a memory controller, NCR 53C96 compatible SCSI,
/// and a single-drive ATA-1 interface.
class f108_device :  public device_t
{
public:
	// construction/destruction
	f108_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// interface routines
	virtual void map(address_map &map) ATTR_COLD;

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_primetimeii_tag(T &&... args) { m_primetimeii.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_rom_tag(T &&... args) { m_rom.set_tag(std::forward<T>(args)...); }
	void set_ram_info(u32 *ram, u32 size);

	auto write_ata_irq() { return m_ata_irq.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	u32 rom_switch_r(offs_t offset);

private:
	void ata_irq_w(int state);
	u32 ata_data_r(offs_t offset, u32 mem_mask);
	void ata_data_w(offs_t offset, u32 data, u32 mem_mask);
	u16 scc_r(offs_t offset);
	void scc_w(offs_t offset, u16 data);
	void scc_irq_w(int state);

	required_device<cpu_device> m_maincpu;
	required_device<primetimeii_device> m_primetimeii;
	required_device<ata_interface_device> m_ata;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c96_device> m_ncr1;
	required_device<z80scc_device> m_scc;
	required_region_ptr<u32> m_rom;

	devcb_write_line m_ata_irq;

	bool m_overlay;
	u32 *m_ram_ptr, *m_rom_ptr;
	u32 m_ram_size, m_rom_size;
};

// device type definition
DECLARE_DEVICE_TYPE(F108, f108_device)

#endif // MAME_APPLE_F108_H
