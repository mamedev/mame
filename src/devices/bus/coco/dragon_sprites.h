// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
#ifndef MAME_BUS_COCO_DRAGON_SPRITES_H
#define MAME_BUS_COCO_DRAGON_SPRITES_H

#pragma once

#include "cococart.h"
#include "video/tms9928a.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dragon_sprites_device

class dragon_sprites_device :
		public device_t,
		public device_cococart_interface
{
public:
	// construction/destruction
	dragon_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual u8 cts_read(offs_t offset) override;
	virtual void cts_write(offs_t offset, u8 data) override;

private:
	required_memory_region m_eprom;
	required_device<tms9929a_device> m_vdp;

	void nmi_w(int state);
};


// device type definition
DECLARE_DEVICE_TYPE(DRAGON_SPRITES, dragon_sprites_device)

#endif // MAME_BUS_COCO_DRAGON_SPRITES_H
