// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_BRUC100_H
#define MAME_BUS_MSX_SLOT_BRUC100_H

#pragma once

#include "slot.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_BRUC100, msx_slot_bruc100_device)


class msx_slot_bruc100_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_bruc100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_rom_start(const char *region, uint32_t offset) { m_rom_region.set_tag(region); m_region_offset = offset; }

	virtual uint8_t read(offs_t offset) override;
	void select_bank(uint8_t bank);

protected:
	virtual void device_start() override;
	virtual void device_post_load() override;

private:
	required_memory_region m_rom_region;
	uint32_t m_region_offset;
	const uint8_t *m_rom;
	uint8_t m_selected_bank;
	const uint8_t *m_bank_base;

	void map_bank();
};


#endif // MAME_BUS_MSX_SLOT_BRUC100_H
