// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_SIS7018_AUDIO_H
#define MAME_MACHINE_SIS7018_AUDIO_H

#pragma once

#include "pci.h"

class sis7018_audio_device : public pci_device
{
public:
	sis7018_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

//  virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

	void memory_map(address_map &map);
	void io_map(address_map &map);

private:
	virtual u8 capptr_r() override;

	u32 pmc_id_r();
//  void pmc_status_r();
//  u32 pmc_control(offs_t offset, u32 data, u32 mem_mask = ~0);

	u8 unmap_log_r(offs_t offset);
	void unmap_log_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(SIS7018_AUDIO, sis7018_audio_device)

#endif
