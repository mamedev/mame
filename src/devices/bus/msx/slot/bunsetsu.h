// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_BUNSETSU_H
#define MAME_BUS_MSX_SLOT_BUNSETSU_H

#pragma once

#include "rom.h"
#include "slot.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_BUNSETSU, msx_slot_bunsetsu_device)


class msx_slot_bunsetsu_device : public msx_slot_rom_device
{
public:
	msx_slot_bunsetsu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	void set_bunsetsu_region_tag(const char *tag) { m_bunsetsu_region.set_tag(tag); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 buns_read();
	void buns_write(offs_t offset, u8 data);

	required_region_ptr<uint8_t> m_bunsetsu_region;
	u32 m_bunsetsu_address;
};


#endif // MAME_BUS_MSX_SLOT_BUNSETSU_H
