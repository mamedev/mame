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

protected:
	nubus_vikbw_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	uint32_t viking_ack_r();
	void viking_ack_w(uint32_t data);
	uint32_t viking_enable_r();
	void viking_disable_w(uint32_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	std::vector<uint32_t> m_vram;
	uint32_t m_vbl_disable, m_palette[2];
};

// device type definition
DECLARE_DEVICE_TYPE(NUBUS_VIKBW, nubus_vikbw_device)

#endif // MAME_BUS_NUBUS_VIKBW_H
