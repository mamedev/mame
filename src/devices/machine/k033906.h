// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami 033906 SHARC-to-PCI host bridge

***************************************************************************/

#ifndef MAME_MACHINE_K033906_H
#define MAME_MACHINE_K033906_H

#pragma once

#include "pci.h"

class k033906_device : public pci_host_device
{
public:
	k033906_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	u32 read(offs_t offset, u32 mem_mask = ~0);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0);
	void set_reg(int state);

	u32 memory_r(offs_t offset, u32 mem_mask = ~0);
	void memory_w(offs_t offset, u32 data, u32 mem_mask = ~0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

private:
	enum
	{
		AS_PCI_MEM = 1,
		AS_PCI_IO = 2
	};

	address_space_config m_memory_config;
	address_space_config m_io_config;
	memory_share_creator<u32> m_ram;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_memory;
	uint8_t m_reg_set; // 1 = access configuration / 0 = access RAM
};

DECLARE_DEVICE_TYPE(K033906, k033906_device)

#endif // MAME_MACHINE_K033906_H
