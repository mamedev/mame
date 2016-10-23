// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_SLOT_ROM_H
#define __MSX_SLOT_ROM_H

#include "slot.h"

#define MCFG_MSX_SLOT_ROM_ADD(_tag, _startpage, _numpages, _region, _offset) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_ROM, _startpage, _numpages) \
	msx_slot_rom_device::set_rom_start(*device, "^" _region, _offset);

class msx_slot_rom_device : public device_t,
							public msx_internal_slot_interface
{
public:
	msx_slot_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	msx_slot_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	static void set_rom_start(device_t &device, const char *region, uint32_t offset);

	virtual void device_start() override;

	virtual uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;

private:
	required_memory_region m_rom_region;
	uint32_t m_region_offset;
	const uint8_t *m_rom;
};

extern const device_type MSX_SLOT_ROM;

#endif
