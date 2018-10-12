// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_ROM_H
#define MAME_BUS_MSX_SLOT_ROM_H

#pragma once

#include "slot.h"

#define MCFG_MSX_SLOT_ROM_ADD(_tag, _startpage, _numpages, _region, _offset) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_ROM, _startpage, _numpages) \
	downcast<msx_slot_rom_device &>(*device).set_rom_start(_region, _offset);

class msx_slot_rom_device : public device_t,
							public msx_internal_slot_interface
{
public:
	msx_slot_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_rom_start(const char *region, uint32_t offset) { m_rom_region.set_tag(region); m_region_offset = offset; }

	virtual DECLARE_READ8_MEMBER(read) override;

protected:
	msx_slot_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;

private:
	required_memory_region m_rom_region;
	uint32_t m_region_offset;
	const uint8_t *m_rom;
};

DECLARE_DEVICE_TYPE(MSX_SLOT_ROM, msx_slot_rom_device)

#endif // MAME_BUS_MSX_SLOT_ROM_H
