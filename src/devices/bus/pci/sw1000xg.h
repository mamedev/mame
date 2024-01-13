// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_SOUND_SW1000XG_H
#define MAME_SOUND_SW1000XG_H

#pragma once

#include "pci_slot.h"

class sw1000xg_device : public pci_card_device {
public:
	sw1000xg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void map(address_map &map);

	u32 read(offs_t offset, u32 mem_mask);
	void write(offs_t offset, u32 data, u32 mem_mask);
};

DECLARE_DEVICE_TYPE(SW1000XG, sw1000xg_device)

#endif
