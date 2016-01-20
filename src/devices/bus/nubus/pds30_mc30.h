// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __NUBUS_XCEEDMC30_H__
#define __NUBUS_XCEEDMC30_H__

#include "emu.h"
#include "nubus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nubus_xceedmc30_device

class nubus_xceedmc30_device :
		public device_t,
		public device_video_interface,
		public device_nubus_card_interface
{
public:
		// construction/destruction
		nubus_xceedmc30_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
		nubus_xceedmc30_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const rom_entry *device_rom_region() const override;

		UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
		virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

		DECLARE_READ32_MEMBER(xceedmc30_r);
		DECLARE_WRITE32_MEMBER(xceedmc30_w);
		DECLARE_READ32_MEMBER(vram_r);
		DECLARE_WRITE32_MEMBER(vram_w);

public:
		dynamic_buffer m_vram;
		UINT32 *m_vram32;
		UINT32 m_mode, m_vbl_disable, m_toggle;
		UINT32 m_palette[256], m_colors[3], m_count, m_clutoffs;
		emu_timer *m_timer;
		std::string m_assembled_tag;
};


// device type definition
extern const device_type PDS030_XCEEDMC30;

#endif  /* __NUBUS_XCEEDMC30_H__ */
