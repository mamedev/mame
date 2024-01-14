// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// PCI interfacing gate array common to the sw1000xg and the ds2416

#ifndef MAME_SOUND_YMP21_H
#define MAME_SOUND_YMP21_H

#pragma once

#include "pci_slot.h"

class ymp21_device : public pci_card_device {
protected:
	ymp21_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void map(address_map &map);

	u32 read(offs_t offset, u32 mem_mask);
	void write(offs_t offset, u32 data, u32 mem_mask);
};

#endif
