// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_NUBUS_VIKBW_H
#define MAME_BUS_NUBUS_VIKBW_H

#pragma once

#include "nubus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nubus_vikbw_device

class nubus_vikbw_device :
		public device_t,
		public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_vikbw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	nubus_vikbw_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	DECLARE_READ32_MEMBER(viking_ack_r);
	DECLARE_WRITE32_MEMBER(viking_ack_w);
	DECLARE_READ32_MEMBER(viking_enable_r);
	DECLARE_WRITE32_MEMBER(viking_disable_w);

	std::vector<uint8_t> m_vram;
	uint32_t m_vbl_disable, m_palette[2];
};

// device type definition
DECLARE_DEVICE_TYPE(NUBUS_VIKBW, nubus_vikbw_device)

#endif // MAME_BUS_NUBUS_VIKBW_H
