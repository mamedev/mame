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
		nubus_vikbw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
		nubus_vikbw_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const tiny_rom_entry *device_rom_region() const override;

		uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;

		uint32_t viking_ack_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
		void viking_ack_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
		uint32_t viking_enable_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
		void viking_disable_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

public:
		std::vector<uint8_t> m_vram;
		uint32_t m_vbl_disable, m_palette[2];
};


// device type definition
extern const device_type NUBUS_VIKBW;

#endif  /* __NUBUS_VIKBW_H__ */
