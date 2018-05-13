// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_BUNSETSU_H
#define MAME_BUS_MSX_SLOT_BUNSETSU_H

#pragma once

#include "bus/msx_slot/slot.h"
#include "bus/msx_slot/rom.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_BUNSETSU, msx_slot_bunsetsu_device)


#define MCFG_MSX_SLOT_BUNSETSU_ADD(_tag, _startpage, _numpages, _region, _offset, _bunsetsu_region_tag) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_BUNSETSU, _startpage, _numpages) \
	downcast<msx_slot_rom_device &>(*device).set_rom_start(_region, _offset); \
	downcast<msx_slot_bunsetsu_device &>(*device).set_bunsetsu_region_tag(_bunsetsu_region_tag);

class msx_slot_bunsetsu_device : public msx_slot_rom_device
{
public:
	msx_slot_bunsetsu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_bunsetsu_region_tag(const char *tag) { m_bunsetsu_region.set_tag(tag); }

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

protected:
	virtual void device_reset() override;

private:
	required_region_ptr<uint8_t> m_bunsetsu_region;
	uint32_t m_bunsetsu_address;
};


#endif // MAME_BUS_MSX_SLOT_BUNSETSU_H
