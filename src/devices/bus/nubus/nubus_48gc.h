// license:BSD-3-Clause
// copyright-holders:R. Belmont, Vas Crabb
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
		public device_nubus_card_interface,
		public device_video_interface,
		public device_palette_interface
{
protected:
	// construction/destruction
	jmfb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	// palette implementation
	uint32_t palette_entries() const override;

	TIMER_CALLBACK_MEMBER(vbl_tick);

	uint32_t jmfb_r(offs_t offset, uint32_t mem_mask = ~0);
	void jmfb_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void dac_ctrl_w(uint8_t data);
	void dac_data_w(uint8_t data);
	void mode_w(uint32_t data, bool rgb);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void update_crtc();

	uint32_t rgb_unpack(offs_t offset, uint32_t mem_mask = ~0);
	void rgb_pack(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	required_ioport m_monitor;
	memory_view m_vram_view;
	emu_timer *m_timer;

	std::vector<uint32_t> m_vram;
	uint32_t m_vbl_disable, m_toggle, m_stride, m_base;
	uint32_t m_registers[0x100];
	uint8_t m_preload;

	uint8_t m_colors[3], m_count, m_clutoffs, m_mode;

	uint16_t m_hactive, m_hbporch, m_hsync, m_hfporch;
	uint16_t m_vactive, m_vbporch, m_vsync, m_vfporch;
	uint16_t m_multiplier;
	uint16_t m_modulus;
	uint8_t m_pdiv;
};

class nubus_48gc_device : public jmfb_device
{
public:
	nubus_48gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

private:
	void mac_48gc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
};

class nubus_824gc_device : public jmfb_device
{
public:
	nubus_824gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

private:
	void mac_824gc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
};

// device type definition
DECLARE_DEVICE_TYPE(NUBUS_48GC,  nubus_48gc_device)
DECLARE_DEVICE_TYPE(NUBUS_824GC, nubus_824gc_device)

#endif  /// MAME_BUS_NUBUS_NUBUS_48GC_H
