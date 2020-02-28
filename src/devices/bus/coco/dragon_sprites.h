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
	dragon_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual DECLARE_READ8_MEMBER(cts_read) override;
	virtual DECLARE_WRITE8_MEMBER(cts_write) override;

private:
	required_memory_region m_eprom;
	required_device<tms9929a_device> m_vdp;

	DECLARE_WRITE_LINE_MEMBER(nmi_w);
};


// device type definition
DECLARE_DEVICE_TYPE(DRAGON_SPRITES, dragon_sprites_device)

#endif // MAME_BUS_COCO_DRAGON_SPRITES_H
