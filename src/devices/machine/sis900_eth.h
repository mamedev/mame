// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_SIS900_ETH_H
#define MAME_MACHINE_SIS900_ETH_H

#pragma once

#include "pci.h"
#include "machine/eepromser.h"

class sis900_eth_device : public pci_device
{
public:
	sis900_eth_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::LAN; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

//  virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	void memory_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

private:
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_memory_region m_eth_rom;

	bool m_eromar_mode;

	virtual uint8_t capptr_r() override;
	u32 pmc_id_r();
//  void pmc_status_r();
//  u32 pmc_control(offs_t offset, u32 data, u32 mem_mask = ~0);

	u8 eromar_r();
	void eromar_w(u8 data);
};

DECLARE_DEVICE_TYPE(SIS900_ETH, sis900_eth_device)

#endif
