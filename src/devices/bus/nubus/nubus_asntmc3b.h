// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __NUBUS_ASNTMC3B_H__
#define __NUBUS_ASNTMC3B_H__

#include "emu.h"
#include "nubus.h"
#include "machine/dp8390.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nubus_mac8390_device

class nubus_mac8390_device :
		public device_t,
		public device_nubus_card_interface
{
public:
		// construction/destruction
		nubus_mac8390_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const tiny_rom_entry *device_rom_region() const override;

		void dp_irq_w(int state);
		uint8_t dp_mem_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
		void dp_mem_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;

		uint8_t asntm3b_ram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
		void asntm3b_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
		uint32_t en_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
		void en_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

		required_device<dp8390_device> m_dp83902;

private:
		uint8_t m_ram[0x20000];
		uint8_t m_prom[16];
};

class nubus_asntmc3nb_device : public nubus_mac8390_device
{
public:
	nubus_asntmc3nb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class nubus_appleenet_device : public nubus_mac8390_device
{
public:
	nubus_appleenet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const tiny_rom_entry *device_rom_region() const override;
};

// device type definition
extern const device_type NUBUS_ASNTMC3NB;
extern const device_type NUBUS_APPLEENET;

#endif  /* __NUBUS_ASNTMC3B_H__ */
