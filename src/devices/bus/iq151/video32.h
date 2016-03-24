// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __IQ151_VIDEO32_H__
#define __IQ151_VIDEO32_H__

#include "emu.h"
#include "iq151.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> iq151_video32_device

class iq151_video32_device :
		public device_t,
		public device_iq151cart_interface
{
public:
	// construction/destruction
	iq151_video32_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// iq151cart_interface overrides
	virtual void read(offs_t offset, UINT8 &data) override;
	virtual void write(offs_t offset, UINT8 data) override;
	virtual void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect) override;

private:
	UINT8 *     m_videoram;
	UINT8 *     m_chargen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


// device type definition
extern const device_type IQ151_VIDEO32;

#endif  /* __IQ151_VIDEO32_H__ */
