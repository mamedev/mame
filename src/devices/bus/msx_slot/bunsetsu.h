// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_SLOT_BUNSETSU_H
#define __MSX_SLOT_BUNSETSU_H

#include "bus/msx_slot/slot.h"
#include "bus/msx_slot/rom.h"


extern const device_type MSX_SLOT_BUNSETSU;


#define MCFG_MSX_SLOT_BUNSETSU_ADD(_tag, _startpage, _numpages, _region, _offset, _bunsetsu_region_tag) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_BUNSETSU, _startpage, _numpages) \
	msx_slot_rom_device::set_rom_start(*device, _region, _offset); \
	msx_slot_bunsetsu_device::set_bunsetsu_region_tag(*device, _bunsetsu_region_tag);
class msx_slot_bunsetsu_device : public msx_slot_rom_device
{
public:
	msx_slot_bunsetsu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void set_bunsetsu_region_tag(device_t &device, const char *tag) { dynamic_cast<msx_slot_bunsetsu_device &>(device).m_bunsetsu_region_tag = tag; }

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

private:
	memory_region *m_bunsetsu_region;
	const char *m_bunsetsu_region_tag;
	UINT32 m_bunsetsu_address;
};


#endif
