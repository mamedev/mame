// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_PCI_TRIDENT_4DWAVEDX_H
#define MAME_BUS_PCI_TRIDENT_4DWAVEDX_H

#pragma once

#include "pci_slot.h"

#include "bus/pc_joy/pc_joy.h"

class trident_4dwavedx_device : public pci_card_device
{
public:
	trident_4dwavedx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND; }

protected:
	trident_4dwavedx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

	virtual u8 capptr_r() override;

private:
	void io_map(address_map &map);
	void mmio_map(address_map &map);

	void gameport_map(address_map &map);

	required_device<pc_joy_device> m_joy;

	u32 m_ddma_config;
	u32 m_legacy_control;
	struct {
		bool enable;
		u8 vector;
	} m_interrupt_snoop;

	u8 m_power_state;

	u8 m_asr4, m_asr5, m_asr6;
};

DECLARE_DEVICE_TYPE(TRIDENT_4DWAVEDX, trident_4dwavedx_device)

#endif // MAME_BUS_PCI_TRIDENT_4DWAVEDX_H
