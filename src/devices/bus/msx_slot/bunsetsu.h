// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_BUNSETSU_H
#define MAME_BUS_MSX_SLOT_BUNSETSU_H

#pragma once

#include "bus/msx_slot/slot.h"
#include "bus/msx_slot/rom.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_BUNSETSU, msx_slot_bunsetsu_device)


class msx_slot_bunsetsu_device : public msx_slot_rom_device
{
public:
	msx_slot_bunsetsu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_bunsetsu_region_tag(const char *tag) { m_bunsetsu_region.set_tag(tag); }

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_reset() override;

private:
	required_region_ptr<uint8_t> m_bunsetsu_region;
	uint32_t m_bunsetsu_address;
};


#endif // MAME_BUS_MSX_SLOT_BUNSETSU_H
