// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_NUBUS_PDS30_30HR_H
#define MAME_BUS_NUBUS_PDS30_30HR_H

#pragma once

#include "nubus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nubus_xceed30hr_device

class nubus_xceed30hr_device :
		public device_t,
		public device_video_interface,
		public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_xceed30hr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nubus_xceed30hr_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	uint32_t xceed30hr_r(offs_t offset, uint32_t mem_mask = ~0);
	void xceed30hr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t vram_r(offs_t offset, uint32_t mem_mask = ~0);
	void vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	std::vector<uint8_t> m_vram;
	uint32_t *m_vram32;
	uint32_t m_mode, m_vbl_disable, m_toggle;
	uint32_t m_palette[256], m_colors[3], m_count, m_clutoffs;
	emu_timer *m_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(PDS030_XCEED30HR, nubus_xceed30hr_device)

#endif // MAME_BUS_NUBUS_PDS30_30HR_H
