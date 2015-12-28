// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __NUBUS_SPEC8S3_H__
#define __NUBUS_SPEC8S3_H__

#include "emu.h"
#include "nubus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nubus_spec8s3_device

class nubus_spec8s3_device :
		public device_t,
		public device_video_interface,
		public device_nubus_card_interface
{
public:
		// construction/destruction
		nubus_spec8s3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
		nubus_spec8s3_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const rom_entry *device_rom_region() const override;
		virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

		UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;

		DECLARE_READ32_MEMBER(spec8s3_r);
		DECLARE_WRITE32_MEMBER(spec8s3_w);
		DECLARE_READ32_MEMBER(vram_r);
		DECLARE_WRITE32_MEMBER(vram_w);

public:
		dynamic_buffer m_vram;
		UINT32 *m_vram32;
		UINT32 m_mode, m_vbl_disable;
		UINT32 m_palette[256], m_colors[3], m_count, m_clutoffs;
		emu_timer *m_timer;
		std::string m_assembled_tag;

private:
		//UINT32 m_7xxxxx_regs[0x100000/4];
		//int m_width, m_height, m_patofsx, m_patofsy;
		//UINT32 m_vram_addr, m_vram_src;
		//UINT8 m_fillbytes[256];
		bool m_vbl_pending;
		int m_parameter;
};


// device type definition
extern const device_type NUBUS_SPEC8S3;

#endif  /* __NUBUS_SPEC8S3_H__ */
