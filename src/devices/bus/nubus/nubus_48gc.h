// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_NUBUS_NUBUS_48GC_H
#define MAME_BUS_NUBUS_NUBUS_48GC_H

#pragma once

#include "nubus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> jmfb_device

class jmfb_device :
		public device_t,
		public device_video_interface,
		public device_nubus_card_interface
{
protected:
	// construction/destruction
	jmfb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool is824);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	uint32_t mac_48gc_r(offs_t offset, uint32_t mem_mask = ~0);
	void mac_48gc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	screen_device *m_screen;
	emu_timer *m_timer;

	std::vector<uint8_t> m_vram;
	uint32_t m_mode, m_vbl_disable, m_toggle, m_stride, m_base;
	uint32_t m_palette[256], m_colors[3], m_count, m_clutoffs;
	uint32_t m_registers[0x100];
	int m_xres, m_yres;
	const bool m_is824;
};

class nubus_48gc_device : public jmfb_device
{
public:
	nubus_48gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class nubus_824gc_device : public jmfb_device
{
public:
	nubus_824gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const tiny_rom_entry *device_rom_region() const override;
};

// device type definition
DECLARE_DEVICE_TYPE(NUBUS_48GC,  nubus_48gc_device)
DECLARE_DEVICE_TYPE(NUBUS_824GC, nubus_824gc_device)

#endif  /// MAME_BUS_NUBUS_NUBUS_48GC_H
