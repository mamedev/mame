// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __NUBUS_VIKBW_H__
#define __NUBUS_VIKBW_H__

#include "emu.h"
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
		nubus_vikbw_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
		nubus_vikbw_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const rom_entry *device_rom_region() const override;

		UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;

		DECLARE_READ32_MEMBER(viking_ack_r);
		DECLARE_WRITE32_MEMBER(viking_ack_w);
		DECLARE_READ32_MEMBER(viking_enable_r);
		DECLARE_WRITE32_MEMBER(viking_disable_w);

public:
		dynamic_buffer m_vram;
		UINT32 m_vbl_disable, m_palette[2];
};


// device type definition
extern const device_type NUBUS_VIKBW;

#endif  /* __NUBUS_VIKBW_H__ */
