// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_PCI_YMF740C_H
#define MAME_BUS_PCI_YMF740C_H

#pragma once

#include "pci_slot.h"
#include "sound/ymopl.h"

class ymf740c_device : public pci_card_device
{
public:
	ymf740c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND; }

protected:
	ymf740c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;
	virtual u8 capptr_r() override;
private:
	required_device<ymf262_device> m_opl3;

	void map(address_map &map) ATTR_COLD;
	void fm_map(address_map &map) ATTR_COLD;

	u32 m_legacy_audio_control;
};

DECLARE_DEVICE_TYPE(YMF740C, ymf740c_device)

#endif // MAME_BUS_PCI_YMF740C_H
