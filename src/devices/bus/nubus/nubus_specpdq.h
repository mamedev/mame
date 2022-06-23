// license:BSD-3-Clause
// copyright-holders:R. Belmont, Vas Crabb
#ifndef MAME_BUS_NUBUS_NUBUS_SPECPDQ_H
#define MAME_BUS_NUBUS_NUBUS_SPECPDQ_H

#pragma once

#include "nubus.h"
#include "supermac.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nubus_specpdq_device :
		public device_t,
		public device_nubus_card_interface,
		public device_video_interface,
		public device_palette_interface
{
public:
	// construction/destruction
	nubus_specpdq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nubus_specpdq_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	// palette implementation
	virtual uint32_t palette_entries() const override;

	TIMER_CALLBACK_MEMBER(vbl_tick);

private:
	uint32_t specpdq_r(offs_t offset, uint32_t mem_mask = ~0);
	void specpdq_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t vram_r(offs_t offset, uint32_t mem_mask = ~0);
	void vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void blitter_pattern_fill();
	void blitter_copy_forward();
	void blitter_copy_backward();

	void update_crtc();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_ioport m_userosc;
	emu_timer *m_timer;

	supermac_spec_crtc m_crtc;
	supermac_spec_shift_reg m_shiftreg;

	std::vector<uint32_t> m_vram;
	uint32_t m_mode, m_vbl_disable;
	uint32_t m_colors[3], m_count, m_clutoffs;

	uint16_t m_stride;
	uint16_t m_vint;
	uint8_t m_hdelay;
	uint8_t m_osc;

	uint16_t m_blit_stride;
	uint32_t m_blit_src, m_blit_dst;
	uint32_t m_blit_width, m_blit_height;
	uint8_t m_blit_patoffs;
	uint32_t m_blit_pat[64];

	uint32_t m_7xxxxx_regs[0x100000 / 4];
};


// device type definition
DECLARE_DEVICE_TYPE(NUBUS_SPECPDQ, nubus_specpdq_device)

#endif // MAME_BUS_NUBUS_NUBUS_SPECPDQ_H
