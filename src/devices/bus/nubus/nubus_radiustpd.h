// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_NUBUS_NUBUS_RADIUSTPD_H
#define MAME_BUS_NUBUS_NUBUS_RADIUSTPD_H

#pragma once

#include "nubus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nubus_radiustpd_device

class nubus_radiustpd_device :
		public device_t,
		public device_video_interface,
		public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_radiustpd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nubus_radiustpd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(vbl_tick);

private:
	uint32_t radiustpd_r(offs_t offset, uint32_t mem_mask = ~0);
	void radiustpd_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t vram_r(offs_t offset, uint32_t mem_mask = ~0);
	void vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	std::vector<uint32_t> m_vram;
	uint32_t m_mode, m_vbl_disable, m_toggle;
	uint32_t m_palette[256], m_colors[3], m_count, m_clutoffs;
	emu_timer *m_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(NUBUS_RADIUSTPD, nubus_radiustpd_device)

#endif // MAME_BUS_NUBUS_NUBUS_RADIUSTPD_H
