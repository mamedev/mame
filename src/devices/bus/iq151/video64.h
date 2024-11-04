// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_IQ151_VIDEO64_H
#define MAME_BUS_IQ151_VIDEO64_H

#pragma once

#include "iq151.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> iq151_video64_device

class iq151_video64_device :
		public device_t,
		public device_gfx_interface,
		public device_iq151cart_interface
{
public:
	// construction/destruction
	iq151_video64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// iq151cart_interface overrides
	virtual void read(offs_t offset, uint8_t &data) override;
	virtual void write(offs_t offset, uint8_t data) override;
	virtual void io_read(offs_t offset, uint8_t &data) override;
	virtual void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect) override;

private:
	required_region_ptr<uint8_t> m_videoram;
	required_region_ptr<uint8_t> m_chargen;
};


// device type definition
DECLARE_DEVICE_TYPE(IQ151_VIDEO64, iq151_video64_device)

#endif // MAME_BUS_IQ151_VIDEO64_H
