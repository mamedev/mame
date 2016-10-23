// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __NUBUS_M2VIDEO_H__
#define __NUBUS_M2VIDEO_H__

#include "emu.h"
#include "nubus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nubus_m2video_device

class nubus_m2video_device :
		public device_t,
		public device_video_interface,
		public device_nubus_card_interface
{
public:
		// construction/destruction
		nubus_m2video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
		nubus_m2video_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const tiny_rom_entry *device_rom_region() const override;

		uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
		virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

		uint32_t m2video_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
		void m2video_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
		uint32_t vram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
		void vram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

public:
		std::vector<uint8_t> m_vram;
		uint32_t *m_vram32;
		uint32_t m_mode, m_vbl_disable, m_toggle;
		uint32_t m_palette[256], m_colors[3], m_count, m_clutoffs;
		emu_timer *m_timer;
		std::string m_assembled_tag;
};


// device type definition
extern const device_type NUBUS_M2VIDEO;

#endif  /* __NUBUS_M2VIDEO_H__ */
