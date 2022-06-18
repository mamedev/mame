// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_NUBUS_NUBUS_CB264_H
#define MAME_BUS_NUBUS_NUBUS_CB264_H

#pragma once

#include "nubus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nubus_cb264_device

class nubus_cb264_device :
		public device_t,
		public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_cb264_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nubus_cb264_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	uint32_t cb264_r(offs_t offset, uint32_t mem_mask = ~0);
	void cb264_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cb264_ramdac_r(offs_t offset);
	void cb264_ramdac_w(offs_t offset, uint32_t data);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	std::vector<uint32_t> m_vram;
	uint32_t m_cb264_mode, m_cb264_vbl_disable, m_cb264_toggle;
	uint32_t m_palette[256], m_colors[3], m_count, m_clutoffs;
};


// device type definition
DECLARE_DEVICE_TYPE(NUBUS_CB264, nubus_cb264_device)

#endif // MAME_BUS_NUBUS_NUBUS_CB264_H
