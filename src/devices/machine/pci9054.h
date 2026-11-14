// license:BSD-3-Clause
// copyright-holders:MrBaristaNZ
/**************************************************************************************************

    PLX PCI9054 PCI I/O accelerator

**************************************************************************************************/

#ifndef MAME_MACHINE_PCI9054_H
#define MAME_MACHINE_PCI9054_H

#pragma once

#include "machine/pci.h"

class pci9054_device : public pci_device
{
public:
	pci9054_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// local bus side: the configuration registers, and the direct-master windows (offsets relative to
	// the window base, the board decodes the local addresses)
	u32 local_r(offs_t offset, u32 mem_mask = ~0);
	void local_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 dm_mem_r(offs_t offset, u32 mem_mask = ~0);
	void dm_mem_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 dm_io_r(offs_t offset, u32 mem_mask = ~0);
	void dm_io_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void config_map(address_map &map) override ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void pcisr_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void log_unimplemented_read(offs_t offset, u32 mem_mask);
	void log_unimplemented_write(offs_t offset, u32 data, u32 mem_mask);

	offs_t dm_address(offs_t offset, bool io) const;
	bool config_cycle(u8 &bus, u8 &devfn, u16 &reg);

	u8 m_lmisc1;
	u32 m_dmrr, m_dmlbam, m_dmlbai, m_dmpbam, m_dmcfga;
};

DECLARE_DEVICE_TYPE(PCI9054, pci9054_device)

#endif // MAME_MACHINE_PCI9054_H
