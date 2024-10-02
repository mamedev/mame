// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
#ifndef MAME_BUS_COCO_DRAGON_JCBSND_H
#define MAME_BUS_COCO_DRAGON_JCBSND_H

#pragma once

#include "cococart.h"
#include "sound/ay8910.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dragon_jcbsnd_device

class dragon_jcbsnd_device :
		public device_t,
		public device_cococart_interface
{
public:
	// construction/destruction
	dragon_jcbsnd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual u8 *get_cart_base() override;
	virtual memory_region *get_cart_memregion() override;

	virtual u8 cts_read(offs_t offset) override;
	virtual void cts_write(offs_t offset, u8 data) override;

	// internal state
	device_image_interface *m_cart;
private:
	required_memory_region m_eprom;
	required_device<ay8910_device> m_ay8910;
};


// device type definition
DECLARE_DEVICE_TYPE(DRAGON_JCBSND, dragon_jcbsnd_device)

#endif // MAME_BUS_COCO_DRAGON_JCBSND_H
